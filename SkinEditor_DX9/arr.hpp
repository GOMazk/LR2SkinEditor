#pragma once

#include <stddef.h>

typedef struct ARR {
    void* data = NULL;
    int count = 0;
    int bufSize = 0;
    //
    int unitSize = 0;

    int Alloc(int structsize, int size);
    int Realloc(int size);
    int Free();

    int push_back(void* newdata);
    bool Is_full();
    void* Get_new();
    int InsertAt(int at, void* newdata);
    void* Get_newAt(int at);
    int DeleteAt(int at);

    void* Get_last();
}ARR;