/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   code_generator.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: HaJuYoung(juha) <jy.h4456@arielnetworks.co +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 22:39:22 by HaJuYoung (juha)  #+#    #+#             */
/*   Updated: 2025/08/26 11:51:02 by HaJuYoung(juha)  ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "code_generator.h"

#include "liblicence.h"

static void usage(const char** argv) {
    char usage_message[1024] = {0};
    int pos = 0;

    pos += sprintf(usage_message, "%s [corp_email_id] [sysconfig_host_name][option]\n", argv[0]);
    pos += sprintf(usage_message + pos, "example\t: %s jy.h4456 emgcA -y 2 -m 6 -d 3\n", argv[0]);
    pos += sprintf(usage_message + pos, "\nOption\n"
                                        "\t-y : expire year\n"
                                        "\t-m : expire month\n"
                                        "\t-d : expire day\n");
    printf(COLOR_RED"Usage\t: %s"COLOR_RESET, usage_message);
}

static bool gen_generator_msg(const Code_generator_info* info, char* msg, size_t msg_size) {
    time_t request_time = info->crypt_info.request_time;
    time_t expire_time = info->crypt_info.expire_time;
    struct tm* request_timeinfo = NULL;
    struct tm* expire_timeinfo = NULL;
    char request_time_str[26] = {0};
    char expire_time_str[26] = {0};

    if (info == NULL || msg == NULL) {
        printError(FLF, "Invalid parameters");
        return false;
    }

    request_timeinfo = localtime(&request_time);
    strftime(request_time_str, sizeof(request_time_str), "%Y-%m-%d %H:%M:%S", request_timeinfo);

    expire_timeinfo = localtime(&expire_time);
    strftime(expire_time_str, sizeof(expire_time_str), "%Y-%m-%d %H:%M:%S", expire_timeinfo);
    if (expire_time == 0) {
        strcpy(expire_time_str, "no limit");
    }

    snprintf(msg, msg_size,
             "---------------------------- [Licence Information] -------------------------------------\n"
             "User name: %s\n"
             "Equipment host name: %s\n"
             "Request time: %s\n"
             "Expire time: %s\n"
             "Licence code: %s\n"
             "----------------------------------------------------------------------------------------\n",
             info->request_user_name,
             info->equipment_name,
             request_time_str,
             expire_time_str,
             info->licence_code);
    return true;
}

static void print_code_generator_info(const Code_generator_info* gen_info) {
    char msg[2048] = {0};

    if (gen_info == NULL) {
        printError(FLF, "Invalid Code_generator_info pointer");
        return;
    }
    gen_generator_msg(gen_info, msg, sizeof(msg));
    printf(COLOR_GREEN "%s" COLOR_RESET, msg);
}

static bool set_code_generator_info(Code_generator_info* info, int argc, char** argv) {
    int i = 0;
    int is_option = 0;

    if (info == NULL || argv == NULL || argv[1] == NULL || argv[2] == NULL) {
        printError(FLF, "Invalid parameters");
        return false;
    }

    memset(info, 0, sizeof(Code_generator_info));

    while (i < argc) {
        if (strncmp("-", argv[i], 1) == 0) {
            switch (argv[i][1]) {
                case 'y':
                    if (i + 1 < argc) {
                        info->crypt_info.expire_time += atoi(argv[i + 1]) * 365 * 24 * 60 * 60;
                    }
                    break;
                case 'm':
                    if (i + 1 < argc) {
                        info->crypt_info.expire_time += atoi(argv[i + 1]) * 30 * 24 * 60 * 60;
                    }
                    break;
                case 'd':
                    if (i + 1 < argc) {
                        info->crypt_info.expire_time += atoi(argv[i + 1]) * 24 * 60 * 60;
                    }
                    break;
                case 's':
                    printError(FLF, "test option, not use");
                    if (i + 1 < argc) {
                        info->crypt_info.expire_time += atoi(argv[i + 1]);
                    }
                    break;
            }
            is_option = 1;
        } else {
            if (info->request_user_name[0] == '\0') {
                strncpy(info->request_user_name, argv[i], MAX_USER_NAME_LENGTH - 1);
            } else {
                strncpy(info->equipment_name, argv[i], MAX_EQUIPMENT_NAME_LENGTH - 1);
            }
        }
        i++;
    }

    info->crypt_info.request_time = time(NULL);
    if (is_option) {
        info->crypt_info.expire_time += info->crypt_info.request_time;
        if ((info->crypt_info.request_time >= info->crypt_info.expire_time)) {
            printError(FLF, "Invalid expire time");
            exit(EXIT_FAILURE);
        }
    } else {
        info->crypt_info.expire_time = 0;
    }


    return true;
}

static bool create_code(Code_generator_info* info) {
    char buf[1024] = {0};
    int len = 0;

    if (info == NULL) {
        printError(FLF, "Invalid Code_generator_info pointer");
        return false;
    }

    len = encryptEVP((unsigned char*)PASSWORD, (unsigned char*)&(info->crypt_info), sizeof(info->crypt_info), (unsigned char*)buf);
    if (len < 0) {
        return false;
    }

    if (bin2hex((unsigned char*)buf, len, info->licence_code) < 0) {
        return false;
    }

    return true;
}

static void save_code_generator_log_file(const Code_generator_info* info) {
    char fullpath[512] = {0};
    struct tm* tm_info = NULL;
    char time_str[64] = {0};
    char buf[512] = {0};
    time_t tmp_time;

    if (info == NULL) {
        printError(FLF, "Invalid Code_generator_info pointer");
        return;
    }

    tmp_time = info->crypt_info.request_time;
    tm_info = localtime(&tmp_time);
    time_str[strftime(time_str, sizeof(time_str), "%Y%m%d", tm_info)] = 0;

    snprintf(fullpath, sizeof(fullpath), "%s/%s/%s%s.log",
             getenv("IV_HOME"), "log/CODE_GENERATOR/", "code_generator_", time_str);

    gen_generator_msg(info, buf, sizeof(buf));
    save_file(buf, strlen(buf), fullpath, O_CREAT | O_APPEND | O_WRONLY);
}

int main(int argc, char** argv) {
    Code_generator_info licence_info;

    if ((2 < argc && argc <  10) == false) {
        usage((const char**)argv);
        return EXIT_FAILURE;
    }

    if (!set_code_generator_info(&licence_info, argc, argv)) {
        printError(FLF, "Failed to set generator info");
        return EXIT_FAILURE;
    }

    if (!create_code(&licence_info)) {
        printError(FLF, "Failed to create licence");
        return EXIT_FAILURE;
    }
    save_code_generator_log_file(&licence_info);
    print_code_generator_info(&licence_info);
    return EXIT_SUCCESS;
}