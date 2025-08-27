/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicense.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung (juha) <contemplation.person@gma +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:03:58 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/28 23:11:16 by HaJuYoung (juha) ###   ########.fr       */
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
#include <pthread.h>
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

#define MAX_UUID_LEN 37
#define MAX_MAC_LEN 18
#define MAX_AES_PADDING 33
#define MAX_AES_BIN_LEN (sizeof(Crypt_info) + MAX_AES_PADDING)

#define FLF __FILE__, __LINE__, __func__

#define MAC_DIR "/sys/class/net"
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
#define DEBUG_OPENSSL_VERSION()                                         \
    printf("OpenSSL Version: 0x%08lxL (%s)\n",                          \
           (unsigned long)OPENSSL_VERSION_NUMBER, OPENSSL_API_VERSION); \
    printf("USE_NEW_SHA256: %d\n", USE_NEW_SHA256);
#else
#define DEBUG_OPENSSL_VERSION()
#endif

typedef enum {
    license_false,
    license_true
} Licence_bool;

typedef struct __attribute__((__packed__)) {
    time_t request_time;
    time_t expire_time;
} Crypt_info;

typedef struct {
    Crypt_info crypt_info;
    int req_time;
    unsigned char sha256[SHA256_DIGEST_LENGTH];
    unsigned char aes_bin[MAX_AES_BIN_LEN];
    char uuid[MAX_UUID_LEN];
    char mac[MAX_MAC_LEN];
    int aes_len;
} License_generator_info;

static License_generator_info info;

Licence_bool check_license();

/**
 * @brief Executes the main feature operation after license verification.
 *
 * This function requires that check_license() has been called at least once prior to its invocation.
 * Not verifying the license beforehand may lead to undefined behavior or security risks.
 *
 * @note Always call check_license() before using this function.
 * @see check_license()
 *
 * @return The license expiration time, or 0 if the license is infinite.
 */
static inline time_t get_expire_time() {
    return info.crypt_info.expire_time;
}

/**
 * @brief Performs the main feature operation.
 * @details This function depends on prior license verification.
 *          You must call check_license() at least once before invoking this function.
 *          Failure to do so may result in undefined behavior or security violations.
 *
 * @note Call check_license() before using this function.
 *
 * @see check_license()
 * @return request time
 */
static inline time_t get_create_date() {
    return info.crypt_info.request_time;
}

#endif