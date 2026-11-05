param([string]$BashPath = 'C:\devkitPro\msys2\usr\bin\bash.exe')
$ErrorActionPreference = 'Stop'
$taskRoot = $PSScriptRoot
if (!(Test-Path -LiteralPath $BashPath)) { throw 'Pass -BashPath with the path to your MSYS2 bash.exe.' }
Push-Location -LiteralPath $taskRoot
$previousRoot = $env:STUDIO_BUILD_ROOT
try {
    $env:STUDIO_BUILD_ROOT = $taskRoot.Replace('\','/')
    & $BashPath -lc 'source /opt/wonderful/bin/wf-env && cd "$STUDIO_BUILD_ROOT" && make -j4'
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    & python tools/check_backup_buffers.py
    if ($LASTEXITCODE -ne 0) { throw 'Backup buffers are not safely placed in shared RAM.' }
} finally { $env:STUDIO_BUILD_ROOT = $previousRoot; Pop-Location }
