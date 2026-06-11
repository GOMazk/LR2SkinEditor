#include "arr.hpp"
#include <windows.h>

int ARR::Alloc(int structsize, int size) {
    if (data) return -1; //already exist

    unitSize = structsize;

    data = malloc(unitSize * size);
    if (!data) return -1; //malloc fail
    bufSize = size;

    memset(data, 0, unitSize * size);
    return 0;
}

int ARR::Realloc(int size) { //set new max size
    void* temp = realloc(data, unitSize * size);
    if (!temp) return -1;

    data = temp;
    memset((void*)((int)data + unitSize * bufSize), 0, unitSize * (size - bufSize));
    bufSize = size;
    return 0;
}

int ARR::Free() {
    if (data) free(data);
    data = NULL;
    bufSize = 0;
    count = 0;
    return 0;
}

int ARR::push_back(void* newdata) {
    if (count >= bufSize) Realloc(bufSize * 2); //make buffer size double
    memcpy((void*)((int)data + unitSize * count), newdata, unitSize);
    count++;
    return 0;
}

bool ARR::Is_full() {
    return (count >= bufSize);
}

void* ARR::Get_new() {
    int oldcount = count;
    if (oldcount >= bufSize) Realloc(bufSize * 2); //make buffer size double
    count++;
    return (void*)((int)data + unitSize * oldcount);
}

int ARR::InsertAt(int at, void* newdata) {
    if (count+1 >= bufSize) Realloc(bufSize * 2); //make buffer size double
    for (int i = count; i >= at; i--) {
        memcpy((void*)((int)data + unitSize * (i+1)), (void*)((int)data + unitSize * i), unitSize);
    }
    memcpy((void*)((int)data + unitSize * at), newdata, unitSize);
    count++;
    return 0;
}

void* ARR::Get_newAt(int at) {
    if (count + 1 >= bufSize) Realloc(bufSize * 2); //make buffer size double
    for (int i = count; i >= at; i--) {
        memcpy((void*)((int)data + unitSize * (i + 1)), (void*)((int)data + unitSize * i), unitSize);
    }
    memset((void*)((int)data + unitSize * at), 0, unitSize);
    count++;
    return (void*)((int)data + unitSize * at);
}

int ARR::DeleteAt(int at) {
    if (at < 0 || at > count) return -1;
    for (int i = at; i < count-1; i++) {
        memcpy((void*)((int)data + unitSize * i), (void*)((int)data + unitSize * (i+1)), unitSize);
    }
    count--;
    return 0;
}


void* ARR::Get_last() {
    return (void*)((int)data + unitSize * (count-1));
}