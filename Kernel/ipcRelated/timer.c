#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <libasm.h>
#include <constants.h>
#include <video.h>

typedef struct Timer *TimerPtr;

typedef struct Timer{
	TimerPtr next;
	
	uint64_t endInterval;
	ProcessControlBlockADT sleepingProcess;
	uint32_t pid;
} Timer;

typedef struct TimerList{
	TimerPtr head;
} TimerList;

TimerList list = { NULL };

static void insertInList(TimerPtr timer){
	TimerPtr current = list.head;
	if(current == NULL){
		list.head = timer;
		return;
	}
	TimerPtr prev = NULL;
	while(current != NULL){
		prev = current;
		current = current->next;
	}
	current = timer;
	prev->next = current;
	return;
}

static void removeFromList(TimerPtr timer){
	if(timer == NULL){
		return;
	}
	TimerPtr current = list.head;
	if(current->pid == timer->pid){
		list.head = current->next;
		current->next = NULL;
		return;
	}
	TimerPtr prev = NULL;
	while(current != NULL && current->pid !=timer->pid){
		prev = current;
		current = current->next;
	}
	if(current == NULL){
		return;
	}
	prev->next = current->next;
	return;
}

void updateTimers(uint64_t currentMillis){
	TimerPtr current = list.head;
	while(current != NULL){
		if(currentMillis >= current->endInterval){
			setProcessState(current->sleepingProcess, READY);
		}
		current = current->next;
	}
	return;
}

void sleep(uint64_t millis){
	ProcessControlBlockADT currentProcess = getCurrentProcessEntry();
	TimerPtr timer = (TimerPtr) allocTimer();
	uint64_t TSCFreq = getTSCFrequency();
	timer->endInterval = _readTimeStampCounter()/TSCFreq + millis;
	timer->sleepingProcess = currentProcess;
	timer->pid = getProcessId(currentProcess);
	timer->next = NULL;
	insertInList(timer);
	setProcessState(currentProcess, BLOCKED);
	_int20h();
	removeFromList(timer);
	freeTimer(timer);
	return;
}

uint64_t getTimerSize(){
	return sizeof(Timer);
}