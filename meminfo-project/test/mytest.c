#include <stdio.h>
#include "meminfo.h"

int main(int argc, char **argv) {

    char *arg = "-n";
    if(argc == 3) {
        arg = argv[2];
    }

    Mem_info *mem_info = mem_generate(argv[1], arg);

    mem_getstats(mem_info);

    print_stats(mem_info, arg);

    mem_clean(mem_info);

    return 0;
}

