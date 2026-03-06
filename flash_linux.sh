#!/bin/bash
# Build, sign and flash script for resetLoadAndRun on Linux
# Mirrors build.ps1 + flash_windows.ps1 behavior.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MAKEFILE_DIR="$SCRIPT_DIR/Makefile"
OUTPUT_DIR="$SCRIPT_DIR/output"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() { echo -e "${GREEN}[INFO]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_step() { echo -e "${BLUE}[STEP]${NC} $1"; }

# Candidate STM32CubeIDE install directories (ordered by preference)
CUBEIDE_DIRS=(
    "$HOME/st/stm32cubeide_2.1.0"
    "$HOME/st/stm32cubeide_1.19.0"
    "$HOME/st/stm32cubeide_current"
    "/opt/st/stm32cubeide_2.1.0"
    "/opt/st/stm32cubeide_1.19.0_2"
    "/opt/st/stm32cubeide_current"
)

find_cubeide_dir() {
    local d
    for d in "${CUBEIDE_DIRS[@]}"; do
        if [ -d "$d/plugins" ]; then
            echo "$d"
            return 0
        fi
    done
    return 1
}

setup_toolchain_env() {
    local cubeide_dir="$1"
    local make_plugin
    local gcc_plugin

    make_plugin=$(ls -d "$cubeide_dir"/plugins/com.st.stm32cube.ide.mcu.externaltools.make.linux64_* 2>/dev/null | head -1)
    gcc_plugin=$(ls -d "$cubeide_dir"/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*.linux64_* 2>/dev/null | tail -1)

    if [ -z "$make_plugin" ] || [ -z "$gcc_plugin" ]; then
        print_warning "CubeIDE found at $cubeide_dir, but make/GCC plugins were not detected"
        return 1
    fi

    MAKE_PATH="$make_plugin/tools/bin"
    GCC_PATH="$gcc_plugin/tools/bin"

    export MAKE_PATH
    export GCC_PATH
    export PATH="$MAKE_PATH:$GCC_PATH:$PATH"

    print_status "CubeIDE: $cubeide_dir"
    print_status "Toolchain: $GCC_PATH"
    return 0
}

# Configure build toolchain from CubeIDE
CUBEIDE_DIR="$(find_cubeide_dir || true)"
if [ -n "$CUBEIDE_DIR" ]; then
    setup_toolchain_env "$CUBEIDE_DIR" || true
fi

# Tool paths - prefer standalone STM32CubeProgrammer v2.21+, fallback to CubeIDE bundled
CUBEPROG_STANDALONE="/opt/st/STM32CubeProgrammer_v2.21"
PROGRAMMER=""
SIGNING_TOOL=""
LOADER=""

if [ -x "$CUBEPROG_STANDALONE/bin/STM32_Programmer_CLI" ]; then
    PROGRAMMER="$CUBEPROG_STANDALONE/bin/STM32_Programmer_CLI"
    SIGNING_TOOL="$CUBEPROG_STANDALONE/bin/STM32_SigningTool_CLI"
    LOADER="$CUBEPROG_STANDALONE/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr"
    print_status "Using standalone STM32CubeProgrammer v2.21"
else
    PROGRAMMER_PLUGIN=""
    if [ -n "$CUBEIDE_DIR" ]; then
        PROGRAMMER_PLUGIN=$(ls -d "$CUBEIDE_DIR"/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.linux64_* 2>/dev/null | head -1)
    fi
    if [ -z "$PROGRAMMER_PLUGIN" ]; then
        for ide in "${CUBEIDE_DIRS[@]}"; do
            PROGRAMMER_PLUGIN=$(ls -d "$ide"/plugins/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.linux64_* 2>/dev/null | head -1)
            if [ -n "$PROGRAMMER_PLUGIN" ]; then
                break
            fi
        done
    fi

    if [ -z "$PROGRAMMER_PLUGIN" ]; then
        print_error "STM32CubeProgrammer not found"
        print_error "Install standalone v2.21+ to /opt/st/STM32CubeProgrammer_v2.21"
        print_error "Or install STM32CubeIDE and ensure cubeprogrammer plugin is present"
        exit 1
    fi

    PROGRAMMER="$PROGRAMMER_PLUGIN/tools/bin/STM32_Programmer_CLI"
    SIGNING_TOOL="$PROGRAMMER_PLUGIN/tools/bin/STM32_SigningTool_CLI"
    LOADER="$PROGRAMMER_PLUGIN/tools/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr"
    print_warning "Using CubeIDE-bundled programmer (v2.21+ standalone recommended)"
fi

# Memory addresses from flash_windows.ps1
FSBL_ADDR="0x70000000"
APP_ADDR="0x70100000"
MODEL_ADDR="0x70380000"

show_help() {
    echo "Usage: $0 [COMMAND]"
    echo ""
    echo "Commands:"
    echo "  build       Build binaries (make)"
    echo "  sign        Sign FSBL and Application binaries"
    echo "  flash       Flash signed binaries to board"
    echo "  all         Build + sign + flash (default)"
    echo "  clean       Clean build artifacts"
    echo "  help        Show this help"
    echo ""
    echo "Memory Layout:"
    echo "  $FSBL_ADDR  - FSBL"
    echo "  $APP_ADDR   - Application"
    echo "  $MODEL_ADDR - NN weights (optional)"
}

get_signing_tool_version() {
    local ver_out
    local ver_token

    ver_out="$($SIGNING_TOOL --version 2>&1 || $SIGNING_TOOL -v 2>&1 || true)"
    ver_token=$(echo "$ver_out" | grep -Eo '[0-9]+\.[0-9]+(\.[0-9]+)?' | head -1)

    # Normalize to MAJOR.MINOR.PATCH
    if [[ "$ver_token" =~ ^[0-9]+\.[0-9]+$ ]]; then
        ver_token="${ver_token}.0"
    fi

    if [[ "$ver_token" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        echo "$ver_token"
    else
        echo ""
    fi
}

is_version_gt_2_21_0() {
    local version="$1"
    local major minor patch

    IFS='.' read -r major minor patch <<< "$version"

    # Strictly greater than 2.21.0
    if [ "$major" -gt 2 ]; then
        return 0
    fi
    if [ "$major" -lt 2 ]; then
        return 1
    fi
    if [ "$minor" -gt 21 ]; then
        return 0
    fi
    if [ "$minor" -lt 21 ]; then
        return 1
    fi
    if [ "$patch" -gt 0 ]; then
        return 0
    fi
    return 1
}

do_build() {
    print_step "Building resetLoadAndRun..."
    if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
        print_error "arm-none-eabi-gcc not found"
        print_error "Set CUBEIDE_DIRS in this script or install STM32CubeIDE toolchain"
        exit 1
    fi

    cd "$MAKEFILE_DIR"
    make all -j4
    print_status "Build complete"
}

do_sign() {
    print_step "Signing FSBL and Application binaries..."
    local sign_version
    local align_args=()

    sign_version="$(get_signing_tool_version)"
    if [ -n "$sign_version" ] && is_version_gt_2_21_0 "$sign_version"; then
        align_args=(-align)
        print_status "Signing tool v${sign_version} detected (>2.21.0): using -align"
    else
        print_warning "Signing tool version <=2.21.0 or undetected: not using -align"
    fi

    mkdir -p "$OUTPUT_DIR"
    rm -f "$OUTPUT_DIR/FSBL-trusted.bin" "$OUTPUT_DIR/Appli-trusted.bin"

    "$SIGNING_TOOL" \
        -bin "$MAKEFILE_DIR/FSBL/build/resetLoadAndRun_FSBL.bin" \
        -nk -of 0x80000000 -t fsbl \
        -o "$OUTPUT_DIR/FSBL-trusted.bin" -hv 2.3 "${align_args[@]}"

    "$SIGNING_TOOL" \
        -bin "$MAKEFILE_DIR/Appli/build/resetLoadAndRun_Appli.bin" \
        -nk -of 0x80000000 -t fsbl \
        -o "$OUTPUT_DIR/Appli-trusted.bin" -hv 2.3 "${align_args[@]}"

    print_status "Signing complete"
}

do_flash() {
    print_step "Flashing FSBL..."
    "$PROGRAMMER" -c port=SWD mode=UR -el "$LOADER" -w "$OUTPUT_DIR/FSBL-trusted.bin" "$FSBL_ADDR"

    print_step "Flashing Application..."
    "$PROGRAMMER" -c port=SWD mode=UR -el "$LOADER" -w "$OUTPUT_DIR/Appli-trusted.bin" "$APP_ADDR"

    # Optional model flash. Same intention as flash_windows.ps1 commented block.
    if [ -f "$SCRIPT_DIR/Models/network_rel.bin" ]; then
        print_step "Flashing NN Weights..."
        "$PROGRAMMER" -c port=SWD mode=UR -el "$LOADER" -w "$SCRIPT_DIR/Models/network_rel.bin" "$MODEL_ADDR"
    else
        print_warning "Models/network_rel.bin not found, skipping NN weights"
    fi

    print_step "Resetting device..."
    "$PROGRAMMER" -c port=SWD mode=UR -rst

    print_status "Flash completed successfully"
}

do_clean() {
    print_step "Cleaning build artifacts..."
    cd "$MAKEFILE_DIR"
    make clean
    rm -rf "$OUTPUT_DIR"
    print_status "Clean complete"
}

case "${1:-all}" in
    build)
        do_build
        ;;
    sign)
        do_sign
        ;;
    flash)
        do_flash
        ;;
    all)
        do_build
        do_sign
        do_flash
        ;;
    clean)
        do_clean
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        print_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac
