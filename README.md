# DSi Studio

Browse your DSiWare collection and copy supported games to your Nintendo DSi Home Menu.

**[Start here](docs/GETTING-STARTED.md) · [Help](docs/HELP.md) · [Backups and recovery](docs/RECOVERY.md)**

**[Download DSi Studio 0.4.3](https://github.com/ShinMikite/DSi-Studio/releases/download/v0.4.3/DSiStudio-0.4.3.zip)**

Extract the ZIP and open **START HERE.txt**. Copy the folders inside **COPY TO SD CARD** onto your DSpico microSD.

## Using Studio

1. Copy Studio to the **Games** folder on your DSpico microSD card.
2. Extract each complete game folder into **DSiWare** on the card. Keep its original files and names; Studio finds the game and matching information automatically.
3. Open Studio and complete the required backup for your DSi.
4. Choose a game and select **Copy to DSi**. Follow the checks and hold to confirm.
5. Add another game, or power off, remove the DSpico and restart to play from the Home Menu.

Studio runs offline. Users supply their own games and matching information files. Games, firmware and console backups are not included.

Original folders containing files such as `00000000` and `tmd.0` work directly. Existing `game.nds` + `game.tmd` folders still work. Folder names can be readable names or numeric title IDs; Studio reads the display name from the game itself. No renaming or PC helper is needed. See [supported game folders](docs/GETTING-STARTED.md#3-add-a-game).

You need a DSi, its charger, a working DSpico with DSi storage access, and a microSD card. An ordinary R4 launch can browse games but cannot install them with Studio. The [setup guide](docs/GETTING-STARTED.md) explains the folders and screens.

## Download

Get **DSiStudio-0.4.3.zip** from [Releases](https://github.com/ShinMikite/DSi-Studio/releases). The source ZIP is for building the app yourself.

GitHub's **Code → Download ZIP** downloads source code, not a ready-to-use app.

## Before installing

Installing changes the DSi's internal storage. A failed write can leave the console unable to start. Studio requires a console-specific backup, but **a backup alone does not provide recovery**. Studio cannot restore backups. Read [Backups and recovery](docs/RECOVERY.md).

This is experimental software. Hardware testing has used USA DSi consoles with DSpico; other models, regions and launch methods have not been verified.

## Source and licenses

[Build instructions](docs/BUILDING.md) · [License](LICENSE) · [Third-party notices](THIRD-PARTY-NOTICES.txt) · [Artwork](assets/ARTWORK.txt)

Most code is GPL-3.0-or-later; individual files identify exceptions. DSi Studio is independent of Nintendo.
