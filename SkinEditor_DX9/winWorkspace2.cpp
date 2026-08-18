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

int CountCsvColumns(CSTR& line) {
    if (!line.body || line.length() <= 0) return 0;
    int count = 1;
    const char* text = line.outstr();
    for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] == ',') ++count;
    }
    return count > 30 ? 30 : count;
}

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
int WORKSPACE::FindIMG(int gr, int x, int y, int w, int h, int ifGroup) {
    int j = 0;
    for (j = 0; j < arr_IMG.count; j++) {
        IMG imgCompare = ((IMG*)arr_IMG.data)[j];
        if (imgCompare.gr == gr
            && imgCompare.x == x
            && imgCompare.y == y
            && imgCompare.w == w
            && imgCompare.h == h
            && (ifGroup < 0 || imgCompare.ifGroup == ifGroup))
            break;
    }
    return j;
}


//int WORKSPACE::MoveObject() {
//    
//    return 0;
//}

int WORKSPACE::InsertLine(int pos) {

    if (pos < 0) pos = 0;
    if (pos > skinfileLines.count) pos = skinfileLines.count;

    SKINFILELINEREAD* line = (SKINFILELINEREAD*)skinfileLines.Get_newAt(pos);
    line->line.assign("newline");
    line->isComment = true;
    line->modified = true;
    line->num = pos;

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = insertLine;
        hs->target = pos;
    }

    for (int i = 0; i < skinfileLines.count; ++i)
        ((SKINFILELINEREAD*)skinfileLines.data)[i].numTotal = i;
    for (int i = 0; i < arr_SRC.count; ++i)
        if (((SRC*)arr_SRC.data)[i].declare >= pos) ++((SRC*)arr_SRC.data)[i].declare;
    for (int i = 0; i < arr_DST.count; ++i)
        if (((DST*)arr_DST.data)[i].declare >= pos) ++((DST*)arr_DST.data)[i].declare;

    return 0;
}
int WORKSPACE::DeleteLine(int pos) {

    if (pos < 0 || pos >= skinfileLines.count) return -1;

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = removeLine;
        hs->target = pos;
        hs->older.line.assign(((SKINFILELINEREAD*)skinfileLines.data)[pos].line);
    }

    skinfileLines.DeleteAt(pos);

    for (int i = 0; i < skinfileLines.count; ++i)
        ((SKINFILELINEREAD*)skinfileLines.data)[i].numTotal = i;
    for (int i = 0; i < arr_SRC.count; ++i) {
        SRC& src = ((SRC*)arr_SRC.data)[i];
        if (src.declare == pos) src.declare = -1;
        else if (src.declare > pos) --src.declare;
    }
    for (int i = 0; i < arr_DST.count; ++i) {
        DST& dst = ((DST*)arr_DST.data)[i];
        if (dst.declare == pos) dst.declare = -1;
        else if (dst.declare > pos) --dst.declare;
    }
    preview_selected_object_model_indices.clear();
    preview_selected_object_model_index = -1;
    preview_selection_anchor_model_index = -1;
    preview_selected_obj_valid = false;
    preview_selected_obj_last_valid = false;

    return 0;
}
int WORKSPACE::EditLine(int pos, CSTR oldlinebody, CSTR newlinebody) {


    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    line.line.assign(newlinebody);

    line.isComment = (*line.line.atPos(0) != '#');
    line.isSEcomment = (*line.line.atPos(0) == '$');

    if (!line.isComment) {
        SplitCSV(line.line, &line.csv, ",");
        line.csvColumnCount = CountCsvColumns(line.line);
    } else {
        line.csvColumnCount = 0;
    }
    line.modified = true;

    HISTORY* hs = applyingHistory ? NULL : (HISTORY*)arr_history.Get_new();
    if (hs) {
        hs->op = overwriteLine;
        hs->target = pos;
    }
    CsvToLine(pos);
    if (hs) {
        hs->older.line.assign(oldlinebody);
        hs->newer.line.assign(newlinebody);
    }

    previewReloadPending = true;
    previewReloadRequestedAt = GetTickCount64();

    return 0;
}

