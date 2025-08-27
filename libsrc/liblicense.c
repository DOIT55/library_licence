/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicense.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung (juha) <contemplation.person@gma +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:02:29 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/28 23:32:41 by HaJuYoung (juha) ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicense.h"

static License_generator_info info;

static void printError(const char* file, const int line, const char* function_name, const char* msg) {
    if (file == NULL || line == 0 || function_name == NULL) {
        printError(FLF, "Invalid error parameters");
        return;
    }
    fprintf(stderr, COLOR_RED);
    if (msg) {
        fprintf(stderr, "[%s:%d] %s: %s\n", file, line, function_name, msg);
    } else {
        fprintf(stderr, "[%s:%d] %s\n", file, line, function_name);
    }
    fprintf(stderr, COLOR_RESET);
}

static char* set_mac() {
    char mac_path[512] = {0};
    int fd = 0;
    DIR* dp = NULL;
    struct dirent* entry = NULL;

    dp = opendir(MAC_DIR);
    if (dp == NULL) {
        printError(FLF, "Failed to open MAC directory");
        return NULL;
    }

    readdir(dp);  // .
    readdir(dp);  // ..
    entry = readdir(dp);

    if (entry == NULL) {
        printError(FLF, "No MAC address found");
        return NULL;
    }

    sprintf(mac_path, "%s/%s/address", MAC_DIR, entry->d_name);
    closedir(dp);

    fd = open(mac_path, O_RDONLY);
    if (fd < 0) {
        printError(FLF, "Failed to open MAC file");
        return NULL;
    }

    if (read(fd, info.mac, sizeof(info.mac) - 1) == -1) {
        printError(FLF, "Failed to read MAC file");
        close(fd);
        return NULL;
    }

    close(fd);

    return info.mac;
}

static char* set_uuid() {
    int fd = 0;

    if (access(UUID_FILE_PATH, R_OK) != 0) {
        return NULL;
    }

    fd = open(UUID_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        printError(FLF, "Failed to open UUID file");
        return NULL;
    }

    if (read(fd, info.uuid, sizeof(info.uuid) - 1) == -1) {
        printError(FLF, "Failed to read UUID file");
        close(fd);
        return NULL;
    }
    close(fd);

    return info.uuid;
}

#if USE_NEW_SHA256

static Licence_bool set_sha256_signature() {
    EVP_MD_CTX* ctx = NULL;
    unsigned char buf[MAX_MAC_LEN + MAX_AES_BIN_LEN + MAX_UUID_LEN] = {0};
    size_t data_len = 0;

    if (info.mac[0] == '\0' || info.aes_bin[0] == '\0') {
        printError(FLF, "Missing required fields");
        return license_false;
    }

    memcpy(buf, info.mac, MAX_MAC_LEN);
    memcpy(buf + MAX_MAC_LEN, info.aes_bin, MAX_AES_BIN_LEN);
    memcpy(buf + MAX_MAC_LEN + MAX_AES_BIN_LEN, info.uuid, MAX_UUID_LEN);

    ctx = EVP_MD_CTX_new();
    if (!ctx) {
        printError(FLF, "Failed to create EVP_MD_CTX");
        return license_false;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        printError(FLF, "Failed to initialize EVP_Digest");
        goto error;
    }

    data_len = sizeof(buf);
    if (EVP_DigestUpdate(ctx, buf, data_len) != 1) {
        printError(FLF, "Failed to update EVP_Digest");
        goto error;
    }

    if (EVP_DigestFinal_ex(ctx, info.sha256, NULL) != 1) {
        printError(FLF, "Failed to finalize EVP_Digest");
        goto error;
    }

    EVP_MD_CTX_free(ctx);
    return license_true;

error:
    EVP_MD_CTX_free(ctx);
    return license_false;
}
#else

static Licence_bool set_sha256_signature() {
    SHA256_CTX sha256;
    unsigned char buf[MAX_MAC_LEN + MAX_AES_BIN_LEN + MAX_UUID_LEN] = {0};
    size_t data_len = 0;

    if (info.mac == NULL || info.aes_bin == NULL) {
        printError(FLF, "Missing required fields");
        return license_false;
    }

    memcpy(buf, info.mac, MAX_MAC_LEN);
    memcpy(buf + data_len, info.aes_bin, MAX_AES_BIN_LEN);
    memcpy(buf + data_len + MAX_AES_BIN_LEN, info.uuid, MAX_UUID_LEN);

    if (SHA256_Init(&sha256) == license_false) {
        printError(FLF, "Failed to initialize SHA256");
        return license_false;
    }
    data_len = sizeof(buf);
    if (SHA256_Update(&sha256, buf, data_len) == license_false) {
        printError(FLF, "Failed to update SHA256");
        return license_false;
    }
    if (SHA256_Final(info.sha256, &sha256) == license_false) {
        printError(FLF, "Failed to finalize SHA256");
        return license_false;
    }
    return license_true;
}
#endif

