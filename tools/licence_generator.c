/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   licence_generator.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 23:27:31 by HaJuYoung (juha)  #+#    #+#             */
/*   Updated: 2025/08/25 11:08:54 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "code_generator.h"
#include "liblicence.h"

static void usage(const char **argv, void (*printError)(const char *, const int, const char *, const char *)) {
    char usage_message[1024] = {0};
    int pos = 0;

    pos = sprintf(usage_message, "Usage: %s [licence_code]\n", argv[0]);
    pos = sprintf(usage_message + pos, "example: %s 93A01F6BA.. \n", argv[0]);
    printError(FLF, usage_message);
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

    save_file(licence_info->hax_code, fullpath, "w");
    save_file("\n", fullpath, "a");
    save_file((char *)licence_info->sha256_signature, fullpath, "a");

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
    
    //debug
    // char buf[SHA256_DIGEST_LENGTH * 2 + 1] = {0};
    // bin2hex(licence_info.sha256_signature, SHA256_DIGEST_LENGTH, (unsigned char *)buf);
    // printf("SHA256 Signature: %s\n", buf);

    free_allocation(licence_info);
    return 0;
}