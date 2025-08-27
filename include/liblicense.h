/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicence.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:03:58 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/27 11:46:52 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef LIBLICENSE_H
#define LIBLICENSE_H

#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/fs.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BOLD "\033[1m"

#define MAX_USER_NAME_LENGTH 32
#define MAX_EQUIPMENT_NAME_LENGTH 64
#define UUID_LENGTH 37
#define MAC_LENGTH 18
#define AES_LENGTH 512
#define INTERFACE_MAX_LENGTH 64
#define PLAINTEXT_LENGTH 1024
#define KEY_MAX_LENGTH 128
#define LICENSE_LENGTH 512
#define HOST_NAME_MAX 64
#define SIGNATURE_LENGTH SHA256_DIGEST_LENGTH
#define UNLIMITED 0

#define FLF __FILE__, __LINE__, __func__

#define MAC_DIR "/sys/class/net/"
#define UUID_FILE_PATH "/sys/class/dmi/id/product_uuid"

#if defined(OPENSSL_VERSION_NUMBER)
    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
        #define USE_NEW_SHA256 1
        #define OPENSSL_API_VERSION "3.x (New EVP API)"
    #else
        #define USE_NEW_SHA256 0
        #define OPENSSL_API_VERSION "2.x (Legacy SHA256 API)"
    #endif
#elif defined(LIBRESSL_VERSION_NUMBER)
    #define USE_NEW_SHA256 1
    #define OPENSSL_API_VERSION "LibreSSL (EVP API)"
#else
    #define USE_NEW_SHA256 0
    #define OPENSSL_API_VERSION "Unknown (Legacy API)"
    #warning "OpenSSL version not detected, using legacy API"
#endif

#ifdef DEBUG
    #define DEBUG_OPENSSL_VERSION() \
        printf("OpenSSL Version: 0x%08lxL (%s)\n", \
               (unsigned long)OPENSSL_VERSION_NUMBER, OPENSSL_API_VERSION); \
        printf("USE_NEW_SHA256: %d\n", USE_NEW_SHA256);
#else
    #define DEBUG_OPENSSL_VERSION()
#endif

// ...existing code...

typedef struct {
    char **mac_list;
    char *host_name;
    char *uuid;
    char hex_code[SHA256_DIGEST_LENGTH * 2 + 1];
    char signature_row[AES_LENGTH + MAC_LENGTH + UUID_LENGTH + HOST_NAME_MAX];
    unsigned char aes_row[512];
    unsigned char sha256_signature[SHA256_DIGEST_LENGTH * 2 + 1];
} Licence_info;
typedef struct __attribute__((__packed__)) {
    time_t request_time;
    time_t expire_time;
} Crypt_info;

void printError(const char *file, const int line, const char *function_name, const char *msg);
char **new_mac_list();
char *new_uuid();

int encryptEVP(unsigned char *key, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext);
int decryptEVP(unsigned char *szKey, unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext);

bool create_sha256_signature(const void *signature_data, const char *add_str, unsigned char *out);
size_t hex2bin(const char *hex, unsigned char *out);
int bin2hex(const unsigned char *bin, size_t len, unsigned char *out);
void save_file(const char* msg, int msg_len, const char* fullpath, int option);
char *new_host_name();
bool init_license_info(Licence_info *license_info, char *license_code);
void run_main_logic(void (*run_main_func)(int, char **, char **), int argc, char **argv, char **envp, int check_time);
void hex_dump(const void* data, size_t size, const char* label);
time_t license_check();

#endif