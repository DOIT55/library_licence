/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   license_generator.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 23:27:31 by HaJuYoung (juha)  #+#    #+#             */
/*   Updated: 2025/08/27 13:10:04 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "code_generator.h"
#include "liblicense.h"

static void usage(const char **argv, void (*printError)(const char *, const int, const char *, const char *)) {
    char usage_message[1024] = {0};
    int pos = 0;

    pos += sprintf(usage_message,
                   "Usage: %s [license_code]\n"
                   "example: %s 93A01F6BA.. \n",
                   argv[0],
                   argv[0]);
    printError(FLF, usage_message);
}


static void free_allocation(Licence_info license_info) {
    char **mac_list = license_info.mac_list;

    while (*mac_list) {
        free(*mac_list);
        mac_list++;
    }
    free(license_info.mac_list);
    license_info.mac_list = NULL;
    free(license_info.uuid);
    free(license_info.host_name);
}

static bool create_license_file(Licence_info *license_info) {
    char fullpath[512] = {0};
    size_t len = 0;

    sprintf(fullpath, "%s/%s/%s", getenv("IV_HOME"), "data", ".license");

    save_file((char *)license_info->sha256_signature, SHA256_DIGEST_LENGTH,fullpath, O_CREAT | O_WRONLY | O_TRUNC);

    len = hex2bin((char *)license_info->hex_code, license_info->aes_row);

    save_file((char *)license_info->aes_row, len, fullpath, O_APPEND | O_WRONLY);

    return true;
}

int main(int argc, char **argv) {
    Licence_info license_info = {0};

    printf("%s\n", OPENSSL_VERSION_TEXT);
    if (argc != 2) {
        usage((const char **)argv, printError);
        return EXIT_FAILURE;
    }

    init_license_info(&license_info, argv[1]);
    create_license_file(&license_info);

    free_allocation(license_info);
    return 0;
}