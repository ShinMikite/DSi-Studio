# Building DSi Studio

The native app uses BlocksDS, the Wonderful toolchain, libnds and mbedTLS. The SDK's cryptography dependency also links the no-IP DSWifi support library; Studio does not initialize Wi-Fi or make network connections. It has been compiled with BlocksDS 1.25.0 and mbedTLS 3.6.4. Follow the [BlocksDS installation guide](https://blocksds.skylyrac.net/docs/setup/) to install the toolchain.

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

Use Python 3.11 or newer and GCC:

```sh
python -m pip install -r tests/requirements.txt
python tools/test.py
```

Tests cover catalog parsing, backup formats and session checks, touch controls, and installer error handling on a disposable host filesystem. The installer tests substitute hardware, cryptography and save formatting; they do not emulate a DSi. UI previews are written to `artifacts/`.

## Assets

Run `python tools/make_audio.py` to reproduce the music and effects. The bitmap font is included; `tools/make_font.py` requires the named Windows font and Pillow.
