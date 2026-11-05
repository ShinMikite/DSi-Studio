# Start here

[Back to DSi Studio](../README.md) · [Questions and help](HELP.md)

These instructions are for people who have received a **DSi Studio test build**. There is no public download yet. Do not use GitHub's **Code → Download ZIP** as the app download.

## 1. Check your DSpico first

Your DSpico needs to be working on your DSi before you add Studio. Turn on the DSi with the DSpico inserted and check that its file menu opens.

If the DSpico does not start, finish its own setup first. Studio does not install the DSpico's firmware or menu. The [DSpico project's documentation](https://github.com/LNH-team/dspico) explains its setup; if you bought a prepared card, check the instructions supplied with it. Do not change a working cartridge's firmware just to add Studio.

A working file menu is only the first check. Studio also needs access to the DSi's internal storage. If it later says **Browsing only**, stop and see [Help](HELP.md#studio-says-browsing-only).

## 2. Copy Studio to the microSD card

Power off the DSi. Remove the microSD from the DSpico and connect it to your computer with a card reader. Use the microSD card, not the DSpico's USB firmware drive.

Open the card in your computer's file browser. The first folder you see is often called the card's **root**. On an already prepared Pico card, you will usually see `_pico` and `_picoboot.nds` there. Leave them in place.

Open or create a folder named **Games** and copy **DSiStudio.nds** into it. If your test build came in a ZIP, extract the ZIP first. On Windows, right-click the ZIP and choose **Extract All**; on macOS, double-click it.

## 3. Add a game

Go back to the card's first folder. Create a folder named **DSiWare**.

Drag your complete game folder into **DSiWare**, or copy the game file and its matching `.tmd` there directly. For example:

| Where on the card | What goes there |
| --- | --- |
| `Games` | `DSiStudio.nds` |
| `DSiWare` | `Game.nds` and `Game.tmd` |

`Game` is an example name. The two files must have the same name before the ending: **Game.nds + Game.tmd**. The `.tmd` must belong to that exact version of the game. Renaming an unrelated file does not make it match.

You may put each pair in its own folder inside **DSiWare**. Games must be unpacked; Studio does not open ZIP archives on the console. Already-decrypted `.dsi`, `.srl` and `.app` game files are also supported, with a matching `.tmd` beside them.

**Only have the game file?** Supply its matching `.tmd` before continuing. Studio runs offline and does not download missing files. [More help](HELP.md#i-only-have-a-game-file-no-tmd).

## 4. Open Studio

Eject the microSD using your computer's eject option, then put it back into the DSpico. Insert the DSpico into the powered-off DSi and connect the DSi's charger.

Turn on the DSi. In the Pico file menu, open **Games → DSi Studio**. On Studio's title screen, select **Open library**.

## 5. Let Studio back up this DSi

The first time you use a console, Studio shows **First: Back up this DSi**. Choose **Back up** and wait for both saving and checking to finish. This takes several minutes. Keep the charger and card connected.

The backup is a copy of the console's internal storage. It is required before the installation library opens. It does not change the DSi or install a recovery tool. [Read what a backup can and cannot do](RECOVERY.md).

After the backup succeeds, preserve a second copy on your computer:

1. Power off the DSi and connect the microSD to your computer again.
2. Open **DSiStudio → backups** on the card.
3. Copy the entire **backups** folder to a safe place on your computer. Leave the original on the card.
4. Eject the card and return it to the DSpico.

For a second DSi, Studio creates another console-specific backup folder. Keep both folders; do not rename one console's backup to use it on another.

## 6. Install a game

Open Studio again and choose **Open library**. A new Studio session checks the full backup once. Later installations in that same session reuse that check.

Select your game, then tap **Copy to DSi**. Studio checks the game before showing a confirmation. Read the message, then hold **A** or the **Hold to copy** button until copying starts.

Keep the console powered on, the charger connected, and the card inserted. Wait for **Added to your DSi**. If an error appears, note its exact wording and use the [Help page](HELP.md); do not repeatedly retry a failed write.

## 7. Add another game or play

To install another game, dismiss the success message and select the next title. You do not need to restart between games. Each game has its own checks and confirmation.

When finished, power off the DSi. Remove the DSpico while the console is off, then turn the DSi on normally. Open the installed game from the DSi Home Menu. The DSpico is not needed to launch that installed game.
