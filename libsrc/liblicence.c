/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   liblicence.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:02:29 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/25 10:59:28 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicence.h"

void printError(const char* file, const int line, const char* function_name, const char* msg) {
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
    perror("INFO");
    fprintf(stderr, COLOR_RESET);
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

char** new_mac_list() {
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

char* new_uuid() {
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

// HACK : deprecated sha function 3.xx
bool create_sha256_signature(const void* signature_data, const char* add_str, unsigned char* out) {
    SHA256_CTX sha256;
    char buf[1024] = {0};
    size_t remaining = 0;

    if (!signature_data || !out) {
        printError(FLF, "Invalid signature_data pointer");
        return false;
    }

    // signature_data의 실제 길이만 복사 (문자열이라고 가정)
    size_t data_len = strlen((const char*)signature_data);
    if (data_len >= sizeof(buf)) {
        printError(FLF, "Signature data too long");
        return false;
    }

    memcpy(buf, signature_data, data_len);
    buf[data_len] = '\0';  // null terminator 추가

    if (add_str) {
        remaining = sizeof(buf) - data_len - 1;
        strncat((char*)buf, add_str, remaining);
    }

    if (SHA256_Init(&sha256) == false) {
        printError(FLF, "Failed to initialize SHA256");
        return false;
    }
    if (SHA256_Update(&sha256, buf, strlen(buf)) == false) {
        printError(FLF, "Failed to update SHA256");
        return false;
    }
    if (SHA256_Final(out, &sha256) == false) {
        printError(FLF, "Failed to finalize SHA256");
        return false;
    }
    return true;
}

int bin2hex(const unsigned char* bin, size_t len, unsigned char* out) {
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
        printError(FLF, "Key length must be greater than 16 characters");
        return -1;
    }
    strncpy((char*)iv, (char*)key, 16);
    iv[16] = 0;

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
        printError(FLF, "Key length must be greater than 16 characters");
        return -1;
    }
    strncpy((char*)iv, (char*)szKey, 16);
    iv[16] = 0;

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

/**
 * @param msg: message to log
 * @param fullpath: /full/path/file
 * @param option: fopen option
 */
void save_file(const char* msg, const char* fullpath, const char* option) {
    FILE* fp = NULL;
    char cmd[512] = {0};
    char path[256] = {0};

    if (msg == NULL || fullpath == NULL || option == NULL) {
        printError(FLF, "Invalid parameters");
        return;
    }
    strncpy(path, (char*)fullpath, sizeof(path) - 1);
    dirname((char*)path);

    sprintf(cmd, "mkdir -p %s", path);
    system(cmd);

    fp = fopen(fullpath, option);
    if (fp == NULL) {
        printError(FLF, "Failed to open log file");
        return;
    }
    fprintf(fp, "%s\n", msg);

    fclose(fp);
}

char* new_host_name() {
    char host_name[128];
    FILE* fp = NULL;

    fp = popen("grep HOST_NAME $IV_HOME/data/sysconfig | cut -d'=' -f2| tr -d [:space:]", "r");
    if (fp == NULL) {
        printError(FLF, "Failed to run command");
        return NULL;
    }

    if (fgets(host_name, sizeof(host_name) - 1, fp) == NULL) {
        printError(FLF, "Failed to read command output");
        pclose(fp);
        return NULL;
    }

    pclose(fp);

    return strdup(host_name);
}

bool init_licence_info(Licence_info* licence_info, char* licence_code) {
    if (licence_info == NULL || licence_code == NULL) {
        printError(FLF, "Invalid Licence_info pointer");
        return false;
    }

    memset(licence_info, 0, sizeof(Licence_info));

    licence_info->mac_list = new_mac_list();
    licence_info->uuid = new_uuid();
    licence_info->host_name = new_host_name();
    strcpy(licence_info->hax_code, licence_code);

    sprintf(licence_info->signature_row, "%s%s%s%s", licence_info->host_name, licence_info->mac_list[0], licence_info->hax_code, licence_info->uuid);

    create_sha256_signature((const void*)licence_info->signature_row, NULL, licence_info->sha256_signature);

    // TODO: delete
    printf("row data : %s", licence_info->signature_row);

    return true;
}

time_t licence_check() {
    Licence_info licence_info = {0};
    Crypt_info crypt_info = {0};
    char fullpath[64] = {0};
    int fd = 0;
    char buf[512] = {0};
    char sha256[SHA256_DIGEST_LENGTH * 2 + 1] = {0};
    char *newline_pos = NULL;
    char **free_ptr = NULL;

    sprintf(fullpath, "%s/%s/%s", getenv("IV_HOME"), "data", ".licence");

    fd = open(fullpath, O_RDONLY);
    if (fd < 0) {
        printError(FLF, "Failed to open licence file");
        exit(EXIT_FAILURE);
    }
    read(fd, buf, sizeof(buf) - 1);

    newline_pos = strchr(buf, '\n');
    if (newline_pos) {
        *newline_pos = '\0';
        strcpy(sha256, newline_pos + 1);
    } else {
        printError(FLF, "Invalid licence");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (!init_licence_info(&licence_info, buf)) {
        printError(FLF, "Failed to initialize licence info");
        exit(EXIT_FAILURE);
    }

    if (!memcmp(licence_info.sha256_signature, newline_pos + 1, SHA256_DIGEST_LENGTH * 2)) {
        printError(FLF, "Invalid licence");
        exit(EXIT_FAILURE);
    }

    free_ptr = licence_info.mac_list;
    while (*free_ptr) {
        free(*free_ptr);
        free_ptr++;
    }
    free(licence_info.mac_list);
    free(licence_info.host_name);
    free(licence_info.uuid);
    licence_info.mac_list = NULL;
    licence_info.host_name = NULL;
    licence_info.uuid = NULL;

    decryptEVP((unsigned char *)PASSWORD, (unsigned char *)licence_info.hax_code, strlen(licence_info.hax_code), (unsigned char *)&crypt_info);

    return crypt_info.expire_time;
}

/**
 * @param run_main_func: function pointer to run main logic
 * @param argc: argument count
 * @param argv: argument vector
 * @param envp: environment pointer
 * @param check_time: time interval for licence check (while loop)
 */
void run_main_logic(void (*run_main_func)(int, char **, char **), int argc, char **argv, char **envp, int check_time) {
    time_t now = 0;
    time_t licence_check_time = 0;
    time_t expire_date = 0;

    now = time(NULL);
    licence_check_time = now + check_time;
    expire_date = licence_check();

    if (check_time < 1 || expire_date) {
        while (true) {
            run_main_func(argc, argv, envp);
        }
    }

    while (now < expire_date) {
        run_main_func(argc, argv, envp);
        if (difftime(now, licence_check_time) >= 0) {
            licence_check();
            licence_check_time += check_time;
        }
        now = time(NULL);
    }
}

