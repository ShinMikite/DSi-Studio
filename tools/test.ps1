$ErrorActionPreference = 'Stop'
Push-Location -LiteralPath (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force build,artifacts | Out-Null
    & gcc -std=c11 -Wall -Wextra -Werror -Icommon tests/test_catalog.c common/catalog.c -o build/test_catalog.exe
    if ($LASTEXITCODE) { throw 'Test compilation failed' }
    & ./build/test_catalog.exe
    if ($LASTEXITCODE) { throw 'Catalog tests failed' }
    & gcc -std=c11 -Wall -Wextra -Werror -Icommon tests/test_backup_format.c common/backup_format.c -o build/test_backup_format.exe
    if ($LASTEXITCODE) { throw 'Backup format test compilation failed' }
    & ./build/test_backup_format.exe
    if ($LASTEXITCODE) { throw 'Backup format tests failed' }
    & gcc -std=c11 -Wall -Wextra -Werror -Icommon tests/test_backup_session.c common/backup_session.c common/backup_format.c -o build/test_backup_session.exe
    if ($LASTEXITCODE) { throw 'Backup session test compilation failed' }
    & ./build/test_backup_session.exe
    if ($LASTEXITCODE) { throw 'Backup session tests failed' }
    & gcc -std=c11 -Wall -Wextra -Werror -Icommon tests/test_ui_input.c common/ui_input.c -o build/test_ui_input.exe
    if ($LASTEXITCODE) { throw 'Touch-control test compilation failed' }
    & ./build/test_ui_input.exe
    if ($LASTEXITCODE) { throw 'Touch-control tests failed' }
    & gcc -std=c11 -Wall -Wextra -Werror -Icommon tests/render_ui.c common/ui.c common/ui_input.c -o build/render_ui.exe
    if ($LASTEXITCODE) { throw 'Renderer compilation failed' }
    & ./build/render_ui.exe
    if ($LASTEXITCODE) { throw 'Render failed' }
    & python -c "from pathlib import Path; from PIL import Image; [Image.open(p).save(p.with_suffix('.png')) for p in Path('artifacts').glob('*.ppm')]"
    if ($LASTEXITCODE) { throw 'Preview conversion failed' }
} finally { Pop-Location }
