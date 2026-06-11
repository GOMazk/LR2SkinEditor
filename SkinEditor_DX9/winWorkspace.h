#pragma once
#include "../LR2/structure.h"
#include "arr.hpp"
#include <imgui.h>
#include "ImageLoader.h"

typedef struct SKINFILELINEREAD {
    int numTotal;

    CSTR filename;
    int num;

    CSTR line;
    bool isComment;
    bool isSEcomment;
    bool show = true;

    CSVbuf csv;

    int ifgroup;

    bool isIfGroupHead = false;
    bool isIfGroupEnd = false;
    bool isGroupHead = false;
    bool isGroupEnd = false;
    bool isObjectHead = false;
    bool isObjectEnd = false;

    int objID;
    int objType;
    int objInTypeID;

    bool isSRC = false;
    bool isDST = false;
    bool isOther = false;
}SKINFILELINEREAD;

typedef struct IFUNIT {
    int data[10] = {};
    int depth = 0; //
    int order = 0;
    int parentID = 0; //
    //int declare = 0;

    bool hide = 0;

    int grCount=0;
}IFUNIT;

typedef struct IMG {
    CSTR name;
    int gr, x, y, w, h;
}IMG;

typedef struct SRCGR {
    CSTR path{};
    CSTR filename{};

    bool loaded = false;
    //SDL_Texture* texture = NULL;
    //D3Image texture;
    PDIRECT3DTEXTURE9 texture;
    int sizeX, sizeY;

    CSTR name{};

    int grID;
    int isIf;

    int LR2ID;
    int SEID;

    bool wildcard = false;
    bool fromWildcard = false;
    ARR arr_wildcard; //CSTR
}SRCGR;

typedef struct SRC {
    int gr;
    CSTR name{"noname"};
    int x, y;
    int sizeX, sizeY;
    int div_x, div_y, cycle;
    int timer;

    int num, align, keta; //for numSRC
    int muki, range, type, disable;//for sliderSRC
    int click, panel, plusonly; //for buttonSRC

    int declare;
    int ifGroup;
    int srcID;

    int objType;//
    int objID;
}SRC;

typedef struct DST {
    int leadDST = -1;
    CSTR name{"noname"};
    int src;
    int animation = 0;

    ARR arr_animation;//DST_ANIMATION

    int loop, timer;
    int op1, op2, op3;
    int op4;//for turntable

    int declare;
    int ifGroup;
    int dstID;
}DST;

typedef struct DST_ANIMATION{
    int time;
    float x, y, w, h;
    int acc;
    int a, r, g, b;
    int blend; //
    int filter; //Anti-aliasing
    float angle;
    int center; //angle axis, numpad
}DST_ANIMATION;

typedef struct SKINUNIT {
    int ID;
    int type; //0:text 1:buttoon 2:slider 3:onmouse 4:BGA 5:bargraph 6:number 7:mask //10:img
    CSTR name{ "noname" };

    int src;
    int dst;

    ARR body; //CSTR
    ARR bodyCSV; //CSV
    SRC srcc;
    DST dstt;
    int type2; //-1:non-group 0:group

    int ifGroup;

    int igType;
    int igID;
}SKINUNIT, SEOBJ;

typedef enum HISTORYOP {
    overwriteLine,
    insertLine,
    removeLine,
    moveLine,
    group,
    ungroup,
    openFile = 255,
}HISTORYOP;

typedef struct HISTORY {
    HISTORYOP op;

    int target;
    SKINFILELINEREAD older;
    SKINFILELINEREAD newer;
}HISTORY;

