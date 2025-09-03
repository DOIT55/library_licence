/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   license_generator.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 22:39:22 by HaJuYoung (juha)  #+#    #+#             */
/*   Updated: 2025/09/03 15:53:28 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicense.h"

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

static void usage(const char** argv) {
    char usage_message[1024] = {0};
    int pos = 0;

    pos += sprintf(usage_message, COLOR_BOLD "\nUSAGE : %s [option]\n", argv[0]);
    pos += sprintf(usage_message + pos, "example\t: %s -y 2 -m 6 -d 3\n\n" COLOR_RESET, argv[0]);
    pos += sprintf(usage_message + pos,
                   "DESCRIPTION:"
                   "    Generates a license code based on the provided corporate email and system configuration.\n\n" COLOR_GREEN
                   "EXAMPLES:\n"
                   "    ./server/code_generator\n"
                   "        → Generates a license with unlimited expiration.\n\n"
                   "    ./server/code_generator -y 2 -m 6 -d 3\n"
                   "        → Generates a license that expires in 2 years, 6 months, and 3 days.\n\n" COLOR_RESET
                   "OPTIONS:\n"
                   "    -y <year>     Set expiration period in years.\n"
                   "    -m <month>    Set expiration period in months.\n"
                   "    -d <day>      Set expiration period in days.\n\n"
                   "NOTES:\n"
                   "    - If no options are provided, the license will have no expiration (unlimited).\n"
                   "    - You can combine -y, -m, and -d to specify a custom expiration duration.\n");
    printf("%s" COLOR_RESET, usage_message);
}

#if USE_NEW_SHA256

static License_error_code set_sha256_signature() {
    EVP_MD_CTX* ctx = NULL;
    unsigned char buf[MAX_MAC_LEN + MAX_AES_BIN_LEN + MAX_UUID_LEN] = {0};
    size_t data_len = 0;
    size_t mac_len, uuid_len;
    unsigned char *p = buf;

    if (info.mac[0] == '\0' || info.aes_bin[0] == '\0') {
        printError(FLF, "Missing required fields");
        return Sha256_invalid_parameter_error;
    }

    // Calculate actual lengths
    mac_len = strnlen(info.mac, MAX_MAC_LEN);
    uuid_len = strnlen(info.uuid, MAX_UUID_LEN);
    
    // Copy data with actual lengths
    memcpy(p, info.mac, mac_len); p += mac_len;
    memcpy(p, info.aes_bin, info.aes_len); p += info.aes_len;
    memcpy(p, info.uuid, uuid_len);
    
    data_len = mac_len + info.aes_len + uuid_len;

    ctx = EVP_MD_CTX_new();
    if (!ctx) {
        return Sha256_create_error;
    }

    if (!EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)) {
        EVP_MD_CTX_free(ctx);
        return Sha256_init_error;
    }

    if (!EVP_DigestUpdate(ctx, buf, data_len)) {
        EVP_MD_CTX_free(ctx);
        return Sha256_update_error;
    }

    if (!EVP_DigestFinal_ex(ctx, info.signature_sha256, NULL)) {
        EVP_MD_CTX_free(ctx);
        return Sha256_final_error;
    }

    EVP_MD_CTX_free(ctx);
    return License_result_success;
}
#else

static License_error_code set_sha256_signature() {
    SHA256_CTX sha256;
    unsigned char buf[MAX_MAC_LEN + MAX_AES_BIN_LEN + MAX_UUID_LEN] = {0};
    size_t data_len = 0;
    size_t mac_len, uuid_len;
    unsigned char *p = buf;

    if (info.mac[0] == '\0' || info.aes_bin[0] == '\0') {
        return Sha256_invalid_parameter_error;
    }

    // Calculate actual lengths
    mac_len = strnlen(info.mac, MAX_MAC_LEN);
    uuid_len = strnlen(info.uuid, MAX_UUID_LEN);
    
    // Copy data with actual lengths
    memcpy(p, info.mac, mac_len); p += mac_len;
    memcpy(p, info.aes_bin, info.aes_len); p += info.aes_len;
    memcpy(p, info.uuid, uuid_len);
    
    data_len = mac_len + info.aes_len + uuid_len;

    if (!SHA256_Init(&sha256)) {
        return Sha256_init_error;
    }
    if (!SHA256_Update(&sha256, buf, data_len)) {
        return Sha256_update_error;
    }
    if (!SHA256_Final(info.signature_sha256, &sha256)) {
        return Sha256_final_error;
    }
    return License_result_success;
}
#endif

/**
 *
 *
 * @param key
 * @param plaintext
 * @param plaintext_len
 * @param ciphertext
 *
 * @return
 */
