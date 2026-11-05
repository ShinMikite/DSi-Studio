// SPDX-License-Identifier: GPL-3.0-or-later
// Raw NAND backup with no$gba footer. Footer/key layout follows zoogie/dumpTool.
// This module never enables NAND writes and never restores a backup.
#include "studio.h"
#include "backup_format.h"
#include "backup_session.h"
#include "messages.h"
#include "dsi.h"
#include "audio.h"
#include <nds/arm9/sdmmc.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

extern void F_XY(uint8_t *key,const uint8_t *x,const uint8_t *y);
// Storage reads are performed by ARM7. All destination buffers must be in
// shared main RAM, never on ARM9's private DTCM stack, even if stack-aligned.
static uint8_t backup_raw_buffer[32768] __attribute__((aligned(32)));
static uint8_t backup_hardware_sector[512] __attribute__((aligned(32)));
#define work backup_raw_buffer
static BackupSession session;
static const char *progress_title="BACK UP THIS DSi";
static UiTouch cancel_touch;
bool studio_backup_ready(void) { return session.valid && !strcmp(session.root,studio_root); }
void studio_backup_reset(void) { backup_session_clear(&session); }
static bool file_stamp(const char *path,BackupStamp *stamp) {
    struct stat st;
    if(stat(path,&st) || !S_ISREG(st.st_mode)) return false;
    stamp->size=st.st_size; stamp->modified=st.st_mtime; return true;
}
static void progress(const char *body,int percent) {
    studio_music(false); ui_set_pressed(UI_NONE); ui_panel(progress_title,body,UI_PROGRESS_CANCEL,percent); studio_present();
}
static bool cancel(void) {
    scanKeys(); if(keysDown()&KEY_B) return true;
    UiAction hit=UI_NONE;
    if(keysHeld()&KEY_TOUCH) { touchPosition t; touchRead(&t); hit=ui_panel_hit(UI_PROGRESS_CANCEL,t.px,t.py); }
    return ui_touch_update(&cancel_touch,hit,keysDown()&KEY_TOUCH,keysHeld()&KEY_TOUCH,keysUp()&KEY_TOUCH)==UI_CANCEL;
}
static bool mkdir_ok(const char *path) {
    struct stat st;
    if(!stat(path,&st)) return S_ISDIR(st.st_mode);
    return mkdir(path,0777)==0;
}
static bool exists(const char *path) { return access(path,F_OK)==0; }
static bool close_checked(FILE *f) {
    bool ok=fflush(f)==0; if(fclose(f)) ok=false; return ok;
}
static bool write_new(const char *path,const void *data,size_t n) {
    if(exists(path)) return false;
    FILE *f=fopen(path,"wb"); if(!f) return false;
    bool ok=fwrite(data,1,n,f)==n; if(!close_checked(f)) ok=false;
    return ok;
}
static const char *footer_for_console(uint8_t footer[64],uint64_t *size) {
    if(!isDSiMode()) return "B01: Studio is in DS mode. Open it using your DSpico's DSi-mode launch.";
    if(!studio_nand) return "B02: Internal storage could not be mounted. Restart the console and open Studio again.";
    if(!studio_crypto) return "B03: The console identity was not available. Restart the console and try again.";
    if(!nand_Startup()) return "B04: The internal storage reader did not start.";
    *size=(uint64_t)nand_GetSectors()*512;
    if(!backup_nand_size(*size)) return "B05: This NAND size is not supported by this build. Copying is blocked.";
    while(fifoCheckDatamsg(FIFO_USER_03)) {
        uint8_t discard[32]; fifoGetDatamsg(FIFO_USER_03,sizeof(discard),discard);
    }
    if(!fifoSendValue32(FIFO_USER_03,1)) return "B06: Could not request the storage identity.";
    int frames=0; while(!fifoCheckDatamsg(FIFO_USER_03) && frames++<120) swiWaitForVBlank();
    uint32_t reply[5];
    if(!fifoCheckDatamsg(FIFO_USER_03)) return "B07: The storage identity request timed out.";
    if(fifoGetDatamsg(FIFO_USER_03,sizeof(reply),(uint8_t*)reply)!=sizeof(reply)) return "B08: The storage identity reply was incomplete.";
    if(reply[0]) return "B09: The storage identity could not be read.";
    backup_footer(footer,(uint8_t*)(reply+1),(uint8_t*)studio_console_id);
    // Verify that the footer decrypts the real raw first sector correctly.
    uint8_t iv[20],key[16],plain[512];
    uint32_t x[4]={studio_console_id[0],studio_console_id[0]^0x24ee6906,
                  studio_console_id[1]^0xe65b601d,studio_console_id[1]};
    const uint8_t y[16]={0x76,0xdc,0xb9,0x0a,0xd3,0xc4,0x4d,0xbd,0x1d,0xdd,0x2d,0x20,0x05,0x00,0xa0,0xe1};
    if(mbedtls_sha1(footer+16,16,iv)) return "B10: The backup identity checksum failed.";
    if(!nand_ReadSectors(0,1,work)) return "B11: The first raw NAND sector could not be read.";
    if(!nand_ReadSectorsCrypt(0,1,backup_hardware_sector)) return "B12: The first decrypted NAND sector could not be read.";
    DC_InvalidateRange(work,512);
    DC_InvalidateRange(backup_hardware_sector,512);
    F_XY(key,(uint8_t*)x,y);
    dsi_context ctx; dsi_init_ctr(&ctx,key,iv); dsi_crypt_ctr(&ctx,work,plain,512);
    if(plain[510]!=0x55 || plain[511]!=0xaa) return "B13: The backup identity could not decrypt the NAND header. Copying is blocked.";
    if(memcmp(plain,backup_hardware_sector,512)) return "B14: Software and hardware NAND checks disagree. Copying is blocked.";
    return NULL;
}
static bool hash_file(const char *path,uint64_t size,uint8_t digest[32]) {
    if(studio_size(path)!=size) return false;
    FILE *f=fopen(path,"rb"); if(!f) return false;
    mbedtls_sha256_context ctx; mbedtls_sha256_init(&ctx);
    bool ok=mbedtls_sha256_starts(&ctx,0)==0;
    int last=-1;
    for(uint64_t done=0;ok && done<size;) {
        size_t n=size-done; if(n>sizeof(work)) n=sizeof(work);
        if(fread(work,1,n,f)!=n || mbedtls_sha256_update(&ctx,work,n)) { ok=false; break; }
        done+=n; int percent=(int)(done*100/size);
        if(percent!=last) { progress("Checking every byte of your backup...",percent); last=percent; }
        if(cancel()) { ok=false; break; }
    }
    if(ok) ok=mbedtls_sha256_finish(&ctx,digest)==0;
    mbedtls_sha256_free(&ctx); if(fclose(f)) ok=false; return ok;
}
static bool footer_matches(const char *path,uint64_t size,const uint8_t footer[64]) {
    FILE *f=fopen(path,"rb"); if(!f) return false;
    uint8_t found[64];
    bool ok=fseek(f,(long)size,SEEK_SET)==0 && fread(found,1,64,f)==64 && !memcmp(found,footer,64);
    if(fclose(f)) ok=false;
    return ok;
}
bool studio_backup_check_cached(void) {
    if(!studio_backup_ready()) return false;
    char file[192],meta[192]; uint8_t record[112]; BackupStamp stamp={0};
    snprintf(file,sizeof(file),"%sDSiStudio/backups/%08lx%08lx/nand.bin",studio_root,
             (unsigned long)studio_console_id[1],(unsigned long)studio_console_id[0]);
    snprintf(meta,sizeof(meta),"%sDSiStudio/backups/%08lx%08lx/backup.meta",studio_root,
             (unsigned long)studio_console_id[1],(unsigned long)studio_console_id[0]);
    bool ok=studio_size(meta)==sizeof(record) && studio_read(meta,record,sizeof(record)) &&
            file_stamp(file,&stamp) && backup_session_matches(&session,studio_root,record,stamp) &&
            stamp.size>=64 && footer_matches(file,stamp.size-64,record+16);
    if(!ok) studio_backup_reset();
    return ok;
}
static bool choice(const char *body) {
    return studio_choice("FIRST: BACK UP THIS DSi",body);
}
bool studio_backup(bool required) {
    // Present a complete frame before synchronous storage/FIFO work.
    studio_music(false); ui_set_pressed(UI_NONE);
    ui_panel("PREPARING BACKUP","Checking this DSi and your card...\n\nYour console has not been changed.",UI_PROGRESS,-1);
    studio_present();
    nand_WriteProtect(true);
    cancel_touch=(UiTouch){0};
    if(!required) studio_backup_reset(); // The Backup button always requests a full check.
    uint8_t footer[64]; uint64_t size;
    const char *setup_error=footer_for_console(footer,&size);
    if(setup_error) {
        studio_backup_reset();
        studio_notice("BACKUP NOT AVAILABLE",setup_error); return false;
    }
    char base[128],dir[160],file[192],meta[192],part[192],partmeta[192];
    snprintf(base,sizeof(base),"%sDSiStudio/backups",studio_root);
    snprintf(dir,sizeof(dir),"%s/%08lx%08lx",base,(unsigned long)studio_console_id[1],(unsigned long)studio_console_id[0]);
    snprintf(file,sizeof(file),"%s/nand.bin",dir); snprintf(meta,sizeof(meta),"%s/backup.meta",dir);
    snprintf(part,sizeof(part),"%s/nand.partial",dir); snprintf(partmeta,sizeof(partmeta),"%s/backup.partial",dir);
    if(exists(file) || exists(meta)) {
        uint8_t record[112],digest[32];
        BackupStamp before={0},after={0};
        bool ok=studio_size(meta)==sizeof(record) && studio_read(meta,record,sizeof(record)) &&
                backup_record_matches(record,sizeof(record),size,footer) &&
                file_stamp(file,&before) && before.size==size+64 && footer_matches(file,size,footer);
        if(ok && required && backup_session_matches(&session,studio_root,record,before)) return true;
        studio_backup_reset();
        progress_title="CHECK BACKUP";
        if(ok) ok=hash_file(file,size+64,digest) && !memcmp(digest,record+80,32) &&
                  file_stamp(file,&after) && before.size==after.size && before.modified==after.modified;
        if(!ok) {
            studio_notice("BACKUP CHECK STOPPED","The backup check failed or was canceled. Existing files were kept. Copy DSiStudio/backups to your PC for inspection before trying again."); return false;
        }
        backup_session_store(&session,studio_root,record,after);
        if(!required) studio_notice("BACKUP VERIFIED","Your backup matches this DSi and passed its full file check. Copy the DSiStudio/backups folder to your PC for safekeeping.");
        return true;
    }
    studio_backup_reset();
    if(!choice(STUDIO_BACKUP_INTRO)) return false;
    if(!(getBatteryLevel()&BATTERY_CHARGER_CONNECTED)) {
        studio_notice("CONNECT YOUR CHARGER","Connect the DSi charger, then choose Back up this DSi again."); return false;
    }
    struct statvfs fs;
    if(statvfs(studio_root,&fs) || !fs.f_bsize || (uint64_t)fs.f_bavail*fs.f_bsize<size+64+1024*1024) {
        studio_notice("CARD NEEDS MORE SPACE","Free at least 260 MB on the selected card, then try again. L/R changes the selected card."); return false;
    }
    char parent[64]; snprintf(parent,sizeof(parent),"%sDSiStudio",studio_root);
    if(!mkdir_ok(parent) || !mkdir_ok(base) || !mkdir_ok(dir)) {
        studio_notice("CANNOT SAVE BACKUP","The backup folder could not be created. Check that the selected card is writable."); return false;
    }
    // Only unfinished files owned by this flow can be removed on retry.
    if((exists(part) && remove(part)) || (exists(partmeta) && remove(partmeta))) {
        studio_notice("CANNOT RETRY BACKUP","An unfinished backup could not be cleared. Reconnect the card to your PC and check it."); return false;
    }
    FILE *f=fopen(part,"wb");
    if(!f) { studio_notice("CANNOT SAVE BACKUP","Could not create a backup on the selected card."); return false; }
    mbedtls_sha256_context ctx; mbedtls_sha256_init(&ctx);
    progress_title="BACK UP THIS DSi";
    bool ok=mbedtls_sha256_starts(&ctx,0)==0;
    const char *error="A read or card write failed. Your DSi was not changed.";
    int last=-1;
    for(uint64_t done=0;ok && done<size;) {
        size_t n=size-done; if(n>sizeof(work)) n=sizeof(work);
        if(!nand_ReadSectors((sec_t)(done/512),n/512,work) || fwrite(work,1,n,f)!=n || mbedtls_sha256_update(&ctx,work,n)) { ok=false; break; }
        done+=n; int percent=(int)(done*100/size);
        if(percent!=last) { progress("Saving this DSi to your card...",percent); last=percent; }
        if(cancel()) { ok=false; error="Backup canceled. Your DSi was not changed. You can try again whenever you are ready."; }
    }
    uint8_t expected[32],actual[32],record[112],readback[112];
    if(ok) ok=fwrite(footer,1,64,f)==64 && !mbedtls_sha256_update(&ctx,footer,64) && !mbedtls_sha256_finish(&ctx,expected);
    mbedtls_sha256_free(&ctx); if(!close_checked(f)) ok=false;
    if(ok) {
        error="Backup verification failed or was canceled. No backup was marked complete. Please check the card and try again.";
        progress_title="CHECK BACKUP";
        ok=footer_matches(part,size,footer) && hash_file(part,size+64,actual) && !memcmp(expected,actual,32);
    }
    if(ok) {
        backup_record(record,size,footer,expected);
        ok=write_new(partmeta,record,sizeof(record)) && studio_size(partmeta)==sizeof(record) &&
           studio_read(partmeta,readback,sizeof(readback)) && !memcmp(record,readback,sizeof(record));
    }
    if(!ok) {
        remove(part); remove(partmeta); studio_notice("BACKUP NOT COMPLETED",error); return false;
    }
    // Keep the image if the second rename fails. Never delete a complete backup.
    if(rename(part,file) || rename(partmeta,meta)) {
        studio_notice("BACKUP NEEDS ATTENTION","The final backup files could not be saved together. Copy DSiStudio/backups to your PC for inspection. Installation is blocked."); return false;
    }
    BackupStamp saved;
    if(file_stamp(file,&saved)) backup_session_store(&session,studio_root,record,saved);
    studio_notice("BACKUP SAVED AND CHECKED","Your DSi backup is ready. Copy the DSiStudio/backups folder to your PC for safekeeping.\n\nReturn to your collection when done.");
    // Creating the backup never flows directly into NAND installation.
    return false;
}
