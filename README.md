# DSi Studio

Browse your DSiWare collection and copy supported games to your Nintendo DSi Home Menu.

**[Start here](docs/GETTING-STARTED.md) · [Help](docs/HELP.md) · [Backups and recovery](docs/RECOVERY.md)**

## Using Studio

1. Copy Studio to the **Games** folder on your DSpico microSD card.
2. Drag your complete game folders into **DSiWare** on the card. Each unpacked game needs its matching `.tmd` file.
3. Open Studio and complete the required backup for your DSi.
4. Choose a game and select **Copy to DSi**. Follow the checks and hold to confirm.
5. Add another game, or power off, remove the DSpico and restart to play from the Home Menu.

Studio runs offline. Users supply their own games and matching information files. Games, firmware and console backups are not included.

You need a DSi, its charger, a working DSpico with DSi storage access, and a microSD card. An ordinary R4 launch can browse games but cannot install them with Studio. The [setup guide](docs/GETTING-STARTED.md) explains the folders and screens.

## Download

There is no public app download yet. Downloads will appear under [Releases](https://github.com/ShinMikite/DSi-Studio/releases).

GitHub's **Code → Download ZIP** downloads source code, not a ready-to-use app.

## Before installing

Installing changes the DSi's internal storage. A failed write can leave the console unable to start. Studio requires a console-specific backup, but **a backup alone does not provide recovery**. Studio cannot restore backups. Read [Backups and recovery](docs/RECOVERY.md).

## Source and licenses

[Build instructions](docs/BUILDING.md) · [License](LICENSE) · [Third-party notices](THIRD-PARTY-NOTICES.txt) · [Artwork](assets/ARTWORK.txt)

Most code is GPL-3.0-or-later; individual files identify exceptions. DSi Studio is independent of Nintendo.
