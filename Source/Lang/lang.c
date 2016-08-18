#include "lang.h"

#define	_TRUNCATE ((size_t)-1)

int LoadLanguageSet(char * LangRegion);
int LoadLanguageFile(strtable *restrict, unsigned int, char *, char *);

int CheckLanguageFile()
{
	FILE * LangRegionFile = fopen(".//language.txt", "r, ccs=UTF-8");
	
	char LangRegion[6];
	
	LangRegion[0] = 0;
	
	if(LangRegionFile != NULL)
	{
		fgets(LangRegion, 48, LangRegionFile);
		fclose(LangRegionFile);
	} else
	{
		strncat(LangRegion, LANG_DEFAULT, sizeof(LANG_DEFAULT));
	}
	
	LoadLanguageSet(LangRegion);
	
	return 0;
}

int ReadLanguageFile(FILE * LangFile, strtable *restrict LangPtrArr, unsigned int Items)
{
	free(*LangPtrArr);
	*LangPtrArr = malloc(Items * sizeof(strtable));
	char *restrict Temp;
	unsigned int i = 0;
	char *restrict Mem = NULL;
	
	while (i != Items)
	{
		Temp = (char *)malloc(MAX_READ_BYTES * sizeof(char));	// Malloc a temp memory to storage string for further handle
		
		fgets(Temp, MAX_READ_BYTES, LangFile);	// ReadLine from file
		
		if(feof(LangFile)) break;
		
		size_t Size = strlen(Temp);	// Get size of the string
		
		if(Size > 2 && Temp[Size-2] == '\r')	// check whether the string is Windows spec so we can remove unusable char to prevent bugs
			Size--;	// We have less a char to copy ahh,be happy with it. 
		
		Mem = (char *)malloc(Size * sizeof(char));	//this is the memory blocks where we are going to storage the handled string actually
		*Mem = 0;	// Mark this memory as a cleared string
		
			// We have extra size when removing the newline char '\n' or string end mark '\0' to storage new '\0'
		
		(*LangPtrArr)[i] = strncat(Mem, Temp, Size-1);	// copy the pointer to our string table array
		
		free(Temp);	// dont trust GC
		Mem = NULL; // set Mem back to NULL
		
		++i;
	}
	
	return 0;
}

int LoadLanguageSet(char * LangRegion)
{
	char * LangFileRoot = (char *)malloc(MAX_READ_BYTES * sizeof(char));
	strcpy(LangFileRoot, LANG_ROOT);
	strncat(LangFileRoot, LangRegion, _TRUNCATE);
	
	//LoadLanguageFile(&LANG_COMMON, LANG_COMMON_ITEMS, LANG_FILE_COMMON, LangFileRoot);
	LoadLanguageFile(&LANG_CONFIG, LANG_CONFIG_ITEMS, LANG_FILE_CONFIG, LangFileRoot);
	
	free(LangFileRoot);
	
	return 0;
}

int LoadLanguageFile(strtable *restrict Lang_Storage, unsigned int Lang_Items, char * LangFileTarget, char * LangFileRoot)
{
	char * LangFileName = (char *)malloc(MAX_READ_BYTES * sizeof(char));
	strncat(LangFileName, LangFileRoot, _TRUNCATE);
	strncat(LangFileName, LangFileTarget, _TRUNCATE);
	
	FILE * LangFile = fopen(LangFileName, "r, ccs=UTF-8");
	
	ReadLanguageFile(LangFile, Lang_Storage, Lang_Items);
	
	free(LangFileName);
	
	fclose(LangFile);
	
	return 0;
}

int UnloadLanguage(strtable *restrict Lang_Storage)
{
	unsigned int i = sizeof(*Lang_Storage);
	while(i--!=0)
	{
		free((*Lang_Storage)[i]);
	}
	free(*Lang_Storage);
	return 0;
}
