// entry of the whole Game

#include "Config/config.h"
#include "Graphics/graphics.h"
#include "Musible.h"

int main (void)
{
	CheckLanguageFile();
	
	LoadConfigFile();

	CreateWindow();
	
	CleanGraphics();
    /*
	
	for(unsigned int i = LANG_CONFIG_ITEMS; i!=0;)
	{
		printf("%s\n", LANG_CONFIG[--i]);
	}
	
	printf("%s\n", (char *)CONFIG[CFG_TITLE]);
	
	ChangeConfigString(CFG_TITLE, u8"音律");
	
	printf("%s\n", (char *)CONFIG[CFG_TITLE]);
	
	SaveConfigFile();*/
	CleanGraphics();
	
	UnloadLanguage(&LANG_CONFIG);
	
	UnloadConfig(CONFIG);

    return 0;
}

int create_window ()
{
    return 0;
}
