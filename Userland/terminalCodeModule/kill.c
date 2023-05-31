

#include <kill.h>
#include <lib.h>


int kill(int argsc, char* argsv[]) {
    if (argsc < 2) {
        return 1;
    }
    _sysKill(stringToInt(argsv[1]));

    return 0;
}