/**
 *
 *
 * @param szKey
 * @param ciphertext
 * @param ciphertext_len
 * @param plaintext
 *
 * @return plaintext_len || -1
 */
static int decryptEVP(unsigned char* szKey, unsigned char* ciphertext, int ciphertext_len, unsigned char* plaintext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len = -1;
    unsigned char iv[33] = {0};

    if (strlen((char*)szKey) <= 32) {
        printError(FLF, "Key length must be greater than 32 characters");
        return -1;
    }
    strncpy((char*)iv, (char*)szKey, 32);

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        printError(FLF, "Failed to create cipher context");
        return -1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, szKey, iv)) {
        printError(FLF, "Failed to initialize decryption");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */

    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        printError(FLF, "Failed to decrypt data");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    plaintext_len = len;

    /* Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        printError(FLF, "Failed to finalize decryption");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

static inline Licence_bool set_aes_bin(char* aes_bin) {
    if (aes_bin == NULL) {
        printError(FLF, "AES bin is NULL");
        return license_false;
    }
    memcpy(info.aes_bin, aes_bin, info.aes_len);
    return license_true;
}

void hex_dump(const void* data, size_t size, const char* label) {
    const unsigned char* bytes = (const unsigned char*)data;
    size_t i, j;

    if (!data) {
        printf("hex_dump: NULL data pointer\n");
        return;
    }

    if (label) {
        printf(COLOR_CYAN "=== %s ===" COLOR_RESET "\n", label);
    }

    printf("Size: %zu bytes\n", size);
    printf("Offset    00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ASCII\n");
    printf("--------  --------------------------------  --------------------------------  ----------------\n");

    for (i = 0; i < size; i += 16) {
        // 오프셋 출력
        printf("%08zX  ", i);

        // 헥사 값 출력 (16바이트씩)
        for (j = 0; j < 16; j++) {
            if (i + j < size) {
                printf("%02X ", bytes[i + j]);
            } else {
                printf("   ");  // 빈 공간
            }

            // 8바이트마다 구분자 추가
            if (j == 7) {
                printf(" ");
            }
        }

        printf(" ");

        // ASCII 문자 출력
        for (j = 0; j < 16 && i + j < size; j++) {
            char c = bytes[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }

        printf("\n");
    }
    printf("\n");
}

Licence_bool check_license() {
    char license_path[232] = {0};
    int fd = 0;
    char license_sha256[SHA256_DIGEST_LENGTH];
    unsigned char passphrase[65] = "5e2e2a9e3f7f5c7f7e3e3f0f3b6c6a9d1d9e9f0f7c7e7d7e6f6e6f6d6c6b6a6a";

    char buf[MAX_AES_BIN_LEN + SHA256_DIGEST_LENGTH] = {0};

    if (getenv("HOME") == NULL) {
        printError(FLF, "HOME environment variable not set");
        return license_false;
    }

    snprintf(license_path, sizeof(license_path), "%s/data/.license", getenv("HOME"));

    printf("license_path %s\n", license_path);
    fd = open(license_path, O_RDONLY);
    if (fd < 0) {
        printError(FLF, "License file not found");
        return license_false;
    }

    info.aes_len = read(fd, buf, sizeof(buf)) - SHA256_DIGEST_LENGTH;
    if (info.aes_len < 0) {
        printError(FLF, "Failed to read license file");
        goto error;
    }

    memcpy(license_sha256, buf, SHA256_DIGEST_LENGTH);

    set_aes_bin((char*)buf + SHA256_DIGEST_LENGTH);
    set_uuid();
    set_mac();
    set_sha256_signature();

    hex_dump(buf, SHA256_DIGEST_LENGTH + info.aes_len, "check license full");  // debug
    hex_dump(license_sha256, SHA256_DIGEST_LENGTH, "License File SHA256 ");    // debug
    hex_dump(info.sha256, SHA256_DIGEST_LENGTH, "create sha256");              // debug
    hex_dump(info.aes_bin, info.aes_len, "AES Bin");                           // debug

    if (memcmp(license_sha256, info.sha256, SHA256_DIGEST_LENGTH) != 0) {
        printError(FLF, "Invalid license");
        goto error;
    }

    if (decryptEVP(passphrase, info.aes_bin, MAX_AES_BIN_LEN, (unsigned char*)&(info.crypt_info)) < 0) {
        printError(FLF, "Invalid license");
        goto error;
    }

    close(fd);
    return license_true;

error:
    close(fd);
    return license_false;
}
