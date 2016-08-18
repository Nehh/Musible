#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_READ_BYTES 1024

#define LANG_COMMON_ITEMS 8u
#define LANG_CONFIG_ITEMS 8u

typedef char **restrict strtable;

strtable LANG_COMMON;
strtable LANG_CONFIG;

int CheckLanguageFile();
int UnloadLanguage(strtable *restrict);

#define LANG_DEFAULT	"zh_CN"

#define	LANG_ROOT	".//Lang//"
#define	LANG_FILE_COMMON		"//common.txt"
#define	LANG_FILE_CONFIG		"//config.txt"

enum {
    LANG_CFG_VERSION,
    LANG_CFG_LANG,
    LANG_CFG_TITLE,
    LANG_CFG_FULLSCREEN,
    LANG_CFG_GAME_WIDTH,
    LANG_CFG_GAME_HEIGHT,
    LANG_CFG_LOCAL_USERNAME,
    LANG_CFG_LOCAL_PASSWORD,
} LANG_CONFIG_DEFS;
