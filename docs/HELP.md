# Questions and help

[Back to DSi Studio](../README.md) · [Start here](GETTING-STARTED.md)

## Where is the download?

Download **DSiStudio-0.4.2.zip** from [Releases](https://github.com/ShinMikite/DSi-Studio/releases). Extract it and open **START HERE.txt**. GitHub's **Code → Download ZIP** contains project source files, not the finished app.

## Do I need a Windows app?

No. Copy complete game folders to **DSiWare** on the card using your computer's normal file browser. Studio runs entirely on the console and does not need an Internet connection.

## I only have a game file, no .tmd

The current version needs a matching `.tmd` information file alongside the game. Studio uses it to check the game and version before installation. Check whether it came with your original game backup or archive.

If it is missing, supply the matching file from your game backup before installing. Do not rename an unrelated file to `.tmd`. Studio checks that both files belong to the same game and version.

## My game folders are named with numbers

That is normal. Prepared collections sometimes name folders after each game's title ID, such as `000300044b4d4745`. Studio accepts those folders and folders named after the game. It reads the display name from the game file.

Leave your existing folders alone. For new games, drag a folder containing a matching pair such as `game.nds` and `game.tmd` into **DSiWare**. You do not need to look up a title ID or create a numbered folder yourself.

## The files inside my ZIP are numbered or named tmd.256

Studio 0.4.2 does not automatically recognize extensionless content such as `00000000` or metadata named `tmd.0`, `tmd.1` or `tmd.256`. Extracting that folder onto the card is not sufficient yet.

For an already-decrypted, single-content game with an identified matching pair, rename the content file to `game.nds` and the matching metadata to `game.tmd`. Put them together in a folder under **DSiWare**. Keep the full metadata file; no trimming is needed. Studio does not use `titleKey.bin` or `titlePass.txt`.

Renaming does not decrypt a file or make mismatched versions compatible. If several content or metadata files are present, ask for help identifying the pair using filenames and extensions only; do not upload the archive, game or keys.

This concerns the filenames inside the folder. A numeric folder containing `game.nds` and `game.tmd` is already in a supported layout.

## Studio says Browsing only

Studio can read the game card but does not have the access needed to back up or install to the DSi. An ordinary R4 launch does not provide that access. A DSpico must be configured for the appropriate DSi launch mode. Adding BIOS files to the card does not enable installation.

## A game is missing from the library

Check that it is DSiWare, that the archive is unpacked, and that the game is inside **DSiWare** on the selected card. Choose **Scan** after adding files. Studio is not a DS cartridge-game installer.

## What does Switch card mean?

It changes where Studio looks for games: the microSD inside the DSpico, or the SD card in the DSi's side slot. The current location is shown at the top of the library. The other card must be accessible to the current launch method.

## Why is it checking every byte of my backup?

That is the full backup check. It can take several minutes and does not write to the DSi. Version 0.4 and later reuse a successful check for later installations in the same session. Restarting Studio, switching cards or a failed check can require a new full check. Choosing **Backup** manually also runs a full check.

## Can I install several games without restarting?

Yes, one at a time. After **Added to your DSi**, dismiss the message and choose the next game. There is no multi-select queue. Restart normally when you have finished installing everything you want.

## Can I use one card with several DSi consoles?

Yes. Each DSi needs its own backup, and Studio stores them in separate folders. Leave all folders intact. Games may share the card; installed-game status is checked on the console currently running Studio.

## Both screens briefly went dark

Keep the DSi away from magnets and do not stack it on another console. A nearby magnet can activate its lid sensor and put it to sleep. If the problem repeats with the consoles separated, report when it happens and whether the sound also stops. Do not interrupt an active write to troubleshoot it.

## Does the backup mean I can recover a console that will not start?

Not by itself. You also need a separate working way to start a restore tool. Studio does not install that recovery method or restore NAND. See [Backups and recovery](RECOVERY.md).

## How do I report a problem?

[Open a help request](https://github.com/ShinMikite/DSi-Studio/issues/new?template=help.yml) with your Studio version, DSi model, cartridge type, the action you were taking and the exact message shown. For file problems, filenames and extensions are enough to start.

Do not upload games, NAND backups, console identity files or private keys. A photo of an error screen is useful if it does not contain those details.
