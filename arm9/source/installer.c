// SPDX-License-Identifier: GPL-3.0-or-later
// DSi Studio installer. Ticket encryption and FAT12 formatting derive from NTM.
#ifdef STUDIO_INSTALLER_HOST_TEST
#include "installer_platform.h"
#else
#include "studio.h"
#include "crypto.h"
#include "sav.h"
#include <nds/arm9/sdmmc.h>
#include <mbedtls/rsa.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#endif
#include "ticket0.h"
#include "messages.h"
#include "game_files.h"
#include "storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

static unsigned char buffer[8192];
static bool exists(const char *s) { return access(s,F_OK)==0; }
static bool directory(const char *s) { struct stat st; return stat(s,&st)==0 && S_ISDIR(st.st_mode); }
static bool mkdir_ok(const char *s) { return directory(s) || mkdir(s,0777)==0; }
static bool close_write(FILE *f) { bool ok=fflush(f)==0; if(fclose(f)!=0) ok=false; return ok; }
static bool write_blob(const char *path,const void *data,size_t n) {
    if(exists(path)) return false;
    FILE *f=fopen(path,"wb"); if(!f) return false;
    bool ok=fwrite(data,1,n,f)==n; if(!close_write(f)) ok=false;
    if(!ok) return false;
    f=fopen(path,"rb"); if(!f) return false;
    size_t at=0;
    while(at<n) { size_t k=n-at; if(k>sizeof(buffer)) k=sizeof(buffer);
        if(fread(buffer,1,k,f)!=k || memcmp(buffer,(const unsigned char*)data+at,k)) { ok=false; break; } at+=k; }
    if(fclose(f)) ok=false;
    return ok;
}
static bool digest_file(const char *path,unsigned char digest[20],const char *label) {
    FILE *f=fopen(path,"rb"); if(!f) return false;
    uint64_t size=studio_size(path),done=0; mbedtls_sha1_context ctx; mbedtls_sha1_init(&ctx);
    bool ok=mbedtls_sha1_starts(&ctx)==0;
    while(ok && done<size) {
        size_t k=size-done; if(k>sizeof(buffer)) k=sizeof(buffer);
        if(fread(buffer,1,k,f)!=k || mbedtls_sha1_update(&ctx,buffer,k)) { ok=false; break; }
        done+=k;
        if((done&0x1ffff)==0 || done==size) studio_progress(label,(int)(done*100/size));
    }
    if(ferror(f)) ok=false;
    if(fclose(f)) ok=false;
    if(ok) ok=mbedtls_sha1_finish(&ctx,digest)==0;
    mbedtls_sha1_free(&ctx); return ok;
}
static bool copy_checked(const char *src,const char *dst,uint64_t n,const char *label) {
    if(exists(dst) || studio_size(src)!=n) return false;
    FILE *in=fopen(src,"rb"); if(!in) return false;
    FILE *out=fopen(dst,"wb"); if(!out) { fclose(in); return false; }
    bool ok=true; uint64_t done=0;
    while(done<n) {
        size_t k=n-done; if(k>sizeof(buffer)) k=sizeof(buffer);
        if(fread(buffer,1,k,in)!=k || fwrite(buffer,1,k,out)!=k) { ok=false; break; }
        done+=k;
        if((done&0x1ffff)==0 || done==n) studio_progress(label,(int)(done*100/n));
    }
    if(ferror(in)) ok=false;
    if(fclose(in)) ok=false;
    if(!close_write(out)) ok=false;
    return ok && studio_size(dst)==n;
}
static bool verify_tmd(const uint8_t *t) {
    // Trust the CP certificate on this stock console, never a key from the card.
    uint8_t cert[0xa00],hash[20];
    if(!studio_read("nand:/sys/cert.sys",cert,sizeof(cert)) ||
       memcmp(cert+0x840,"Root-CA00000001\0",15) ||
       memcmp(cert+0x884,"CP00000007\0",11) || studio_be32(cert+0x880)!=1) return false;
    if(mbedtls_sha1(t+0x140,0xc8,hash)) return false;
    mbedtls_rsa_context rsa; mbedtls_rsa_init(&rsa);
    bool ok=mbedtls_rsa_import_raw(&rsa,cert+0x8c8,256,NULL,0,NULL,0,NULL,0,cert+0x9c8,4)==0 &&
        mbedtls_rsa_complete(&rsa)==0 && mbedtls_rsa_check_pubkey(&rsa)==0 &&
        mbedtls_rsa_set_padding(&rsa,MBEDTLS_RSA_PKCS_V15,MBEDTLS_MD_SHA1)==0 &&
        mbedtls_rsa_pkcs1_verify(&rsa,MBEDTLS_MD_SHA1,20,hash,t+4)==0;
    mbedtls_rsa_free(&rsa); return ok;
}
static bool save_file(const StudioGame *g,const char *ext,const char *dst,uint32_t size,bool format) {
    if(!size) return true;
    char src[STUDIO_PATH]; if(!studio_sidecar(g->path,ext,src,sizeof(src))) return false;
    if(exists(src)) {
        unsigned char a[20],b[20];
        return studio_size(src)==size && digest_file(src,a,"Checking save data...") &&
            copy_checked(src,dst,size,"Copying save data...") && digest_file(dst,b,"Verifying save data...") && !memcmp(a,b,20);
    }
    if(exists(dst)) return false;
    FILE *f=fopen(dst,"wb+"); if(!f) return false;
    memset(buffer,0,sizeof(buffer)); bool ok=true;
    for(uint32_t done=0;done<size;) { size_t n=size-done; if(n>sizeof(buffer)) n=sizeof(buffer);
        if(fwrite(buffer,1,n,f)!=n) { ok=false; break; } done+=n; }
    if(ok && format) ok=initFatHeader(f);
    if(!close_write(f)) ok=false;
    return ok && studio_size(dst)==size;
}
static bool ticket(const StudioGame *g,const char *path) {
    uint8_t blob[sizeof(ticket_v0_t)+0x20]={0}; ticket_v0_t *t=(ticket_v0_t*)blob;
    t->sig_type[1]=1; t->sig_type[3]=1;
    strcpy(t->issuer,"Root-CA00000001-XS00000006");
    uint32_t high=0x30004,low=g->id;
    PUT_UINT32_BE(high,t->title_id,0); PUT_UINT32_BE(low,t->title_id,4);
    memset(t->content_access_permissions,0xff,0x20);
    return dsi_es_block_crypt(blob,sizeof(blob),ENCRYPT)==0 && write_blob(path,blob,sizeof(blob));
}
static bool usage(const char *path,uint64_t *size,uint32_t cluster,int depth) {
    if(depth>8) return false;
    DIR *d=opendir(path); if(!d) return false;
    bool ok=true; struct dirent *e;
    while(ok && (e=readdir(d))) {
        if(!strcmp(e->d_name,".") || !strcmp(e->d_name,"..")) continue;
        char p[STUDIO_PATH]; int n=snprintf(p,sizeof(p),"%s/%s",path,e->d_name);
        if(n<0 || n>=(int)sizeof(p)) { ok=false; break; }
        struct stat st; if(stat(p,&st)) { ok=false; break; }
        if(S_ISDIR(st.st_mode)) ok=usage(p,size,cluster,depth+1);
        else *size+=((uint64_t)st.st_size+cluster-1)/cluster*cluster;
    }
    closedir(d); return ok;
}
static int title_count(void) {
    const char *dirs[]={"00030004","00030005","00030015","00030017"}; int count=0;
    for(int i=0;i<4;i++) {
        char path[64]; snprintf(path,sizeof(path),"nand:/title/%s",dirs[i]);
        DIR *d=opendir(path); if(!d) return -1;
        struct dirent *e;
        while((e=readdir(d))) {
            if(e->d_name[0]=='.') continue;
            char p[STUDIO_PATH]; snprintf(p,sizeof(p),"%s/%s",path,e->d_name);
            if(directory(p)) count++;
        }
        closedir(d);
    }
    return count;
}
bool studio_storage_read(StudioStorage *storage) {
    *storage=(StudioStorage){0};
    if(!studio_nand || !studio_crypto) return false;
    struct statvfs fs;
    if(statvfs("nand:/",&fs) || !fs.f_bsize || fs.f_bsize>UINT32_MAX) return false;
    uint64_t used=0;
    int slots=title_count();
    if(slots<0 || !usage("nand:/title/00030004",&used,(uint32_t)fs.f_bsize,0)) return false;
    return studio_storage_capacity(fs.f_bavail,fs.f_bsize,used,(unsigned)slots,storage);
}
bool studio_install(StudioGame *g) {
    const char *error=NULL;
    if(!studio_nand || !studio_crypto) { studio_notice("DSi ACCESS NEEDED","Copying needs DSi NAND access. Use DSpico with a DSi-mode entry point or DSi ntrboot. Ordinary R4 launch is browse-only."); return false; }
    if(!studio_backup(true)) return false;
    // Reload everything immediately before installation; cached UI metadata is never authoritative.
    StudioGame checked={0}; uint8_t h[0x240],t[520];
    if(!studio_read(g->path,h,sizeof(h)) || !studio_header(h,sizeof(h),studio_size(g->path),&checked)) error="The game file is incomplete or unsupported.";
    snprintf(checked.path,sizeof(checked.path),"%s",g->path);
    if(!error && !studio_game_metadata(&checked,t)) error=checked.issue;
    if(!error && !(checked.regions&(1u<<studio_region))) error="This game does not support your console's region.";
    if(error) { studio_backup_reset(); studio_notice("CANNOT COPY",error); return false; }
    char dst[96],stage[96],tick[96],sticket[96],path[128];
    snprintf(dst,sizeof(dst),"nand:/title/00030004/%08lx",(unsigned long)checked.id);
    snprintf(stage,sizeof(stage),"nand:/tmp/dsistudio/%08lx",(unsigned long)checked.id);
    snprintf(tick,sizeof(tick),"nand:/ticket/00030004/%08lx.tik",(unsigned long)checked.id);
    snprintf(sticket,sizeof(sticket),"nand:/tmp/dsistudio/%08lx.tik",(unsigned long)checked.id);
    if(exists(dst)) { g->installed=true; studio_notice("ALREADY ON YOUR DSi","This title is already installed. Existing games and saves are never replaced."); return false; }
    if(exists(stage) || exists(sticket)) { studio_notice("PREVIOUS COPY FOUND","An unfinished staging folder exists. Preserve your NAND backup and inspect nand:/tmp/dsistudio before retrying. No files were changed."); return false; }
    uint32_t battery=getBatteryLevel();
    if(!(battery&BATTERY_CHARGER_CONNECTED)) { studio_notice("CONNECT POWER","Connect the charger before copying to internal storage."); return false; }
    studio_progress("Verifying Nintendo title metadata...",0);
    if(!verify_tmd(t)) { studio_backup_reset(); studio_notice("SIGNATURE CHECK FAILED","The title metadata could not be verified using this console's certificate. Copying was stopped."); return false; }
    unsigned char hash[20];
    if(!digest_file(checked.path,hash,"Checking game integrity...") || memcmp(hash,checked.hash,20)) { studio_backup_reset(); studio_notice("GAME CHECK FAILED","The ROM and .tmd do not match, or the ROM could not be read. Copy the matching files to the card again."); return false; }
    StudioStorage storage;
    if(!studio_storage_read(&storage)) { studio_backup_reset(); studio_notice("STORAGE UNAVAILABLE","Internal storage information could not be read."); return false; }
    uint64_t cluster=storage.cluster_size,needed=8*cluster;
    uint64_t parts[]={checked.size,checked.public_size,checked.private_size,(checked.appflags&4)?0x4000:0,520,sizeof(ticket_v0_t)+0x20};
    for(int i=0;i<6;i++) needed+=(parts[i]+cluster-1)/cluster*cluster;
    if(!storage.free_slots || needed>storage.free_bytes) {
        studio_notice("NOT ENOUGH SPACE","The DSi Menu needs a free title slot and enough internal storage. No files were changed."); return false;
    }
    if(!studio_confirm("COPY TO DSi MENU?",STUDIO_COPY_WARNING)) return false;
    // The user may have disconnected power while reading the confirmation.
    if(!(getBatteryLevel()&BATTERY_CHARGER_CONNECTED)) {
        studio_notice("CONNECT YOUR CHARGER","The charger was disconnected. Reconnect it and choose Copy again. Nothing was written."); return false;
    }
    // Re-read the small backup record/footer after user interaction. A changed
    // backup requires a fresh full check and a new user confirmation.
    if(!studio_backup_check_cached()) {
        studio_notice("BACKUP CHECK NEEDED","The backup changed or could not be read. Studio must check it again before copying. Nothing was written."); return false;
    }
    fifoSendValue32(FIFO_USER_02,1);
    int waited=0;
    while(!fifoCheckValue32(FIFO_USER_02) && waited++<120) swiWaitForVBlank();
    if(!fifoCheckValue32(FIFO_USER_02) || fifoGetValue32(FIFO_USER_02)!=1) {
        fifoSendValue32(FIFO_USER_02,0);
        studio_notice("COPY STOPPED","The power-button guard did not respond. No files were changed."); return false;
    }
    // No writes happen above this point. Read-only protection is restored on every exit below.
    nand_WriteProtect(false);
    bool new_ticket=false,committed=false;
    if(!mkdir_ok("nand:/tmp") || !mkdir_ok("nand:/tmp/dsistudio") || mkdir(stage,0777)) { error="Could not create the staging folder."; goto done; }
    snprintf(path,sizeof(path),"%s/content",stage); if(mkdir(path,0777)) { error="Could not create content folder."; goto done; }
    snprintf(path,sizeof(path),"%s/content/%08lx.app",stage,(unsigned long)checked.content_id);
    if(!copy_checked(checked.path,path,checked.size,"Copying game to internal storage...") ||
       !digest_file(path,hash,"Verifying the copied game...") || memcmp(hash,checked.hash,20)) { error="Game copy or read-back failed."; goto done; }
    snprintf(path,sizeof(path),"%s/content/title.tmd",stage);
    if(!write_blob(path,t,520)) { error="Metadata write or read-back failed."; goto done; }
    snprintf(path,sizeof(path),"%s/data",stage); if(mkdir(path,0777)) { error="Could not create save folder."; goto done; }
    snprintf(path,sizeof(path),"%s/data/public.sav",stage);
    if(!save_file(&checked,".pub",path,checked.public_size,true)) { error="Public save preparation failed."; goto done; }
    snprintf(path,sizeof(path),"%s/data/private.sav",stage);
    if(!save_file(&checked,".prv",path,checked.private_size,true)) { error="Private save preparation failed."; goto done; }
    if(checked.appflags&4) { snprintf(path,sizeof(path),"%s/data/banner.sav",stage);
        if(!save_file(&checked,".bnr",path,0x4000,false)) { error="Banner save preparation failed."; goto done; } }
    if(!exists(tick)) {
        if(!ticket(&checked,sticket) || !mkdir_ok("nand:/ticket") || !mkdir_ok("nand:/ticket/00030004") || rename(sticket,tick)) { error="Ticket preparation failed."; goto done; }
        new_ticket=true;
    }
    studio_progress("Adding the game to your DSi Menu...",99);
    if(exists(dst) || rename(stage,dst)) { error="Final title commit failed."; goto done; }
    committed=true; g->installed=true;
done:
    if(!committed && new_ticket) {
        // Only the ticket created by this attempt can be removed. Preserve staging for recovery.
        if(remove(tick)) error="Copy failed; a new ticket and staging files remain.";
    }
    nand_WriteProtect(true);
    fifoSendValue32(FIFO_USER_02,0);
    waited=0;
    while(!fifoCheckValue32(FIFO_USER_02) && waited++<120) swiWaitForVBlank();
    if(fifoCheckValue32(FIFO_USER_02)) fifoGetValue32(FIFO_USER_02);
    if(!committed) {
        studio_backup_reset();
        char msg[256]; snprintf(msg,sizeof(msg),"%s\nStaging files may remain in nand:/tmp/dsistudio. No existing title was replaced.",error?error:"Copy failed.");
        studio_notice("COPY STOPPED",msg);
    } else studio_notice("ADDED TO YOUR DSi", "The game was copied and checked.\n\nChoose Done to add another game. When finished, power off, remove the DSpico and restart to play.");
    return committed;
}
