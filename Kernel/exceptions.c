#include <video.h>
#include <time.h>
#include <idtLoader.h>

#define ZERO_EXCEPTION_ID 0

static void zero_division();

void exceptionDispatcher(int exception) {
	if (exception == ZERO_EXCEPTION_ID)
		zero_division();
}

static void zero_division() {
	scr_print("Zero Division Exception: Restarting SO...");
	int i=0;//gotta call console again
	while(1){
		sleep(500);
		scr_print("Unhandled exception");
		scr_printNewline();
		scr_printNewline();
		i++;
		if(i > 9){
			scr_clear();
			i = 0;
		}
	}
}