#ifndef SYNC_H
#define SYNC_H

#include <stdint.h>
#include <string.h>

uint64_t* semOpen(char* semId, uint64_t currentValue);
uint64_t semWait(char*semId);
uint64_t semPost(char*semId);
uint64_t semClose(char*semId);

#endif /* SYNC_H */