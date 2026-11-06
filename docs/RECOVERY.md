# Backups and recovery

[Back to DSi Studio](../README.md) · [Start here](GETTING-STARTED.md)

A backup contains the DSi's internal storage at the time it was created: installed games, saves, settings and internal photos. It does not include files stored separately on either SD card. **Backup verified** means the saved image belongs to this console, has the expected size and matches its recorded checksum.

**The backup does not repair a console by itself. Studio cannot restore it.** If the DSi stops starting normally, you need another way to run a restore tool. We have not yet demonstrated that complete recovery process with Studio's backups.

## Can an unmodified DSi be recovered?

It may be possible using **DSi ntrboot**, a separate recovery boot method that can start a tool before the normal DSi Menu. It requires a compatible cartridge with the right setup, files on the DSi's own SD card, and a specific startup procedure. A DSpico that opens Studio normally is not proof that this recovery setup works.

The [DS-Homebrew ntrboot guide](https://wiki.ds-homebrew.com/ds-index/ntrboot) explains the setup and its current limitations, including **NAND Init failure** errors that can require starting a restore tool through another program. Follow that guide for recovery preparation; normal DSpico startup does not establish that recovery will work.

[SafeNANDManager](https://github.com/DS-Homebrew/SafeNANDManager) is an example of a tool that can restore a console's backup once recovery software is running. Studio's backup files still need an actual restoration test with that route. Never use a different console's backup.

A [hardware NAND connection](https://wiki.ds-homebrew.com/ds-index/hardmod) is another route. It requires opening and soldering the console and suitable equipment; it is not a routine fallback for an inexperienced user. Neither route guarantees recovery from a hardware fault.

## What should I do with my backup?

Keep the original **DSiStudio → backups** folder on the card and a second copy on your computer. If you use several consoles, keep all of their separate folders. Never interrupt an active backup or installation to move the card.

Inside each console's folder, keep **nand.bin** and **backup.meta** together. The image contains raw NAND data and a no$gba footer; the small metadata file is Studio's verification record. Studio requires both. Preserve the original files even if a separate recovery tool requires a different filename or layout.

Existing complete backups are verified, not refreshed or overwritten. Installing more games or playing them does not update the image. Keep separate current copies of important saves and photos.

Treat Studio as experimental. Restoring internal storage is itself a write operation with risk; it is not something to try casually on a valued console.

## What a restore changes

Restoring an earlier image rolls internal storage back to the state captured in that image, including installed titles, saves, settings and internal photos. Newer internal data can be lost. An old backup is not a substitute for a current copy of data you want to keep.
