/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   licence_scanner.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung (juha) <contemplation.person@gma +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 12:02:36 by HaJuYoung(juha)   #+#    #+#             */
/*   Updated: 2025/08/21 23:57:58 by HaJuYoung (juha) ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "liblicence.h"

static void printInfo(const Equipment_info *info) {
    printf(COLOR_GREEN);
    if (info->isRoot) {
        printf(COLOR_BOLD);
    }

    printf(
        "%s info \n"
        "root: %s(%d)$\n"
        "mac: %s$\n"
        "uuid: %s$\n"
        "equipment_name: %s$\n"
        "----------------------\n",
        info->isRoot ? "root" : "default",
        info->isRoot ? "true" : "false", info->isRoot,
        info->mac,
        info->uuid[0] ? info->uuid : "not set",
        info->equipment_name[0] ? info->equipment_name : "not set");

    printf(COLOR_RESET);
}

static void get_equipment_info(Equipment_info *info) {
    char **mac_list = NULL;
    char *uuid = NULL;
    int i = 0;
    char *home_path = NULL;
    char file_path[256] = {0};
    char line[256];
    char *saveptr = NULL;
    char *equipment_name = NULL;
    FILE *fp = NULL;

    home_path = getenv("IV_HOME");

    sprintf(file_path, "%s/data/sysconfig", home_path);

    fp = fopen(file_path, "r");
    if (fp == NULL) {
        printError(FLF, "Failed to open sysconfig file");
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "HOST_NAME")) {
            strtok_r(line, "= \n", &saveptr);
            equipment_name = strtok_r(NULL, "= \n", &saveptr);
            strcpy(info->equipment_name, equipment_name);
            break;
        }
    }
    fclose(fp);

    mac_list = get_mac_list();
    if (mac_list == NULL) {
        printError(FLF, "Failed to get MAC list");
        return;
    }

    while (mac_list[i] != NULL) {
        i++;
    }
    srand(time(NULL));
    strcpy(info->mac, mac_list[rand() % i]);

    uuid = get_uuid();
    if (uuid) {
        info->isRoot = true;
        strcpy(info->uuid, uuid);
        free(uuid);
    }

    i = 0;
    while (mac_list[i]) {
        free(mac_list[i]);
        i++;
    }

    free(mac_list);

    printInfo(info);
}

bool encrypt_aes256cbc(Crypt_info *crypt_info, const Equipment_info info) {
    if (!crypt_info) {
        return false;
    }

    int *p_cipher_len = &crypt_info->cipher_text_len;
    unsigned char *p_cipher_text = crypt_info->cipher_text;
    unsigned char *p_plain_text = crypt_info->plain_text;
    int *p_plain_text_len = &crypt_info->plain_text_len;
    unsigned char *p_key = crypt_info->key;

    memcpy(p_plain_text, &info, sizeof(Equipment_info));
    *p_plain_text_len = sizeof(Equipment_info);
    memcpy(p_key, PASSWORD, strlen(PASSWORD));

    *p_cipher_len = encryptEVP(p_key, p_plain_text, *p_plain_text_len, p_cipher_text);
    if (*p_cipher_len < 0) {
        printError(FLF, "Failed to encrypt data");
        return false;
    }

    return true;
}

bool create_licence(Equipment_info *equipment_info, Crypt_info *crypt_info) {
    unsigned char *p_cipher_text = NULL;
    int *len = NULL;
    unsigned char *p_licence = NULL;

    if (!equipment_info || !crypt_info) {
        return false;
    }

    p_cipher_text = crypt_info->cipher_text;
    len = &crypt_info->cipher_text_len;
    p_licence = crypt_info->licence;

    if (create_sha256_signature(equipment_info) == false) {
        printError(FLF, "Failed to create signature");
        return false;
    }
    if (encrypt_aes256cbc(crypt_info, *equipment_info) == false) {
        printError(FLF, "Failed to encrypt data");
        return false;
    }
    if (bin2hex((unsigned char *)p_cipher_text, *len, p_licence) < 0) {
        printError(FLF, "Failed to convert hex");
        return false;
    }

    return true;
}

static void usage(const char **argv, void (*printError)(const char *, const int, const char *, const char *)) {
    char usage_message[1024] = {0};
    int pos = 0;

    pos = sprintf(usage_message, "Usage: %s [corp_email_id]\n", argv[0]);
    pos = sprintf(usage_message + pos, "example: %s jy.h4456", argv[0]);
    printError(FLF, usage_message);
}

bool save_licence(Crypt_info *crypt_info, const char *path) {
    int fd = 0;

    if (!crypt_info || !path) {
        printError(FLF, "Invalid parameters for save_licence");
        return false;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printError(FLF, "Failed to open file for writing");
        return false;
    }

    if (write(fd, crypt_info->licence, strlen((char *)crypt_info->licence)) < 0) {
        printError(FLF, "Failed to write licence to file");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

int main(int argc, char **argv) {
    Licence_info licence_info = {0};
    Equipment_info *equipment_info = &licence_info.equipment_info;
    Crypt_info crypt_info = {0};
    char save_path[256] = {0};

    if (argc != 2) {
        usage((const char **)argv, printError);
        return 0;
    }
    strcpy(licence_info.request_user_name, argv[1]);
    licence_info.request_time = time(NULL);

    get_equipment_info(equipment_info);
    create_licence(equipment_info, &crypt_info);

    sprintf(save_path, "%s/%s/%s.info", getenv("IV_HOME"), "/data", equipment_info->equipment_name);
    save_licence(&crypt_info, save_path);

    printf(COLOR_BOLD COLOR_WHITE "Licence created successfully!\n" COLOR_RESET);
    // immutable file을 읽고
    // 복호화
    // 비교
    return 0;
}