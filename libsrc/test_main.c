#include "liblicence.h"
// test
void some_main_function(int, char **, char **) {
    printf("hello world");
}

int main() {
    run_main_logic(some_main_function, 0, NULL, NULL, 3600);
    return 0;
}