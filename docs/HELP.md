# Questions and help

[Back to DSi Studio](../README.md) · [Start here](GETTING-STARTED.md)

## Where is the download?

Download **DSiStudio-0.4.5.zip** from [Releases](https://github.com/ShinMikite/DSi-Studio/releases). Extract it and open **START HERE.txt**. GitHub's **Code → Download ZIP** contains project source files, not the finished app.

## Do I need a Windows app?

No. Copy complete game folders to **DSiWare** on the card using your computer's normal file browser. Studio runs entirely on the console and does not need an Internet connection.

## I only have a game file, no .tmd

The game needs its matching metadata in the same folder. It may be named `tmd.0`, `tmd.1`, another `tmd.N`, `tmd` or end in `.tmd`. Studio recognizes these names automatically and uses the file to check the game and version before installation. Check whether it came with your original game backup or archive.

If it is missing, supply the matching file from your game backup before installing. Do not rename an unrelated file to `.tmd`. Studio checks that both files belong to the same game and version.

## My game folders are named with numbers

That is normal. Prepared collections sometimes name folders after each game's title ID, such as `000300044b4d4745`. Studio accepts those folders and folders named after the game. It reads the display name from the game file.

Leave your existing folders alone. For new games, drag the complete extracted folder into **DSiWare** without renaming its files. You do not need to look up a title ID or create a numbered folder yourself.

## The files inside my ZIP are numbered or named tmd.256

Studio **0.4.3 and later** recognize original filenames such as `00000000` plus `tmd.0`, or `00000001` plus `tmd.1`. Extract the entire folder into **DSiWare** and leave the names unchanged. Older Studio versions require an update to read this layout.

The content must already be decrypted and the matching retail metadata must be present in the same folder. Studio does not use `titleKey.bin` or `titlePass.txt`, and it does not download missing files. Existing `game.nds` + `game.tmd` folders remain supported.

## Studio says more than one game version matches

Keep each game version in its own folder under **DSiWare**, using the files together as supplied in that version's original archive. Do not combine files from different versions. Studio stops instead of choosing an arbitrary metadata file. If you are unsure which files belong together, ask for help with filenames and extensions only; do not upload games or keys.

## Studio says Browsing only

Studio can read the game card but does not have the access needed to back up or install to the DSi. An ordinary R4 launch does not provide that access. A DSpico must be configured for the appropriate DSi launch mode. Adding BIOS files to the card does not enable installation.

## A game is missing from the library

Check that it is DSiWare, that the archive is unpacked, and that the whole game folder is inside **DSiWare** on the selected card. Original numbered files need Studio 0.4.3 or later. Choose **Scan** after adding files. Encrypted content is not supported. Studio is not a DS cartridge-game installer.

The library reads up to 256 games and searches up to eight folder levels below **DSiWare**. Very long paths are skipped. If a game is missing, try placing its whole folder directly inside **DSiWare** with a shorter folder name.

## Can I use games from another region?

The game's supported regions must include your DSi's region. Studio checks this before installation and does not bypass the restriction. Hardware testing has used USA DSi consoles with DSpico; other regions and models have not been verified.

## Can I replace, update or remove an installed game?

Studio adds games. It does not overwrite, update or uninstall existing titles. A title already present on the console is marked **Installed**, and its copy button is disabled. Existing saves are preserved.

## What happens to save files?

For a new installation, Studio creates the save files required by the game. Optional existing saves can be supplied beside the game as `.pub` (public), `.prv` (private) and `.bnr` (banner), using the same filename before the extension. For `Game.nds`, use `Game.pub`; for original content named `00000001`, use `00000001.pub`. Each supplied save must have the size the game expects. This is optional; the normal extracted-folder workflow creates fresh saves.

## What does Switch card mean?

It changes where Studio looks for games and keeps the required console backup: the microSD inside the DSpico, or the SD card in the DSi's side slot. The current location is shown at the top of the library. The other card must be accessible to the current launch method. Switching cards requires a backup check on the newly selected card.

## Why is it checking every byte of my backup?

Studio reads the entire backup and compares its SHA-256 checksum with the record saved when it was created. This detects unreadable, truncated or changed backup data. It also checks that the backup belongs to this DSi. The check can take several minutes and does not write to the console.

A successful check is reused for later installations in the same session. Restarting Studio, switching cards, changed backup files or a failed check can require a new full check. Choosing **Backup** manually also runs a full check. This verifies the saved image; it does not compare it with the console's current games and saves.

## Does Backup update my existing backup?

No. If a completed backup exists for this console, **Backup** verifies it without replacing it. Later installations and save changes are not added to that image. Keep both `nand.bin` and `backup.meta` together, and keep separate current copies of any saves or photos you want to preserve. See [Backups and recovery](RECOVERY.md).

## Can I install several games without restarting?

Yes, one at a time. After **Added to your DSi**, dismiss the message and choose the next game. There is no multi-select queue. Restart normally when you have finished installing everything you want.

## What does DSi free mean?

In Studio 0.4.4 and later, the library's top screen shows the remaining space available under Studio's installation limits. A block is 128 KiB; the app's MB display uses 1,048,576 bytes per MB. Studio uses a 128 MiB game-storage budget and keeps at least 1 MiB of internal storage free. Game files, saves and installation overhead all use space, so Studio still checks each installation before copying.

The readout refreshes when you open or scan the library and after every installation attempt. If Studio's title-slot limit is reached, it says the menu is full. If Studio cannot read the storage information, it shows that the figure is unavailable.

## Why does the music stop during a backup or installation?

Music pauses while Studio backs up, checks or copies files and resumes in the library. Use the console's volume controls for music and sound effects.

## Studio says Previous copy found

An unfinished installation is still staged on internal storage. Studio stops rather than overwriting it. Keep your backup and report the exact message through the help link below. Do not delete internal files or repeatedly retry the copy.

## Can I use one card with several DSi consoles?

Yes. Each DSi needs its own backup, and Studio stores them in separate folders. Leave all folders intact. Games may share the card; installed-game status is checked on the console currently running Studio.

## Both screens briefly went dark

Keep the DSi away from magnets and do not stack it on another console. A nearby magnet can activate its lid sensor and put it to sleep. If the problem repeats with the consoles separated, report when it happens and whether the sound also stops. Do not interrupt an active write to troubleshoot it.

## Does the backup mean I can recover a console that will not start?

Not by itself. You also need a separate working way to start a restore tool. Studio does not install that recovery method or restore NAND. See [Backups and recovery](RECOVERY.md).

## How do I report a problem?

[Open a help request](https://github.com/ShinMikite/DSi-Studio/issues/new?template=help.yml) with your Studio version, DSi model, cartridge type, the action you were taking and the exact message shown. For file problems, filenames and extensions are enough to start.

Do not upload games, NAND backups, console identity files or private keys. A photo of an error screen is useful if it does not contain those details.
