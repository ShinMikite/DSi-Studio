// SPDX-License-Identifier: Zlib
//
// Copyright (C) 2005 Michael Noland (joat)
// Copyright (C) 2005 Jason Rogers (Dovoto)
// Copyright (C) 2005-2015 Dave Murphy (WinterMute)
// Copyright (C) 2023 Antonio Niño Díaz

// Default ARM7 core

#include <nds.h>
#include <nds/arm7/sdmmc.h>

volatile bool exit_loop = false;
volatile bool writing_nand = false;

void cid_handler(u32 value, void *unused)
{
    (void)value; (void)unused;
    u32 reply[5]={0};
    reply[0]=isDSiMode()?SDMMC_getCidRaw(1,reply+1):1;
    fifoSendDatamsg(FIFO_USER_03,sizeof(reply),(u8*)reply);
}

void writing_handler(u32 value, void *unused)
{
    (void)unused;
    writing_nand = value != 0;
    fifoSendValue32(FIFO_USER_02, value);
}

void power_button_callback(void)
{
    if (!writing_nand) exit_loop = true;
}

void vblank_handler(void)
{
    inputGetAndSend();
}

int main(int argc, char *argv[])
{
    // Read user information from the firmware (name, birthday, etc)
    readUserSettings();

    // Stop LED blinking
    ledBlink(LED_ALWAYS_ON);

    // Using the calibration values read from the firmware with
    // readUserSettings(), calculate some internal values to convert raw
    // coordinates into screen coordinates.
    touchInit();

    irqInit();
    fifoInit();

    installSystemFIFO(); // Sleep mode, storage, firmware...
    installSoundFIFO();
    fifoSetValue32Handler(FIFO_USER_02, writing_handler, NULL);
    fifoSetValue32Handler(FIFO_USER_03, cid_handler, NULL);
    if (isDSiMode())
        installCameraFIFO();

    // This sets a callback that is called when the power button in a DSi
    // console is pressed. It has no effect in a DS.
    setPowerButtonCB(power_button_callback);

    // Read current date from the RTC and setup an interrupt to update the time
    // regularly. The interrupt simply adds one second every time, it doesn't
    // read the date. Reading the RTC is very slow, so it's a bad idea to do it
    // frequently.
    initClockIRQTimer(LIBNDS_DEFAULT_TIMER_RTC);

    // Now that the FIFO is setup we can start sending input data to the ARM9.
    irqSet(IRQ_VBLANK, vblank_handler);
    irqEnable(IRQ_VBLANK);

    // Send console ID to ARM9
    // DSi Studio also runs as a read-only browser on ordinary DS-mode R4 cards.
    u64 consoleId = isDSiMode() ? getConsoleID() : 0;
    u32 *consoleId32 = (u32 *)&consoleId;
    fifoSendValue32(FIFO_USER_01, consoleId32[0]);
    fifoSendValue32(FIFO_USER_01, consoleId32[1]);

    while (!exit_loop)
    {
        const uint16_t key_mask = KEY_SELECT | KEY_START | KEY_L | KEY_R;
        uint16_t keys_pressed = ~REG_KEYINPUT;

        if (!writing_nand && (keys_pressed & key_mask) == key_mask)
            exit_loop = true;

        swiWaitForVBlank();
    }

    return 0;
}
