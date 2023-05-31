

#include <block.h>
#include <lib.h>

int block(int argsc, char* argsv[]) {
    if (argsc < 2) {
        return 1;
    }
    _sysChangeState(stringToInt(argsv[1]));

    return 0;
}
