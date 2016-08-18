#include	<stdio.h>
#include	<stdlib.h>
#include	<stdbool.h>
#include	<string.h>
#include	"../Lang/lang.h"
#include	"../Utils/str.h"

int	LoadConfigFile();
int	SaveConfigFile();
int	CreateConfigFile();
int UnloadConfig(void ** Config);

int ChangeConfigBool(int, bool);
int ChangeConfigLong(int, unsigned long);
int ChangeConfigString(int, char *);

#define	CONFIG_ITEMS 9

void * CONFIG[CONFIG_ITEMS];

enum {
	CFG_VERSION,
	CFG_LANG,
	CFG_TITLE,
	CFG_FULLSCREEN,
	CFG_GAME_WIDTH,
	CFG_GAME_HEIGHT,
	CFG_LOCAL_USERNAME,
	CFG_LOCAL_PASSWORD,
	CFG_FONT,
};
