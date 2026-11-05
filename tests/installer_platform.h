// SPDX-License-Identifier: GPL-3.0-or-later
// Host-only substitutes for hardware and cryptography. Never used in a DSi build.
#pragma once
#include "catalog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FIFO_USER_02 2
#define BATTERY_CHARGER_CONNECTED 128
#define ENCRYPT 1
#define MBEDTLS_RSA_PKCS_V15 0
#define MBEDTLS_MD_SHA1 1
#define PUT_UINT32_BE(v,p,o) do { (p)[o]=(v)>>24; (p)[(o)+1]=(v)>>16; (p)[(o)+2]=(v)>>8; (p)[(o)+3]=(v); } while(0)
typedef int mbedtls_rsa_context;
typedef int mbedtls_sha1_context;
static bool studio_nand=true, studio_crypto=true;
static int studio_region=1;
static bool backup_ok=true, cached_ok=true, confirm_ok=true, charger=true;
static bool signature_ok=true, digest_ok=true, guard_ack=true;
static bool protected=true, guarded=false;
static unsigned unlocks, resets;
static char notice[80];
static void studio_notice(const char *title,const char *body) { (void)body; snprintf(notice,sizeof(notice),"%s",title); }
static bool studio_confirm(const char *title,const char *body) { (void)title; (void)body; return confirm_ok; }
static void studio_progress(const char *text,int percent) { (void)text; (void)percent; }
static bool studio_backup(bool required) { (void)required; return backup_ok; }
static bool studio_backup_check_cached(void) { return cached_ok; }
static void studio_backup_reset(void) { resets++; }
static uint32_t getBatteryLevel(void) { return charger?BATTERY_CHARGER_CONNECTED:0; }
static void fifoSendValue32(int channel,int value) { (void)channel; guarded=value!=0; }
static bool fifoCheckValue32(int channel) { (void)channel; return guard_ack; }
static int fifoGetValue32(int channel) { (void)channel; return guarded?1:0; }
static void swiWaitForVBlank(void) {}
static void nand_WriteProtect(bool value) { protected=value; if(!value) { if(!guarded) abort(); unlocks++; } }
static void mbedtls_sha1_init(mbedtls_sha1_context *c) { *c=0; }
static int mbedtls_sha1_starts(mbedtls_sha1_context *c) { (void)c; return 0; }
static int mbedtls_sha1_update(mbedtls_sha1_context *c,const void *p,size_t n) { (void)c; (void)p; (void)n; return digest_ok?0:-1; }
static int mbedtls_sha1_finish(mbedtls_sha1_context *c,unsigned char *h) { (void)c; memset(h,0,20); return 0; }
static void mbedtls_sha1_free(mbedtls_sha1_context *c) { (void)c; }
static int mbedtls_sha1(const void *p,size_t n,unsigned char *h) { (void)p; (void)n; memset(h,0,20); return 0; }
static void mbedtls_rsa_init(mbedtls_rsa_context *c) { *c=0; }
static int mbedtls_rsa_import_raw(mbedtls_rsa_context *c,const void *a,size_t b,const void *d,size_t e,const void *f,size_t g,const void *h,size_t i,const void *j,size_t k) { (void)c;(void)a;(void)b;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i;(void)j;(void)k;return 0; }
static int mbedtls_rsa_complete(mbedtls_rsa_context *c) { (void)c;return 0; }
static int mbedtls_rsa_check_pubkey(mbedtls_rsa_context *c) { (void)c;return 0; }
static int mbedtls_rsa_set_padding(mbedtls_rsa_context *c,int p,int h) { (void)c;(void)p;(void)h;return 0; }
static int mbedtls_rsa_pkcs1_verify(mbedtls_rsa_context *c,int h,size_t n,const void *d,const void *s) { (void)c;(void)h;(void)n;(void)d;(void)s;return signature_ok?0:-1; }
static void mbedtls_rsa_free(mbedtls_rsa_context *c) { (void)c; }
static int dsi_es_block_crypt(void *p,size_t n,int mode) { (void)p;(void)n;(void)mode;return 0; }
static bool initFatHeader(FILE *f) { (void)f; return true; }
