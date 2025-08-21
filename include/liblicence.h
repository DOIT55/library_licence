/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicence.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung (juha) <contemplation.person@gma +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:03:58 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/21 23:37:26 by HaJuYoung (juha) ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef LIBLICENCE_H
#define LIBLICENCE_H

#include <dirent.h>
#include <fcntl.h>
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
#define INTERFACE_MAX_LENGTH 64
#define PLAINTEXT_LENGTH 1024
#define KEY_MAX_LENGTH 128
#define LICENCE_LENGTH 512
#define DETECT_STRING "42"
#define PASSWORD "he11oWor1dLicence!"
#define SIGNATURE_LENGTH SHA256_DIGEST_LENGTH

#define FLF __FILE__, __LINE__, __func__

#define MAC_DIR "/sys/class/net/"
#define UUID_FILE_PATH "/sys/class/dmi/id/product_uuid"

typedef struct {
    char interface_name[INTERFACE_MAX_LENGTH];
    char macaddress[MAC_LENGTH];
} Interface_list_info;

typedef struct {
    unsigned char key[KEY_MAX_LENGTH];
    unsigned char cipher_text[PLAINTEXT_LENGTH * 2];
    int cipher_text_len;
    unsigned char plain_text[PLAINTEXT_LENGTH];
    int plain_text_len;
    unsigned char licence[LICENCE_LENGTH];
} Crypt_info;

typedef struct __attribute__((packed)) {
    bool isRoot;
    char equipment_name[MAX_EQUIPMENT_NAME_LENGTH];
    char mac[MAC_LENGTH];
    char uuid[UUID_LENGTH];
    unsigned char signature[SIGNATURE_LENGTH];
} Equipment_info;

typedef struct {
    char request_user_name[MAX_USER_NAME_LENGTH];
    time_t request_time;
    Equipment_info equipment_info;
} Licence_info;

void printError(const char *file, const int line, const char *function_name, const char *msg);
char **get_mac_list();
char *get_uuid();

int encryptEVP(unsigned char *key, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext);
int decryptEVP(unsigned char *szKey, unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext);

bool create_sha256_signature(Equipment_info *info);
size_t hex2bin(const char *hex, unsigned char *out);
int bin2hex(const unsigned char *bin, size_t len, unsigned char *out);

#endif