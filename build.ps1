# Build script for STM32N6 n6cam-basic project
# Usage: .\build.ps1 [clean|all|flash]

param(
    [string]$Target = "all"
)

# Set paths for STM32CubeIDE tools
$MAKE_PATH = "C:\ST\STM32CubeIDE_2.0.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin"
$GCC_PATH = "C:\ST\STM32CubeIDE_2.0.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.win32_1.0.100.202509120712\tools\bin"

# Convert backslashes to forward slashes for Unix-like tools
$MAKE_PATH = $MAKE_PATH -replace '\\', '/'
$GCC_PATH = $GCC_PATH -replace '\\', '/'

# Add to PATH
$env:PATH = "$MAKE_PATH;$GCC_PATH;" + $env:PATH
$env:GCC_PATH = $GCC_PATH

# Navigate to the Makefile directory
Set-Location "$PSScriptRoot\Makefile"

Write-Host "Building STM32N6 resetLoadAndRun Application..." -ForegroundColor Cyan
Write-Host "Target: $Target" -ForegroundColor Yellow

# Execute make
& make $Target

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild completed successfully!" -ForegroundColor Green
    
    # Navigate back to project root for signing
    Set-Location $PSScriptRoot
    
    # Create output directory if it doesn't exist
    if (!(Test-Path output)) {
        New-Item -ItemType Directory -Force -Path output | Out-Null
    }
    
    Write-Host "Signing FSBL binary..." -ForegroundColor Cyan
    # Delete existing file if it exists
    if (Test-Path output\FSBL-trusted.bin) {
        Remove-Item output\FSBL-trusted.bin -Force
    }
    & "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_SigningTool_CLI.exe" -bin Makefile\FSBL\build\resetLoadAndRun_FSBL.bin -nk -of 0x80000000 -t fsbl -o output\FSBL-trusted.bin -hv 2.3 -dump output\FSBL-trusted.bin -align
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Signing Appli binary..." -ForegroundColor Cyan
        # Delete existing file if it exists
        if (Test-Path output\Appli-trusted.bin) {
            Remove-Item output\Appli-trusted.bin -Force
        }
        & "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_SigningTool_CLI.exe" -bin Makefile\Appli\build\resetLoadAndRun_Appli.bin -nk -of 0x80000000 -t fsbl -o output\Appli-trusted.bin -hv 2.3 -dump output\Appli-trusted.bin -align
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Signing completed successfully!" -ForegroundColor Green
        } else {
            Write-Host "Signing Appli failed with error code: $LASTEXITCODE" -ForegroundColor Red
            exit $LASTEXITCODE
        }
    } else {
        Write-Host "Signing FSBL failed with error code: $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} else {
    Write-Host "`nBuild failed with error code: $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}