#include "../LR2/En_fileutil.h"
#include "../LR2/structure.h"
#include "winWorkspace.h"


ARR arr_CommandHelp; //CSVbuf line

int LoadCommandHelp(const char* file) { //temp function, later hardcoded... with script, maybe?
	CSVbuf csv;
	FILE* hFile;

	hFile = fopen(file, "r");
	if (!hFile) return 0;

	CSTR fBuf(260);
	char* pFbuf;

	arr_CommandHelp.Free();
	arr_CommandHelp.Alloc(sizeof(CSVbuf), 100);

	pFbuf = fBuf.outstr();
	while (pFbuf = fgets(pFbuf, 256, hFile)) {
		if (*fBuf.atPos(0) == '#') {
			fBuf.trimWhiteSpace();
			
			CSVbuf* nCsv = (CSVbuf*)(arr_CommandHelp.Get_new()); //.push_back(&csv);
			SplitCSV(fBuf, nCsv, ",");
		}
	}
	fclose(hFile);
	
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
