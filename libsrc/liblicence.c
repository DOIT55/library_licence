/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicence.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:02:29 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/21 12:27:21 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicence.h"

void printError(const char* file, const int line, const char* function_name, const char* msg) {
    if (file == NULL || line == 0 || function_name == NULL) {
        printError(FLF, "Invalid error parameters");
        return ;
    }
    printf(COLOR_RED);
    if (msg) {
        fprintf(stderr, "[%s:%d] %s: %s\n", file, line, function_name, msg);
    } else {
        fprintf(stderr, "[%s:%d] %s\n", file, line, function_name);
    }
    perror("INFO");
    printf(COLOR_RESET);
}

/**
 * @brief you must free the returned string
 * @return string interface that separate space, if failed NULL
 */
static char* get_interface_names_space_separated() {
    char buf[1024] = {0};
    DIR* dp = NULL;
    struct dirent* entry = NULL;
    dp = opendir(MAC_DIR);
    if (dp == NULL) {
        printError(FLF, "Failed to open MAC directory");
        return NULL;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        strcat(buf, entry->d_name);
        strcat(buf, " ");
    }
    closedir(dp);

    return strdup(buf);
}

static int get_word_count_space_seperated(const char* str) {
    int count = 0;
    int i = 0;

    if (str == NULL) {
        return count;
    }

    while (str[i]) {
        if (str[i] == ' ') {
            count++;
        }
        i++;
    }
    return count;
}

static char** create_mac_list(char* interface_names, int interface_count) {
    char* savetok = NULL;
    char* token = NULL;
    char** mac_list = NULL;
    char mac_path[128] = {0};
    char buf[20] = {0};
    int fd = 0;
    int i = 0;

    mac_list = malloc(sizeof(char*) * (interface_count + 1));
    if (mac_list == NULL) {
        printError(FLF, "Failed to allocate memory for MAC list");
        return NULL;
    }
    memset(mac_list, 0, sizeof(char*) * (interface_count + 1));

    token = strtok_r(interface_names, " ", &savetok);
    while (token != NULL) {
        bzero(mac_path, sizeof(mac_path));
        sprintf(mac_path, "%s%s/address", MAC_DIR, token);

        fd = open(mac_path, O_RDONLY);
        if (fd < 0) {
            token = strtok_r(NULL, " ", &savetok);
            continue;
        }

        read(fd, buf, sizeof(buf) - 1);
        close(fd);

        mac_list[i] = strdup(buf);
        if (mac_list[i] == NULL) {
            printError(FLF, "Failed to duplicate string");
            free(mac_list);
            return NULL;
        }

        mac_list[i][17] = '\0';

        i++;
        token = strtok_r(NULL, " ", &savetok);
    }

    if (i == 0) {
        free(mac_list);
        return NULL;
    }

    return mac_list;
}

char** get_mac_list() {
    char* interface_names;
    char** mac_list = NULL;
    int interface_count = 0;

    interface_names = get_interface_names_space_separated();
    if (interface_names == NULL) {
        return mac_list;
    }

    interface_count = get_word_count_space_seperated(interface_names);

    mac_list = create_mac_list(interface_names, interface_count);

    free(interface_names);
    return mac_list;
}

char* get_uuid() {
    char* uuid = NULL;
    int fd = 0;
    char buf[37] = {0};

    if (access(UUID_FILE_PATH, R_OK) != 0) {
        printError(FLF, "UUID file not accessible");
        return NULL;
    }

    fd = open(UUID_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        printError(FLF, "Failed to open UUID file");
        return NULL;
    }

    read(fd, buf, sizeof(buf) - 1);
    close(fd);

    uuid = strdup(buf);
    if (uuid == NULL) {
        printError(FLF, "Failed to duplicate string");
        return NULL;
    }

    return uuid;
}

bool create_sha256_signature(Equipment_info* info) {
    SHA256_CTX sha256;

    if (!info) {
        return false;
    }

    strcpy(info->signature, DETECT_STRING);

    if (SHA256_Init(&sha256) == false) {
        printError(FLF, "Failed to initialize SHA256");
        return false;
    }
    if (SHA256_Update(&sha256, info, sizeof(Equipment_info)) == false) {
        printError(FLF, "Failed to update SHA256");
        return false;
    }
    if (SHA256_Final(info->signature, &sha256) == false) {
        printError(FLF, "Failed to finalize SHA256");
        return false;
    }

    return true;
}

