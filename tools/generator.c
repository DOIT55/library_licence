#include "liblicence.h"

void printInfo(const LicenceInfo *info) {
    printf(COLOR_GREEN);
    if (info->isRoot) {
        printf(COLOR_BOLD);
    }

    printf(
        "%s info \n"
        "root: %s (%d)\n"
        "mac: %s\n"
        "uuid: %s\n"
        "----------------------\n",
        info->isRoot ? "root" : "default",
        info->isRoot ? "true" : "false", info->isRoot,
        info->mac,
        info->uuid[0] ? info->uuid : "not set");

    printf(COLOR_RESET);
}

void initInfo(LicenceInfo *info) {
    char **mac_list = NULL;
    char *uuid = NULL;
    int i = 0;

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

int main() {
    LicenceInfo info = {0};

    initInfo(&info);

    return 0;
}