# DSi Studio

A simple, offline DSiWare installer for Nintendo DSi using DSpico. Browse your games with touch controls and follow the guided steps to add supported titles to your DSi Home Menu.

Studio focuses on complete extracted game folders, required console backups and a straightforward installation flow. It builds on existing DSi homebrew work, including [NAND Title Manager (NTM)](https://github.com/Epicpkmn11/NTM).

**[Start here](docs/GETTING-STARTED.md) · [Help](docs/HELP.md) · [Backups and recovery](docs/RECOVERY.md)**

**[Download DSi Studio 0.4.5](https://github.com/ShinMikite/DSi-Studio/releases/download/v0.4.5/DSiStudio-0.4.5.zip)**

Extract the ZIP and open **START HERE.txt**. Copy the folders inside **COPY TO SD CARD** onto your DSpico microSD.

## Using Studio

1. Copy Studio to the **Games** folder on your DSpico microSD card.
2. Extract each complete game folder into **DSiWare** on the card. Keep its original files and names; Studio finds the game and matching information automatically.
3. Open Studio and complete the required backup for your DSi.
4. Choose a game and select **Copy to DSi**. Follow the checks and hold to confirm.
5. Add another game, or power off, remove the DSpico and restart to play from the Home Menu.

The library shows your available DSi game space in blocks and MB and refreshes it after each installation.

Studio runs offline. Users supply their own games and matching information files. Games, firmware and console backups are not included.

Original folders containing files such as `00000000` and `tmd.0` work directly. Existing `game.nds` + `game.tmd` folders still work. Folder names can be readable names or numeric title IDs; Studio reads the display name from the game itself. No renaming or PC helper is needed. See [supported game folders](docs/GETTING-STARTED.md#3-add-a-game).

You need a DSi, its charger, a working DSpico with DSi storage access, and a microSD card. **DSpico is the supported installation setup.** An ordinary R4 launch can browse games but cannot install them with Studio. The [setup guide](docs/GETTING-STARTED.md) explains the folders and screens.

## Download

Get **DSiStudio-0.4.5.zip** from [Releases](https://github.com/ShinMikite/DSi-Studio/releases). The source ZIP is for building the app yourself.

GitHub's **Code → Download ZIP** downloads source code, not a ready-to-use app.

## Before installing

Installing changes the DSi's internal storage. A failed write can leave the console unable to start. Studio requires a console-specific backup, but **a backup alone does not provide recovery**. Studio cannot restore backups. Read [Backups and recovery](docs/RECOVERY.md).

This is experimental software. Hardware testing has used USA DSi consoles with DSpico; other models, regions and launch methods have not been verified.

## Credits and source

Studio uses cryptographic code from [NTM by Epicpkmn11 and contributors](https://github.com/Epicpkmn11/NTM), and its save formatter derives from NTM with additional error checks. The [source origins](vendor/ntm-crypto/ORIGIN.txt) record the upstream revision and reused components. Original authorship and license notices are preserved.

Studio also uses BlocksDS, the Wonderful toolchain, libnds, FatFs, Mbed TLS and their dependencies. See the [third-party notices](THIRD-PARTY-NOTICES.txt) for credits and license details.

[Build instructions](docs/BUILDING.md) · [License](LICENSE) · [Third-party notices](THIRD-PARTY-NOTICES.txt) · [Artwork](assets/ARTWORK.txt)

Most code is GPL-3.0-or-later; individual files identify exceptions. DSi Studio is independent of Nintendo.
