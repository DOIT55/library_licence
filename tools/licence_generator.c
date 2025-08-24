/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   licence_generator.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung (juha) <contemplation.person@gma +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 23:27:31 by HaJuYoung (juha)  #+#    #+#             */
/*   Updated: 2025/08/24 20:32:19 by HaJuYoung (juha) ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "code_generator.h"
#include "liblicence.h"

typedef struct {
    char **mac_list;
    char *host_name;
    char *uuid;
    char hash_code[SHA256_DIGEST_LENGTH * 2 + 1];
    char code_signature_row[AES_LENGTH + MAC_LENGTH + UUID_LENGTH + HOST_NAME_MAX];
    unsigned char sha256_signature[SHA256_DIGEST_LENGTH * 2 + 1];
} Licence_info;

void usage(const char **argv, void (*printError)(const char *, const int, const char *, const char *)) {
    char usage_message[1024] = {0};
    int pos = 0;

    pos = sprintf(usage_message, "Usage: %s [licence_code]\n", argv[0]);
    pos = sprintf(usage_message + pos, "example: %s 93A01F6BA.. \n", argv[0]);
    printError(FLF, usage_message);
}

static bool init_licence_info(Licence_info *licence_info, char *licence_code) {
    if (licence_info == NULL || licence_code == NULL) {
        printError(FLF, "Invalid Licence_info pointer");
        return false;
    }

    memset(licence_info, 0, sizeof(Licence_info));

    licence_info->mac_list = new_mac_list();
    licence_info->uuid = new_uuid();
    licence_info->host_name = new_host_name();
    strcpy(licence_info->hash_code, licence_code);

    sprintf(licence_info->code_signature_row, "%s%s%s%s", licence_info->host_name, licence_info->mac_list[0], licence_info->hash_code, licence_info->uuid);

    create_sha256_signature((const void *)licence_info->code_signature_row, NULL, &(licence_info->sha256_signature));

    // TODO: delete
    printf("row data : %s", licence_info->code_signature_row);

    return true;
}

static void free_allocation(Licence_info licence_info) {
    char **mac_list = licence_info.mac_list;

    while (*mac_list) {
        free(*mac_list);
        mac_list++;
    }
    free(licence_info.mac_list);
    licence_info.mac_list = NULL;
    free(licence_info.uuid);
    free(licence_info.host_name);
}

static bool create_licence_file(Licence_info *licence_info) {
    char fullpath[512] = {0};

    sprintf(fullpath, "%s/%s/%s", getenv("IV_HOME"), "data", ".licence");

    save_file(licence_info->hash_code, fullpath, "w");
    save_file("\n", fullpath, "a");
    save_file(licence_info->sha256_signature, fullpath, "a");

    return true;
}

int main(int argc, char **argv) {
    Licence_info licence_info = {0};

    if (argc != 2) {
        usage((const char **)argv, printError);
        return EXIT_FAILURE;
    }

    init_licence_info(&licence_info, argv[1]);
    create_licence_file(&licence_info);

    free_allocation(licence_info);
    return 0;
}