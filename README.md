# DSi Studio

Browse your DSiWare collection and copy supported games to your Nintendo DSi Home Menu.

**[Start here](docs/GETTING-STARTED.md) · [Help](docs/HELP.md) · [Backups and recovery](docs/RECOVERY.md)**

**[Download DSi Studio 0.4.2](https://github.com/ShinMikite/DSi-Studio/releases/download/v0.4.2/DSiStudio-0.4.2.zip)**

Extract the ZIP and open **START HERE.txt**. Copy the folders inside **COPY TO SD CARD** onto your DSpico microSD.

## Using Studio

1. Copy Studio to the **Games** folder on your DSpico microSD card.
2. Put each unpacked game and its matching metadata in **DSiWare**, named as a pair such as `game.nds` and `game.tmd`. Original archive filenames may need changing; see [adding a game](docs/GETTING-STARTED.md#3-add-a-game).
3. Open Studio and complete the required backup for your DSi.
4. Choose a game and select **Copy to DSi**. Follow the checks and hold to confirm.
5. Add another game, or power off, remove the DSpico and restart to play from the Home Menu.

Studio runs offline. Users supply their own games and matching information files. Games, firmware and console backups are not included.

Game folders can have names or numeric title IDs. Both `DSiWare/My Game/game.nds` and `DSiWare/000300044b4d4745/game.nds` work when the matching `game.tmd` is beside the game. You do not need to rename existing folders; Studio reads the display name from the game itself.

You need a DSi, its charger, a working DSpico with DSi storage access, and a microSD card. An ordinary R4 launch can browse games but cannot install them with Studio. The [setup guide](docs/GETTING-STARTED.md) explains the folders and screens.

## Download

Get **DSiStudio-0.4.2.zip** from [Releases](https://github.com/ShinMikite/DSi-Studio/releases). The source ZIP is for building the app yourself.

GitHub's **Code → Download ZIP** downloads source code, not a ready-to-use app.

## Before installing

Installing changes the DSi's internal storage. A failed write can leave the console unable to start. Studio requires a console-specific backup, but **a backup alone does not provide recovery**. Studio cannot restore backups. Read [Backups and recovery](docs/RECOVERY.md).

This is experimental software. Hardware testing has used USA DSi consoles with DSpico; other models, regions and launch methods have not been verified.

## Source and licenses

[Build instructions](docs/BUILDING.md) · [License](LICENSE) · [Third-party notices](THIRD-PARTY-NOTICES.txt) · [Artwork](assets/ARTWORK.txt)

Most code is GPL-3.0-or-later; individual files identify exceptions. DSi Studio is independent of Nintendo.
