#include "../LR2/En_fileutil.h"
#include "../LR2/structure.h"
#include "winWorkspace.h"
#include "seHelper.h"
#include "resource.h"
#include "op.h"
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>


ARR arr_CommandHelp; //CSVbuf line

bool LoadEmbeddedTextResource(int resourceId, std::string& text) {
	text.clear();
	HRSRC resource = FindResourceA(NULL, MAKEINTRESOURCEA(resourceId), RT_RCDATA);
	if (!resource) return false;
	HGLOBAL loaded = LoadResource(NULL, resource);
	if (!loaded) return false;
	const void* data = LockResource(loaded);
	const DWORD size = SizeofResource(NULL, resource);
	if (!data || size == 0) return false;
	text.assign((const char*)data, (size_t)size);
	return true;
}

int LoadCommandHelp(const char* file) { //temp function, later hardcoded... with script, maybe?
	std::string contents;
	if (file && *file) {
		FILE* hFile = fopen(file, "rb");
		if (hFile) {
			fseek(hFile, 0, SEEK_END);
			const long size = ftell(hFile);
			if (size >= 0 && fseek(hFile, 0, SEEK_SET) == 0) {
				contents.resize((size_t)size);
				if (size > 0) fread(&contents[0], 1, (size_t)size, hFile);
			}
			fclose(hFile);
		}
	}
	if (contents.empty() && !LoadEmbeddedTextResource(IDR_SKIN_HELPER_TXT, contents)) return -1;

	arr_CommandHelp.Free();
	arr_CommandHelp.Alloc(sizeof(CSVbuf), 100);

	std::istringstream input(contents);
	std::string line;
	while (std::getline(input, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		CSTR fBuf(line.c_str());
		if (*fBuf.atPos(0) == '#') {
			fBuf.trimWhiteSpace();
			
			CSVbuf* nCsv = (CSVbuf*)(arr_CommandHelp.Get_new()); //.push_back(&csv);
			SplitCSV(fBuf, nCsv, ",");
		}
	}
	
	return 0;
}

CSTR GetCommandHelp(const char* command, int column) {

	for (int i = 0; i < arr_CommandHelp.count; i++) {
		CSVbuf& cmd = ((CSVbuf*)arr_CommandHelp.data)[i];
		
		if ( strncmp(command, cmd.str[0], strlen(cmd.str[0]) ) ) continue;
		else return cmd.str[column];
	}
	return "WIP";
}

SECommandValueKind GetCommandValueKind(const char* command, const char* columnHelp) {
	if (!command || !columnHelp) return SE_VALUE_NONE;
	if (_strnicmp(columnHelp, "$op", 3) == 0) return SE_VALUE_OPTION;
	if (_strnicmp(columnHelp, "$st", 3) == 0) return SE_VALUE_TEXT;
	if (_strnicmp(columnHelp, "$num", 4) == 0) return SE_VALUE_NUMBER;
	if (_strnicmp(columnHelp, "$timer", 6) == 0) return SE_VALUE_TIMER;
	if (_strnicmp(columnHelp, "$type", 5) != 0) return SE_VALUE_NONE;

	// $type is command-specific in LR2. It is not always a skin scene type.
	if (_stricmp(command, "#INFORMATION") == 0) return SE_VALUE_SKIN_TYPE;
	if (_stricmp(command, "#SRC_BUTTON") == 0) return SE_VALUE_BUTTON;
	if (_stricmp(command, "#SRC_SLIDER") == 0) return SE_VALUE_SLIDER;
	if (_stricmp(command, "#SRC_BARGRAPH") == 0) return SE_VALUE_BARGRAPH;
	return SE_VALUE_NONE;
}

const char* GetCommandValueName(SECommandValueKind kind, int value) {
	extern const char* SKINTYPESTR[];
	switch (kind) {
	case SE_VALUE_SKIN_TYPE: return value >= 0 && value < 21 ? SKINTYPESTR[value] : "";
	case SE_VALUE_OPTION: return dstName((unsigned)std::abs(value));
	case SE_VALUE_TEXT: return value >= 0 ? textName((unsigned)value) : "";
	case SE_VALUE_NUMBER: return value >= 0 ? numberName((unsigned)value) : "";
	case SE_VALUE_TIMER: return value >= 0 ? timerName((unsigned)value) : "";
	case SE_VALUE_BUTTON: return value >= 0 ? buttonName((unsigned)value) : "";
	case SE_VALUE_SLIDER: return value >= 0 ? sliderName((unsigned)value) : "";
	case SE_VALUE_BARGRAPH: return value >= 0 ? bargraphName((unsigned)value) : "";
	default: return "";
	}
}

int GetCommandValueItemCount(SECommandValueKind kind) {
	switch (kind) {
	case SE_VALUE_SKIN_TYPE: return 21;
	case SE_VALUE_OPTION: return 1999; // 0..999 followed by NOT 1..999
	case SE_VALUE_TEXT: return 303; // 0..302
	case SE_VALUE_NUMBER: return 423; // 0..422
	case SE_VALUE_TIMER: return 200;
	case SE_VALUE_BUTTON: return 402; // 0..401
	case SE_VALUE_SLIDER: return 29; // 0..28
	case SE_VALUE_BARGRAPH: return 60; // 0..59
	default: return 0;
	}
}

int GetCommandValueAt(SECommandValueKind kind, int item) {
	if (kind == SE_VALUE_OPTION && item >= 1000) return -(item - 999);
	return item;
}

///////////////////////////////////////////////////////////////////////
