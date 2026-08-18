#include "../LR2/En_fileutil.h"
#include "../LR2/structure.h"
#include "winWorkspace.h"
#include "resource.h"
#include <Windows.h>
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

///////////////////////////////////////////////////////////////////////
