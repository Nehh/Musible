#include "fonts.h"

static const char *FONT_DEFAULT_FAMILY_NAMES[] = {
   "monospace",
   "sans",
   "serif"
};

int GetAvalibleFontList()
{
	FcConfig* config = FcInitLoadConfigAndFonts();
	//make pattern from font name
	FcPattern* pat = FcNameParse((const FcChar8*)"Arial");
	FcConfigSubstitute(config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
	char* fontFile; //this is what we'd return if this was a function
	// find the font
	FcResult result;
	FcPattern* font = FcFontMatch(config, pat, &result);
	if (font)
	{
		FcChar8* file = NULL;
		if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
		{
			//we found the font, now print it.
			//This might be a fallback font
			fontFile = (char *)file;
			printf("%s\n",fontFile);
		}
	}
	FcPatternDestroy(pat);
	return 0;
}

int GetFontByName(char * fName) {
	FcPattern *pat, *match;
	FcChar8 *path;
	FcResult result;
	
	pat = FcPatternCreate();
	FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)"serif");

	FcDefaultSubstitute(pat); /* fill in other expected pattern fields */
	FcConfigSubstitute(NULL, pat, FcMatchPattern); /* apply any system/user config rules */
	match = FcFontMatch(0, pat, &result); /* find 'best' matching font */

	if (result != FcResultMatch || !match) {
		return 0;
	}

	FcPatternGetString(match, "file", 0, &path);

	printf("%s\n",(const char*)path);

	FcPatternDestroy(match);
	FcPatternDestroy(pat);

	return 0;
}

int GetDefaultFontName() {
	FcPattern *pat, *match;
	FcChar8 *path;
	//TxdFont *font = NULL;
	FcResult result;

	//TXDI_ASSERT(family >= 0 && family < TXD_COUNT_DEFAULT);
	//TXDI_ASSERT(height > 0.0f);
	
	pat = FcPatternCreate();
	FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)"serif");
	
	//if (style & TXD_STYLE_BOLD) { FcPatternAddInteger(pat, "weight", FC_WEIGHT_BOLD); }
	//if (style & TXD_STYLE_ITALIC) { FcPatternAddInteger(pat, "slant", FC_SLANT_ITALIC); }

	//FcPatternAddDouble(pat, FC_DPI, 72.0); /* 72 dpi = 1 pixel per 'point' */
	//FcPatternAddDouble(pat, FC_PIXEL_SIZE, 1.0);c
	//FcPatternAddDouble(pat, FC_SIZE, height);

	FcDefaultSubstitute(pat); /* fill in other expected pattern fields */
	FcConfigSubstitute(0, pat, FcMatchPattern); /* apply any system/user config rules */
	match = FcFontMatch(0, pat, &result); /* find 'best' matching font */

	if (result != FcResultMatch || !match) {
		/* FIXME: better error reporting/handling here...
		* want to minimise the situations where opendefaultfont gives you *nothing* */
		return 0;
	}

	FcPatternGetString(match, "file", 0, &path);

	printf("%s\n",(const char*)path);

	FcPatternDestroy(match);
	FcPatternDestroy(pat);

	return 0;
}
