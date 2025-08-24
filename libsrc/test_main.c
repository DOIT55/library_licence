#include "liblicence.h"

time_t licence_check() {
    time_t expire_date = 0;
    char fullpath[64] = {0};
    int fd = 0;
    char buf[512] = {0};

    sprintf(fullpath, "%s/%s/%s", getenv("IV_HOME"), "data", ".licence");

    fd = open(fullpath, O_RDONLY);
    if (fd < 0) {
        printError(FLF, "Failed to open licence file");
        exit(EXIT_FAILURE);
    }
    read(fd, buf, sizeof(buf) - 1);
    // fine \n
    // strcpy aes
    // make sha256(host_name + mac[0] + hash_code + uuid)
    // memcmp (sha256, after \n memory)
    // if not exit
    // else decode aes
    // get_expire_time
    // return expire_time

    return expire_date;
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

    if (check_time < 1) {
        while (true) {
            run_main_func(argc, argv, envp);
        }
    }

    while (now < expire_date) {
        run_main_func(argc, argv, envp);
        if (difftime(now, licence_check_time) >= 0) {
            // re licence_check
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