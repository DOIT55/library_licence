/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicense.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:03:58 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/29 14:59:53 by HaJuYoung(juha)  ###   ########.fr       */
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

typedef struct __attribute__((__packed__)) {
    time_t request_time;
    time_t expire_time;
} Crypt_info;

typedef struct {
    Crypt_info crypt_info;
    int req_time;
    unsigned char signature_sha256[SHA256_DIGEST_LENGTH];
    unsigned char compare_sha256[SHA256_DIGEST_LENGTH];
    unsigned char aes_bin[MAX_AES_BIN_LEN];
    char uuid[MAX_UUID_LEN];
    char mac[MAX_MAC_LEN];
    int aes_len;
} License_generator_info;

extern License_generator_info info;

#define FOREACH_ERROR_CODE_LIST(_ERROR_CODE_LIST_) \
        _ERROR_CODE_LIST_(License_result_success)\
        _ERROR_CODE_LIST_(Sha256_init_error)\
        _ERROR_CODE_LIST_(Sha256_create_error)\
        _ERROR_CODE_LIST_(Sha256_update_error)\
        _ERROR_CODE_LIST_(Sha256_final_error)\
        _ERROR_CODE_LIST_(Sha256_invalid_parameter_error)\
        _ERROR_CODE_LIST_(Aes256_key_size_error)\
        _ERROR_CODE_LIST_(Aes256_new_error)\
        _ERROR_CODE_LIST_(Aes256_init_error)\
        _ERROR_CODE_LIST_(Aes256_update_error)\
        _ERROR_CODE_LIST_(Aes256_final_error)\
        _ERROR_CODE_LIST_(Env_home_error)\
        _ERROR_CODE_LIST_(License_file_not_found_error)\
        _ERROR_CODE_LIST_(License_file_read_error)\
        _ERROR_CODE_LIST_(License_invalid_error)\
        _ERROR_CODE_LIST_(Set_mac_error)\
        _ERROR_CODE_LIST_(License_invalid_parameter_error)\


#define GENERATE_ERROR_CODE_ENUM(_ERROR_CODE_LIST_ENUM_) _ERROR_CODE_LIST_ENUM_,
#define GENERATE_ERROR_CODE_STRING(_ERROR_CODE_LIST_STRING_) #_ERROR_CODE_LIST_STRING_,

typedef enum {
    FOREACH_ERROR_CODE_LIST(GENERATE_ERROR_CODE_ENUM)
    ERROR_CODE_MAX
} License_error_code;

/**
 * @brief Loads and initializes the license file required for library usage.
 *
 * This function must be called at least once before using any license-dependent
 * functionality. It performs internal initialization and validation routines,
 * including checking the existence and integrity of the license file located at:
 * `$HOME/data/.license`.
 *
 * @return License_error_code indicating success or the type of failure encountered.
 */
License_error_code load_license_file();

static inline char* license_error_code_to_string(License_error_code code) {
    int mask = code >> 31;
    char* error_code[] = {FOREACH_ERROR_CODE_LIST(GENERATE_ERROR_CODE_STRING)};

    return error_code[(code + mask) ^ mask];
}

/**
 * @brief Checks whether the current time is within the license validity period.
 *
 * This function compares the current system time with the license's expiration time
 * to determine if the license is still valid.
 *
 * @note The function load_license_file() must be called at least once before using this function,
 *       as it relies on initialized license data.
 * @see load_license_file()
 *
 * @return 1 if the license is valid (not expired), 0 otherwise.
 */

static inline int is_license_valid_period(){
    return (info.crypt_info.expire_time == 0 ||
        info.crypt_info.expire_time > time(NULL));
}

/**
 * @brief Executes the main feature operation after license verification.
 *
 * This function requires that is_license_valid() has been called at least once prior to its invocation.
 * Not verifying the license beforehand may lead to undefined behavior or security risks.
 *
 * @note The function load_license_file() must be called at least once before using this function,
 *       as it relies on initialized license data.
 * @see load_license_file()
 *
 * @return The license expiration time, or 0 if the license is infinite.
 */
static inline time_t get_license_expire_time() {
    return info.crypt_info.expire_time;
}

/**
 * @brief Performs the main feature operation.
 * @details This function depends on prior license verification.
 *          You must call is_license_valid() at least once before invoking this function.
 *          Failure to do so may result in undefined behavior or security violations.
 *
 * @note The function load_license_file() must be called at least once before using this function,
 *       as it relies on initialized license data.
 * @see load_license_file()
 *
 * @see is_license_valid()
 * @return request time
 */
static inline time_t get_license_create_date() {
    return info.crypt_info.request_time;
}

#endif