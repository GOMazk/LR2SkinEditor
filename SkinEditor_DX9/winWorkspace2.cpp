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

int WORKSPACE::NewIMG(int gr, int x, int y, int w, int h) {
    IMG* img = (IMG*)arr_IMG.Get_new();
    img->name = "noname";
    img->gr = gr;
    img->x = x;
    img->y = y;
    img->w = w;
    img->h = h;

    //TODO:history here

    return 0;
}

int WORKSPACE::DeleteIMG(int pos) {
    arr_IMG.DeleteAt(pos);

    //TODO:history here

    return 0;
}

int WORKSPACE::ModifyIMG(int pos, int gr, int x, int y, int w, int h) {

    IMG& img = ((IMG*)arr_IMG.data)[pos];

    img.gr = gr;
    img.x = x;
    img.y = y;
    img.w = w;
    img.h = h;
    //TODO:history here

    return 0;
}

//check duplicated
int WORKSPACE::FindIMG(int gr, int x, int y, int w, int h) {
    int j = 0;
    for (j = 0; j < arr_IMG.count; j++) {
        IMG imgCompare = ((IMG*)arr_IMG.data)[j];
        if (imgCompare.gr == gr
            && imgCompare.x == x
            && imgCompare.y == y
            && imgCompare.w == w
            && imgCompare.h == h)
            break;
    }
    return j;
}


//int WORKSPACE::MoveObject() {
//    
//    return 0;
//}

int WORKSPACE::InsertLine(int pos) {

    SKINFILELINEREAD* line = (SKINFILELINEREAD*)skinfileLines.Get_newAt(pos);
    line->line.assign("newline");
    line->isComment = true;
    line->num = pos;

    HISTORY* hs = (HISTORY*)arr_history.Get_new();
    hs->op = insertLine;
    hs->target = pos;

    return 0;
}
int WORKSPACE::DeleteLine(int pos) {

    skinfileLines.DeleteAt(pos);

    HISTORY* hs = (HISTORY*)arr_history.Get_new();
    hs->op = removeLine;;
    hs->target = pos;

    return 0;
}
int WORKSPACE::EditLine(int pos, CSTR oldlinebody, CSTR newlinebody) {


    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    line.line.assign(newlinebody);

    line.isComment = (*line.line.atPos(0) != '#');
    line.isSEcomment = (*line.line.atPos(0) == '$');

    if (!line.isComment) SplitCSV(line.line, &line.csv, ",");

    HISTORY* hs = (HISTORY*)arr_history.Get_new();
    hs->op = overwriteLine;
    hs->target = pos;
    CsvToLine(pos);
    hs->older.line.assign(oldlinebody);
    hs->newer.line.assign(newlinebody);

    return 0;
}

int WORKSPACE::EditValue(int pos, int column, const char* newVal) {

    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    CSTR oldLine(line.line);

    line.csv.str[column].assign(newVal);
    line.csv.val[column] = atol(newVal);
    CsvToLine(pos);

    HISTORY* hs = (HISTORY*)arr_history.Get_new();
    hs->op = overwriteLine;
    hs->target = pos;
    hs->older.line.assign(oldLine);
    hs->newer.line.assign(line.line);

    return 0;
}
int WORKSPACE::EditValue(int pos, int column, int newVal) {

    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    CSTR oldLine(line.line);
    line.csv.str[column].resize(12);
    ltoa(newVal, line.csv.str[column], 10);
    line.csv.val[column] = newVal;
    CsvToLine(pos);

    HISTORY* hs = (HISTORY*)arr_history.Get_new();
    hs->op = overwriteLine;
    hs->target = pos;
    hs->older.line.assign(oldLine);
    hs->newer.line.assign(line.line);

    return 0;
}

int WORKSPACE::CsvToLine(int pos) {
    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];

    line.isComment = (*line.line.atPos(0) != '#');
    line.isSEcomment = (*line.line.atPos(0) == '$');

    if (line.isComment || line.isSEcomment) {
        return 0;
    }

    //TODO : truncate ','
    CSTR buf("");

    cstrSprintf(&buf, "%s", line.csv.str[0]);
    for (int i = 1; i < 25; i++) {
        cstrSprintf(&buf, "%s,%s", buf, (line.csv.str[i].body == NULL) ? "" : line.csv.str[i].outstr());
    }

    line.line.assign(buf);
    return 0;
}

int CsvToCSTR(CSVbuf& csv, CSTR& line) {
    CSTR buf("");
    cstrSprintf(&buf, "%s", csv.str[0]);
    for (int i = 1; i < 25; i++) {
        cstrSprintf(&buf, "%s,%s", buf, (csv.str[i].body == NULL) ? "" : csv.str[i].outstr());
    }

    line.assign(buf);
    return 0;
}