typedef struct WORKSPACE {

    char initFlag = 0;
    //data
    game g;

    SkinHeader meta;

    bool loaded = false;
    char mainpath[MAX_PATH];

    byte* filedata = NULL;
    unsigned int filedatasize = 0;

    bool isTextmode = false;

    int skinSizeX = 640 , skinSizeY = 480;

    ARR arr_subpath; //CSTR
    ARR skinfileLines; //SKINFILELINEREAD
    ARR arr_ifunit; //IFUNIT

    ARR arr_CustomFile; //CSTR
    ARR arr_SRCGR; //SRCGR
    ARR arr_SRC; //SRC
    ARR arr_DST; //DST

    ARR arr_IMG; //IMG
    ARR arr_seobj; //SKINUNIT, SEOBJ

    ARR arr_history; //HISTORY
    int previewScreen; //DxLib handle

    //mainwindow
    int num;
    char title[260];
    int proc();
    int init();
    bool needInit = true;
    bool alive;
    int draw();

    //subwindows
    // 
    //
    int SeInit();
    int SeLoadInit();
    //HOW TO ADD FEATURE - STEP 1 : declare flag and function
    int wildcardTOAll(char* path);

    int ScanSkins();
    bool wSkinList;
    int drawSkinList();

    int LoadSkin(char* path);
    int LoadSkinScript(char* path);
    int ParseSkin();
    int currentLeadDST = -1;


    int LoadSceneSE();
    int ReadSkinSE();

    int ParseSkin2();

    int SaveSkinScript(char* path, bool split, bool nocomment);
    int SaveSkinScript2(char* path, bool split, bool nocomment);

    bool wSaveMenu;
    int drawSaveMenu();
    bool wSaveMenu2;
    int drawSaveMenu2();

    //TextEdit
    bool wTextEdit;
    int drawTextEdit();
    int textCursor = 0;
    bool hideComment = false;
    bool hideBlank = false;



    bool wPreview;
    int drawPreview();
    PDIRECT3DTEXTURE9 texture_preview;
    int timerSelected;
    ImVec2 clickPos;
    bool drawRightClick;
    float zoom = 1.0f;

    bool wCustomize;
    int drawCustomize();

    //ImgManager
    bool wImgManager;
    int drawImgManager();
    int grID_selected = 0;
    int gr_selected = 0;
    int src_selected = 0;
    int loadSRC();
    ImVec4 bgColor;
    int oldIf = -1;

    int printSrcImg(SRC src, bool button = 0);
    int printSrcImgButton(SRC src, int num, int w, int h);
    int printSrcImgEx(SRC src, int w, int h);

    bool wFileManager;
    int drawFileManager();

    bool wTreeView;
    int drawTreeView();
    int split, nocomment, exist, success;
    bool wSaveMenuResult;
    char newPath[260] = "";

    //newfile
    bool wNewskin;
    int drawNewskin();
    
    //simpleview (mockup)
    bool wSimplePreview;
    int drawSimplePreview();

    int drawSrc(int iSRCGR, int iSRCID);
    //int drawSrc(int iSRCGR, int iSRCID, int posX, int posY);
    int drawSrc(int iSRCGR, int iSRCID, int posX, int posY, int w = -1, int h = -1, bool stretch = 0);
    //dstview
    bool wDstView;
    int drawDstView();
    int selected_dst;
    float DstViewTime;
    bool isDstViewTimeStop;

    int MakeObjects();
    bool wObjectManager;
    int drawObjectManager();
    int selected_obj;


    bool wObjectManagerTest;
    int drawObjectManagerTest();
    struct preview_selected_obj {
        float x;
        float y;
        float w;
        float h;
    }preview_selected_obj;
    int selectedObjectTest;

    bool wHistory;
    int drawHistory();

    bool wProperty;
    int drawProperty();

    bool wOpList;
    int drawOpList();
    bool op[1000];
    
    int NewIMG(int gr, int x, int y, int w, int h);
    int DeleteIMG(int pos);
    int ModifyIMG(int pos, int gr, int x, int y, int w, int h);
    int FindIMG(int gr, int x, int y, int w, int h);

    int InsertLine(int pos);
    int DeleteLine(int pos);
    int EditLine(int pos, CSTR oldlinebody, CSTR newlinebody);
    int EditValue(int pos, int column, const char* newVal);
    int EditValue(int pos, int column, int newVal);
    int CsvToLine(int pos);


    int LoadSceneSE(skstruct* sk, CSTR skinfile, int p5, char font);

}WORKSPACE;

//every windows have procs, flags
//they have 


extern ARR workspaceList;

int makeTransBackground();
int AutoSRCObjectPos(SRCGR* gr, int* x, int* y, int* w, int* h);
int CsvToCSTR(CSVbuf& csv, CSTR& line);