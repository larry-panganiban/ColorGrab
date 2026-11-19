$ErrorActionPreference = 'Stop'

$packageName = 'colorgrab'
$toolsDir    = Split-Path -Parent $MyInvocation.MyCommand.Definition
$installer   = Join-Path $toolsDir 'colorgrab-setup.exe'

$packageArgs = @{
    packageName    = $packageName
    fileType       = 'exe'
    file           = $installer
    silentArgs     = '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP-'
    validExitCodes = @(0)
}

Install-ChocolateyInstallPackage @packageArgs
