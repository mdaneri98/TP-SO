


static void sleep(int argc, char* argsv[]) {
    unsigned long millis = 0;
    if(argc == 0)
        millis = 2000;
    else if(*argsv[0] == '0' && *(argsv + 1) == 0)
        millis = 0;
    else{
        millis = stringToNum(argsv[0][0]);
        if(millis == 0){
            char *argErr = "You can enter only one number.";
            printString(argErr);
            putChar('\n');
            return;
        }
    }
    _sleep(millis);
}
