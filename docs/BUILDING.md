# Building DSi Studio

The native app uses BlocksDS, the Wonderful toolchain, libnds and Mbed TLS. Release builds use BlocksDS 1.25.0 and Mbed TLS 3.6.4. Follow the [BlocksDS installation guide](https://blocksds.skylyrac.net/docs/setup/) to install the toolchain, then install the optional [Mbed TLS package](https://blocksds.skylyrac.net/docs/libs/ported_libs/#mbed-tls):

```sh
wf-pacman -S blocksds-mbedtls
```

From a configured shell:

```sh
make -j4
python3 tools/check_backup_buffers.py
```

Set `BLOCKSDS`, `BLOCKSDSEXT` and `WONDERFUL_TOOLCHAIN` if your installation differs from the makefile defaults. Output is written to `dist/DSiStudio.nds` and `dist/DSiStudio.dsi`.

On Windows:

```powershell
./build.ps1 -BashPath 'C:/devkitPro/msys2/usr/bin/bash.exe'
```

The wrapper loads the toolchain environment, builds the app, and checks that buffers shared with ARM7 are in accessible RAM. Games, BIOS, firmware and console backups are not needed to compile.

## Tests

Use Python 3.11 or newer and a host GCC with POSIX headers. Linux or WSL works; on Windows, use MSYS2's GCC and put its `usr/bin` directory on `PATH`. The ARM cross-compiler and a plain MinGW compiler are not substitutes for the host test compiler.

```sh
python -m pip install -r tests/requirements.txt
python tools/test.py
```

Tests cover catalog parsing, original and prepared game folders, mismatched and ambiguous metadata, storage capacity and reserves, backup formats and session checks, touch controls, and installer error handling on a disposable host filesystem. The installer tests substitute hardware, cryptography and save formatting; they do not emulate a DSi. UI previews are written to `artifacts/`.

## Assets

Generated assets are included in the source download. To regenerate them:

- `python tools/make_audio.py` reproduces the music and effects. This step requires NumPy (`python -m pip install numpy`).
- `python tools/make_font.py` converts the included public-domain X11 bitmap font into the app's font table. It uses only Python's standard library.

Asset origins and license details are in [assets/ARTWORK.txt](../assets/ARTWORK.txt). App builds use the included assets directly.
