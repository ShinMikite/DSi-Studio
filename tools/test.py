"""Run portable tests using synthetic fixtures; no console dumps are required."""
from pathlib import Path
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
def run(*args):
    subprocess.run(args, cwd=ROOT, check=True)

def main():
    compiler = os.environ.get('CC', 'gcc')
    if not shutil.which(compiler):
        raise SystemExit(f'C compiler not found: {compiler}')
    (ROOT/'build').mkdir(exist_ok=True)
    (ROOT/'artifacts').mkdir(exist_ok=True)
    suites = {
        'test_catalog': ['common/catalog.c'],
        'test_storage': ['common/storage.c'],
        'test_backup_format': ['common/backup_format.c'],
        'test_backup_session': ['common/backup_session.c', 'common/backup_format.c'],
        'test_ui_input': ['common/ui_input.c'],
        'render_ui': ['common/ui.c', 'common/ui_input.c'],
    }
    for name, sources in suites.items():
        output = ROOT/'build'/(name+('.exe' if os.name=='nt' else ''))
        run(compiler, '-std=c11', '-Wall', '-Wextra', '-Werror', '-Icommon',
            f'tests/{name}.c', *sources, '-o', str(output))
        run(str(output))
    run(sys.executable, '-m', 'unittest', 'discover', '-s', 'tests', '-p', 'test_*.py', '-v')
    from PIL import Image
    for path in (ROOT/'artifacts').glob('*.ppm'):
        with Image.open(path) as picture: picture.save(path.with_suffix('.png'))

if __name__ == '__main__': main()
