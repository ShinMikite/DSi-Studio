$ErrorActionPreference = 'Stop'
Push-Location -LiteralPath (Split-Path $PSScriptRoot -Parent)
try {
    & python tools/test.py
    if ($LASTEXITCODE) { throw 'Tests failed' }
} finally { Pop-Location }
