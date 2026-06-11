#include "imgui/imgui.h"

#include "../LR2/structure.h"
#include "../LR2/LR2_skinmanage.h"
#include "../LR2/LR2_skinload.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/LR2_configsave.h"
#include "../LR2/En_fileutil.h"
#include "../LR2/Scene07_Skinselect.h"
#include "../LR2/En_timer.h"
#include "../LR2/En_value.h" //for ByTime

#include "../lib/DxLib/DxLib.h"
#include "winWorkspace.h"

#include "skin.h"
#include "op.h"
#include "arr.hpp"
#include "seHelper.h"

//TODO apply split
int WORKSPACE::SaveSkinScript(char* path, bool split, bool nocomment) {

    FILE* pFile;
    pFile = fopen(path, "wb");
    if (pFile == NULL) return -1;
    for (int i = 0; i < skinfileLines.count; i++) {
        CsvToLine(i);
        if (nocomment && ((SKINFILELINEREAD*)skinfileLines.data)[i].isComment) continue;
        fputs(((SKINFILELINEREAD*)skinfileLines.data)[i].line, pFile);
        fputs("\n", pFile);
    }
    fclose(pFile);
    return 0;
}

//save for object mode
int WORKSPACE::SaveSkinScript2(char* path, bool split, bool nocomment) {

    FILE* pFile;
    pFile = fopen(path, "wb");
    if (pFile == NULL) return -1;
    for (int i = 0; i < arr_seobj.count; i++) {
        SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[i];
        for (int k = 0; k < seobj.bodyCSV.count; k++) {
            CSVbuf& csv = ((CSVbuf*)seobj.bodyCSV.data)[k];
            CSTR buf("");
            CsvToCSTR(csv, buf);
            fputs(buf, pFile);
            fputs("\n", pFile);
        }
    }
    fclose(pFile);
    return 0;
}