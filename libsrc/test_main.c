#include "liblicence.h"

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
            // re licence_check
            licence_check();
            licence_check_time += check_time;
        }
        now = time(NULL);
    }
}

// test
void some_main_function(int, char **, char **) {
    printf("hello world");
}

int main() {
    run_main_logic(some_main_function, 0, NULL, NULL, 3600);
    return 0;
}