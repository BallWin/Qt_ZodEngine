#ifndef ZFONT_ENGINE_H
#define ZFONT_ENGINE_H

#include "qzod_dnseparate_global.h"
#include "zfont.h"

class QZOD_DNSEPARATESHARED_EXPORT ZFontEngine
{
	public:
		ZFontEngine();

		static void Init();
		static ZFont &GetFont(int font_type);
	private:
		static ZFont zfont[MAX_FONT_TYPES];
};

#endif
