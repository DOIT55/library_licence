#pragma once

#ifndef LIBLICENCE_H
#define LIBLICENCE_H

#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

#define FLF __FILE__, __LINE__, __func__

#define MAC_DIR "/sys/class/net/"
#define UUID_FILE_PATH "/sys/class/dmi/id/product_uuid"

typedef struct {
    char interface_name[64];
    char macaddress[18];
} Interface_list_info;

typedef struct {
    bool isRoot;
    char mac[18];
    char uuid[37];
} LicenceInfo;

void printError(const char* file, const int line, const char* function_name, const char *msg);
char** get_mac_list();
char* get_uuid();
#endif