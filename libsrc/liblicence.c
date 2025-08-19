#include "liblicence.h"

void printError(const char* file, const int line, const char* function_name, const char* msg) {
    printf(COLOR_RED);
    fprintf(stderr, "[%s:%d] %s: %s\n", file, line, function_name, msg);
    printf(COLOR_RESET);
}

/**
 * @brief you must free the returned string
 * @return string interface that separate space, if failed NULL
 */
char* get_interface_names_space_separated() {
    char buf[1024] = {0};
    DIR* dp = NULL;
    struct dirent* entry = NULL;
    // get interface name
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

int get_word_count(const char* str) {
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
    return count + 1;
}

char** create_mac_list(char* interface_names, int interface_count) {
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
            printError(FLF, "Failed to open MAC address file");
            free(mac_list);
            return NULL;
        }

        read(fd, buf, sizeof(buf) - 1);
        close(fd);

        mac_list[i] = strdup(buf);
        if (mac_list[i] == NULL) {
            printError(FLF, "Failed to duplicate string");
            free(mac_list);
            return NULL;
        }

        i++;
        token = strtok_r(NULL, " ", &savetok);
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

    interface_count = get_word_count(interface_names);

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