int bin2hex(const unsigned char* bin, size_t len, char* out) {
    size_t i;

    if (bin == NULL || len == 0)
        return -1;

    for (i = 0; i < len; i++) {
        out[i * 2] = "0123456789ABCDEF"[bin[i] >> 4];
        out[i * 2 + 1] = "0123456789ABCDEF"[bin[i] & 0x0F];
    }
    out[len * 2] = '\0';

    return 0;
}

int hexchr2bin(const char hex, char* out) {
    if (out == NULL)
        return 0;

    if (hex >= '0' && hex <= '9') {
        *out = hex - '0';
    } else if (hex >= 'A' && hex <= 'F') {
        *out = hex - 'A' + 10;
    } else if (hex >= 'a' && hex <= 'f') {
        *out = hex - 'a' + 10;
    } else {
        return 0;
    }

    return 1;
}

size_t hex2bin(const char* hex, unsigned char* out) {
    size_t len;
    char b1;
    char b2;
    size_t i;

    if (hex == NULL || *hex == '\0' || out == NULL)
        return 0;

    len = strlen(hex);
    if (len % 2 != 0)
        return 0;
    len /= 2;

    memset(out, 'A', len);
    for (i = 0; i < len; i++) {
        if (!hexchr2bin(hex[i * 2], &b1) || !hexchr2bin(hex[i * 2 + 1], &b2)) {
            return 0;
        }
        (out)[i] = (b1 << 4) | b2;
    }
    return len;
}

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
int encryptEVP(unsigned char* key, unsigned char* plaintext, int plaintext_len, unsigned char* ciphertext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int ciphertext_len;
    unsigned char iv[17];

    if (strlen((char*)key) <= 16) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    strncpy((char*)iv, (char*)key, 16);
    iv[16] = 0;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        return -1;
    }
    ciphertext_len = len;

    /* Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        return -1;
    }
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

/**
 *
 *
 * @param szKey
 * @param ciphertext
 * @param ciphertext_len
 * @param plaintext
 *
 * @return
 */
int decryptEVP(unsigned char* szKey, unsigned char* ciphertext, int ciphertext_len, unsigned char* plaintext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len = -1;
    unsigned char iv[17];

    if (strlen((char*)szKey) <= 16) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    strncpy((char*)iv, (char*)szKey, 16);
    iv[16] = 0;

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, szKey, iv)) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    /* Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

/**
 * @brief you must free arr
 * @return split delim and last arr is NULL, if failed NULL
 */
char** split_string(const char* str, const char* delim) {
    char** list = NULL;
    char* savetok = NULL;
    char* token = NULL;
    char* str_copy = NULL;
    int count = 0;

    if (str == NULL) {
        printError(FLF, "Failed to duplicate string");
        return NULL;
    }

    str_copy = strdup(str);
    if (str_copy == NULL) {
        printError(FLF, "Failed to duplicate string");
        return NULL;
    }

    token = strtok_r(str_copy, delim, &savetok);
    while ((token != NULL)) {
        count++;
        token = strtok_r(NULL, delim, &savetok);
    }
    if (count == 0) {
        printError(FLF, "str not found");
        free(str_copy);
        return NULL;
    }

    list = malloc(sizeof(char*) * (count + 1));
    if (list == NULL) {
        printError(FLF, "Failed to allocate memory for list");
        free(str_copy);
        return NULL;
    }
    memset(list, 0, sizeof(char*) * (count + 1));

    free(str_copy);
    str_copy = strdup(str);
    if (str_copy == NULL) {
        printError(FLF, "Failed to duplicate string");
        free(list);
        return NULL;
    }

    count = 0;
    token = strtok_r(str_copy, delim, &savetok);
    while (token != NULL) {
        list[count] = strdup(token);
        if (list[count] == NULL) {
            printError(FLF, "fail strdup");
            int j = 0;
            for (j = 0; j < count; j++) {
                free(list[j]);
            }
            free(list);
            free(str_copy);
            return NULL;
        }

        count++;
        token = strtok_r(NULL, delim, &savetok);
    }

    free(str_copy);
    return list;
}