static int encryptEVP(unsigned char* key, unsigned char* plaintext, int plaintext_len, unsigned char* ciphertext) {
    EVP_CIPHER_CTX* ctx = NULL;
    int len = 0;
    int ciphertext_len = 0;
    unsigned char iv[33] = {0};

    if (strlen((char*)key) < 32) {
        printError(FLF, "Key length must be greater than 32 characters");
        return -1;
    }
    strncpy((char*)iv, (char*)key, 32);

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        printError(FLF, "Failed to create cipher context");
        return -1;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        printError(FLF, "Failed to initialize encryption");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        printError(FLF, "Failed to encrypt data");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    /* Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        printError(FLF, "Failed to finalize encryption");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

/**
 * @param msg: message to log
 * @param msg_len: length of the message
 * @param fullpath: /full/path/file
 * @param option: open option
 */
static void save_file(const char* msg, int msg_len, const char* fullpath, int option) {
    int fd = -1;
    char cmd[512] = {0};
    char path[256] = {0};

    if (msg == NULL || fullpath == NULL) {
        printError(FLF, "Invalid parameters");
        return;
    }
    strncpy(path, (char*)fullpath, sizeof(path) - 1);
    dirname((char*)path);

    sprintf(cmd, "mkdir -p %s", path);
    system(cmd);
    fd = open(fullpath, option, 0644);
    if (fd == -1) {
        printError(FLF, "Failed to open  file");
        return;
    }
    if (write(fd, msg, msg_len) == -1) {
        printError(FLF, "Failed to write to file");
    }
    close(fd);
}

static char* set_mac() {
    char mac_path[512] = {0};
    int fd = 0;
    DIR* dp = NULL;
    struct dirent* entry = NULL;
    size_t mac_len;

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

    // Remove trailing newline if present
    mac_len = strnlen(info.mac, sizeof(info.mac) - 1);
    if (mac_len > 0 && info.mac[mac_len - 1] == '\n') {
        info.mac[mac_len - 1] = '\0';
    }

    return info.mac;
}

static char* set_uuid() {
    int fd = 0;
    size_t uuid_len = 0;

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

    // Remove trailing newline if present
    uuid_len = strnlen(info.uuid, sizeof(info.uuid) - 1);
    if (uuid_len > 0 && info.uuid[uuid_len - 1] == '\n') {
        info.uuid[uuid_len - 1] = '\0';
    }

    return info.uuid;
}

static License_error_code set_options(int argc, char** argv) {
    int i = 0;
    int is_option = 0;
    int require_number = 0;

    if (argv == NULL) {
        return License_invalid_parameter_error;
    }

    while (i < argc) {
        if (strncmp("-", argv[i], 1) == 0) {
            switch (argv[i][1]) {
                case 'y':
                    if (i + 1 < argc) {
                        require_number = atoi(argv[i + 1]);
                        info.crypt_info.expire_time += require_number * 365 * 24 * 60 * 60;
                    }
                    break;
                case 'm':
                    if (i + 1 < argc) {
                        info.crypt_info.expire_time += atoi(argv[i + 1]) * 30 * 24 * 60 * 60;
                    }
                    break;
                case 'd':
                    if (i + 1 < argc) {
                        info.crypt_info.expire_time += atoi(argv[i + 1]) * 24 * 60 * 60;
                    }
                    break;
                case 's':
                    printError(FLF, "test option, not use");
                    if (i + 1 < argc) {
                        info.crypt_info.expire_time += atoi(argv[i + 1]);
                    }
                    break;
            }
            is_option = 1;
            i++;
        }
        i++;
    }

    info.crypt_info.request_time = time(NULL);
    if (is_option) {
        info.crypt_info.expire_time += info.crypt_info.request_time;
        if ((info.crypt_info.request_time >= info.crypt_info.expire_time)) {
            return License_invalid_error;
        }
    } else {
        info.crypt_info.expire_time = 0;
    }

    return License_result_success;
}

int main(int argc, char** argv) {
    unsigned char passphrase[65] = "012345678901234567890123456789012";
    char buf[SHA256_DIGEST_LENGTH + MAX_AES_BIN_LEN] = {0};
    char path[232] = {0};
    char cmd[256] = {0};
    time_t create_time = 0;
    time_t expire_time = 0;

    printf("%s\n", OPENSSL_VERSION_TEXT);

    if ((argc > 7) 
    || (set_options(argc, argv) != License_result_success)) {
        usage((const char**)argv);
        return EXIT_FAILURE;
    }

    info.aes_len = encryptEVP((unsigned char*)passphrase, (unsigned char*)&info.crypt_info, sizeof(Crypt_info), info.aes_bin);

    set_uuid();
    set_mac();
    set_sha256_signature();

    memcpy(buf, info.signature_sha256, SHA256_DIGEST_LENGTH);
    memcpy(buf + SHA256_DIGEST_LENGTH, info.aes_bin, info.aes_len);

    snprintf(path, sizeof(path), "%s/data/.license", getenv("HOME"));
    save_file(buf, SHA256_DIGEST_LENGTH + info.aes_len, path, O_WRONLY | O_CREAT | O_TRUNC);

    // packed 구조체 멤버 주소 문제 해결: 임시 변수 사용
    create_time = info.crypt_info.request_time;
    expire_time = info.crypt_info.expire_time;

    printf(COLOR_GREEN "license create time : %s\n", ctime(&create_time));
    if (expire_time == 0) {
        printf("license expire time : infinite\n" COLOR_RESET);
    } else {
        printf("license expire time : %s\n" COLOR_RESET, ctime(&expire_time));
    }

    strcat(cmd, "rm -rf ");
    strcat(cmd, argv[0]);

    system(cmd);

    return EXIT_SUCCESS;
}