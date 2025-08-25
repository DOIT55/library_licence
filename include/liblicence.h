/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicence.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:03:58 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/25 14:11:47 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef LIBLICENCE_H
#define LIBLICENCE_H

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
#define LICENCE_LENGTH 512
#define HOST_NAME_MAX 64
#define PASSWORD "he11oWor1dLicence!"
#define SIGNATURE_LENGTH SHA256_DIGEST_LENGTH

#define FLF __FILE__, __LINE__, __func__

#define MAC_DIR "/sys/class/net/"
#define UUID_FILE_PATH "/sys/class/dmi/id/product_uuid"
#if OPENSSL_VERSION_NUMBER < 30000000L && !defined(LIBRESSL_VERSION_NUMBER)
    #define USE_NEW_SHA256 0
#else
    #define USE_NEW_SHA256 1
#endif
typedef struct {
    char **mac_list;
    char *host_name;
    char *uuid;
    char hax_code[SHA256_DIGEST_LENGTH * 2 + 1];
    char signature_row[AES_LENGTH + MAC_LENGTH + UUID_LENGTH + HOST_NAME_MAX];
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
void save_file(const char *msg, const char *fullpath, const char *option);
char *new_host_name();
bool init_licence_info(Licence_info *licence_info, char *licence_code);
void run_main_logic(void (*run_main_func)(int, char **, char **), int argc, char **argv, char **envp, int check_time);

#endif