int WORKSPACE::EditValue(int pos, int column, const char* newVal) {

    if (pos < 0 || pos >= skinfileLines.count || column < 0 || column >= 30 || newVal == NULL) return -1;

    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    CSTR oldLine(line.line);

    line.csv.str[column].assign(newVal);
    line.csv.val[column] = atol(newVal);
    if (line.csvColumnCount < column + 1) line.csvColumnCount = column + 1;
    line.modified = true;
    CsvToLine(pos);

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = overwriteLine;
        hs->target = pos;
        hs->older.line.assign(oldLine);
        hs->newer.line.assign(line.line);
    }

    previewReloadPending = true;
    previewReloadRequestedAt = GetTickCount64();

    return 0;
}
int WORKSPACE::EditValue(int pos, int column, int newVal) {

    if (pos < 0 || pos >= skinfileLines.count || column < 0 || column >= 30) return -1;

    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    CSTR oldLine(line.line);
    line.csv.str[column].resize(12);
    ltoa(newVal, line.csv.str[column], 10);
    line.csv.val[column] = newVal;
    if (line.csvColumnCount < column + 1) line.csvColumnCount = column + 1;
    line.modified = true;
    CsvToLine(pos);

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = overwriteLine;
        hs->target = pos;
        hs->older.line.assign(oldLine);
        hs->newer.line.assign(line.line);
    }

    previewReloadPending = true;
    previewReloadRequestedAt = GetTickCount64();

    return 0;
}

int WORKSPACE::UndoLastEdit() {
    if (arr_history.count <= 0) return -1;

    HISTORY& history = ((HISTORY*)arr_history.data)[arr_history.count - 1];
    const HISTORYOP operation = history.op;
    const int target = history.target;
    CSTR oldLine(history.older.line);
    --arr_history.count;

    applyingHistory = true;
    int result = 0;
    if (operation == overwriteLine) {
        if (target < 0 || target >= skinfileLines.count) result = -1;
        else {
            CSTR currentLine(((SKINFILELINEREAD*)skinfileLines.data)[target].line);
            result = EditLine(target, currentLine, oldLine);
        }
    } else if (operation == insertLine) {
        result = DeleteLine(target);
    } else if (operation == removeLine) {
        result = InsertLine(target);
        if (result == 0) {
            CSTR insertedLine(((SKINFILELINEREAD*)skinfileLines.data)[target].line);
            result = EditLine(target, insertedLine, oldLine);
        }
    } else {
        result = -1;
    }
    applyingHistory = false;

    if (result == 0) {
        g_seObjectEditorModel.Rebuild(*this);
        preview_selected_obj_valid = false;
        preview_selected_obj_last_valid = false;
        previewReloadPending = true;
        previewReloadRequestedAt = GetTickCount64();
    }
    return result;
}

int WORKSPACE::CsvToLine(int pos) {
    if (pos < 0 || pos >= skinfileLines.count) return -1;
    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];

    line.isComment = (*line.line.atPos(0) != '#');
    line.isSEcomment = (*line.line.atPos(0) == '$');

    if (line.isComment || line.isSEcomment) {
        return 0;
    }

    CSTR buf("");

    cstrSprintf(&buf, "%s", line.csv.str[0]);
    int columnCount = line.csvColumnCount;
    if (columnCount < 1) columnCount = 1;
    if (columnCount > 30) columnCount = 30;
    for (int i = 1; i < columnCount; i++) {
        const char* value = (line.csv.str[i].body == NULL) ? "" : line.csv.str[i].outstr();
        if (line.csv.val[i] < 0 && value[0] != '-' && value[0] != '!')
            cstrSprintf(&buf, "%s,!%s", buf, value);
        else
            cstrSprintf(&buf, "%s,%s", buf, value);
    }

    line.line.assign(buf);
    return 0;
}

int CsvToCSTR(CSVbuf& csv, CSTR& line) {
    CSTR buf("");
    cstrSprintf(&buf, "%s", csv.str[0]);
    int columnCount = 30;
    while (columnCount > 1 && csv.str[columnCount - 1].body == NULL) --columnCount;
    for (int i = 1; i < columnCount; i++) {
        const char* value = (csv.str[i].body == NULL) ? "" : csv.str[i].outstr();
        if (csv.val[i] < 0 && value[0] != '-' && value[0] != '!')
            cstrSprintf(&buf, "%s,!%s", buf, value);
        else
            cstrSprintf(&buf, "%s,%s", buf, value);
    }

    line.assign(buf);
    return 0;
}
