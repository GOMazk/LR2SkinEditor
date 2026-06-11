#include "dbgtool.h"
#include <stdio.h>

int dump(char* filename, void* from, int size){

    FILE* pFile;
    char newname[260];
    snprintf(newname,260,"%s.LR2dmp",filename);
    
    fopen_s(&pFile,filename,"wb");
    if (!pFile) return -1;

    fwrite(from, size, 1, pFile);

    fclose(pFile);

    return 0;
}