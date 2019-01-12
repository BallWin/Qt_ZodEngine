#ifndef ZGFILE_H
#define ZGFILE_H

#include "qzod_dnobjects_global.h"


// -- default path to file sprites reference ------
#ifndef ZGFILE_NAME
    #define ZGFILE_NAME "assets/sprites.zgfx"
#endif


// --------------------------------------------
struct QZOD_DNOBJECTSSHARED_EXPORT ZGFileEntry
{
	char filename[512];
	int w, h;
	int f_offset;
};

class QZOD_DNOBJECTSSHARED_EXPORT ZGFile
{
public:
	ZGFile();

	static void Init();
	static void AddFile(string filename, SDL_Surface *img);
	static SDL_Surface *LoadFile(string filename);
	static int FindEntry(string filename);

private:
	static void LoadEntryList();

	static vector<ZGFileEntry> entry_list;
};

#endif
