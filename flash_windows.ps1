# Script PowerShell para programar N6CAM via STM32CubeProgrammer no Windows
# Ajuste os caminhos conforme necessário

$PROGRAMMER = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
$LOADER = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\ExternalLoader\MX66UW1G45G_STM32N6570-DK.stldr"
$FSBL_BIN = "output\FSBL-trusted.bin"
$APPLI_BIN = "output\Appli-trusted.bin"

Write-Host "Gravando FSBL..."
& $PROGRAMMER -c port=SWD mode=UR -el $LOADER -w $FSBL_BIN 0x70000000
if ($LASTEXITCODE -ne 0) {
    Write-Host "Erro ao gravar FSBL!"
    exit 1
}

Write-Host "Gravando Application..."
& $PROGRAMMER -c port=SWD mode=UR -el $LOADER -w $APPLI_BIN 0x70100000
if ($LASTEXITCODE -ne 0) {
    Write-Host "Erro ao gravar Application!"
    exit 1
}

# Write-Host "Gravando NN Weights..."
# & $PROGRAMMER -c port=SWD mode=UR -el $LOADER -w "Models\network_rel.bin" 0x70380000
# if ($LASTEXITCODE -ne 0) {
#     Write-Host "Erro ao gravar NN Weights!"
#     exit 1
# }

Write-Host "Resetando dispositivo..."
& $PROGRAMMER -c port=SWD mode=UR -rst
if ($LASTEXITCODE -ne 0) {
    Write-Host "Erro ao resetar dispositivo!"
    exit 1
}

Write-Host "Flash concluído com sucesso!"