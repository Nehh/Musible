// FUCK YOU FILE IO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO SHITTTTTTTTTTTT!!!!!!!!

#define	_TRUNCATE ((size_t)-1)

#include	"config.h"

int ReadConfigRaw(char ***);

int CheckConfigFile(){
	return 0;
}

int LoadConfigFile()
{
	char ** CFG_RAW;
	
	CFG_RAW = (char **)malloc(sizeof(char *) * CONFIG_ITEMS);
	for(int i = sizeof(CFG_RAW);i!=0;i--){
		CFG_RAW[i-1] = NULL;
	}
	
	FILE * ConfigFile = fopen("config.txt", "r, ccs=UTF-8");
	if(ConfigFile == NULL)
	{
		CreateConfigFile();
		LoadConfigFile();
		return 0;
	}
	char * Temp = NULL;
	char * HandledTemp = NULL;
	Temp = (char *)malloc(MAX_READ_BYTES * sizeof(char));
	while(!feof(ConfigFile))
	{
		*Temp = 0;
		fgets(Temp, MAX_READ_BYTES, ConfigFile);
		if(feof(ConfigFile)) break;
		HandledTemp = StrValid(Temp);
		if(strcspn(HandledTemp,"#\0")==0)
			continue;
		for(unsigned int i = 0; i != LANG_CONFIG_ITEMS; ++i)
		{
			if(strcmp(HandledTemp,LANG_CONFIG[i])!=0)
				continue;
			free(CFG_RAW[i]);
			fgets(Temp, MAX_READ_BYTES, ConfigFile);
			HandledTemp = StrValid(Temp);
			CFG_RAW[i] = malloc(strlen(HandledTemp) * sizeof(char));
			CFG_RAW[i][0] = 0;
			strcat(CFG_RAW[i], HandledTemp);
		}
	}
	fclose(ConfigFile);
	free(Temp);
	ReadConfigRaw(&CFG_RAW);
	unsigned int i = sizeof(CFG_RAW);
	while(--i!=0)
	{
		free((CFG_RAW)[i]);
	}
	free(CFG_RAW);
	return 0;
}

/**	
 *	Target is where we storage the string which we've handled before, now it's serialized and readable.
 *	Usually Target is called Config Raw Data
 * 
 *	WARNING:
 *		This might complete, just !!!MIGHT!!!
 */
int ReadConfigRaw(char *** Target)
{
	unsigned int i = sizeof(*Target);
	void *restrict Temp = NULL;
	while(--i!=-1)
	{
		if(strcmp((*Target)[i],u8"false")==0){
			Temp = malloc(sizeof(bool));
			*(bool *)Temp = (bool)false;
			CONFIG[i] = Temp;
			continue;
		}else if(strcmp((*Target)[i],u8"true")==0){
			Temp = malloc(sizeof(bool));
			*(bool *)Temp = (bool)true;
			CONFIG[i] = Temp;
			continue;
		}

		Temp = malloc(sizeof(unsigned long));
		if(ParseULong((*Target)[i],(unsigned long *)Temp)==true)
		{
			CONFIG[i] = (unsigned long *)Temp;
			continue;
		}
		free(Temp);
		
		CONFIG[i] = strcpy((char *)malloc(strlen((*Target)[i]) * sizeof(char)),(*Target)[i]);
	}
	return 0;
}

/**
 * 创建设置文件
 * 这个函数是用来在找不到设置文件的时候创建一个模板设置文件方便玩家配置以及后续载入的
 * 然而创建配置文件的这个过程卵用没有,只有保存有点用.于是乎..偷懒走起.
 */

int CreateConfigFile()
{
	FILE * ConfigFile = fopen("config.txt", "w, ccs=UTF-8");
	char * Test = u8"Who Cares?\0";
	int size = fwrite(Test,sizeof(*Test),strlen(Test),ConfigFile);
	int err = fclose(ConfigFile);
	return 0;
}

int ChangeConfigBool(int index, bool val)
{
	free(CONFIG[index]);
	CONFIG[index] = (bool *)malloc(sizeof(bool));
	*(bool *)CONFIG[index] = val;
	return 0;
}

int ChangeConfigLong(int index, unsigned long val)
{
	free(CONFIG[index]);
	CONFIG[index] = (unsigned long *)malloc(sizeof(unsigned long));
	*(unsigned long *)CONFIG[index] = val;
	return 0;
}

int ChangeConfigString(int index, char * val)
{
	free(CONFIG[index]);
	void * Temp;
	Temp = (char *)malloc(sizeof(char)*strlen(val));
	*(char *)Temp = 0;
	strcat(Temp,val);
	CONFIG[index] = Temp;
	return 0;
}

/**
 * 创建用来保存默认设置的指针数组
 * 首先创建一个泛型指针数组
 * 然后为默认参数开辟内存空间
 * 返回这个默认参数的指针数组
 * WARNING:用完一定要free
 * WARNING WARNING WARNING -- 现在不应该用这个功能,会让开发变得相当复杂,我们只需要依托一个完整的配置文件就可以避免这个问题.
 */
void ** DefaultConfig()
{
	void ** CFG_DEFAULT = malloc(sizeof(void *) * (CONFIG_ITEMS));
	CFG_DEFAULT[CFG_VERSION]=malloc(sizeof(int));
	CFG_DEFAULT[CFG_LANG]=malloc(sizeof(char *));
	CFG_DEFAULT[CFG_TITLE]=malloc(sizeof(char *));
	CFG_DEFAULT[CFG_FULLSCREEN]=malloc(sizeof(bool));
	CFG_DEFAULT[CFG_GAME_WIDTH]=malloc(sizeof(int));
	CFG_DEFAULT[CFG_GAME_HEIGHT]=malloc(sizeof(int));
	CFG_DEFAULT[CFG_LOCAL_USERNAME]=malloc(sizeof(char *));
	CFG_DEFAULT[CFG_LOCAL_PASSWORD]=malloc(sizeof(char *));
	return CFG_DEFAULT;
}

int UnloadConfig(void ** Config)
{
	unsigned int i = sizeof(Config);
	while(--i!=0)
	{
		free((Config)[i]);
	}
	free(*Config);
	return 0;
}

/**
 * 梳理一下流程
 * SaveConfigFile是用来保存设置文件的,这意味着肯定有一个源文件,否则就应该调用CreateConfigFile而不是SaveConfigFile
 * 因为不可以删除行而只能新建文档写入,所以创建一个tmp文件,等新文件的缓存写入完成的时候删掉旧文档.
 * 读取数据的时候要小心处理无效字符,因为保存的时候即使是无效字符也要一并写入.只改动有效字符
 * 
 * 流程:
 * 打开原本的设置文件
 * 写入模式打开新的设置文件
 * 
 * 如果根本不存在老设置文件,跳转到创建设置文件的流程
 * 
 * 设置一个缓存用的字符串空间,这个字符串空间用来保存从文件直接读取的字符串流
 * 设置一个指向有效字符用的字符指针,这个指针用于提供缓存用字符串空间真正需要被改写的部分
 * 
 * 读取旧文件直到文件结束,以下是读取时所要进行的流程
 * 
 * 将缓存空间的第一个字符设置成字符串结束符避免重新分配空间这一不必要的流程
 * 
 * 读取文件行并写入缓存空间
 * 
 * 获取这个行的可用部分并传给字符指针
 * 
 * 如果这个字符指针指向的第一个字符是注释符,将缓存的那行字符串直接写入到将要写入的文件流,继续读取下一行
 * 
 * 如果是确切可以用的数据(没标明是注释)
 * 
 * 在设置选项的语言翻译里面寻找对应设置数据所在数组中的位置
 * 
 * 如果并没找到符合的数据,视为无效,切换到下一行继续读取
 * 
 * 如果符合语言翻译数据,继续读取下一行,寻找可以写入数据的位置
 * 
 * 将可以写入数据的位置之前的字符串一并复制,将对应的设置数据写入到可以写入的位置并且添加上换行符
 * 
 * 重复此流程
 * 
 * TODO:
 * 测试输出的tmp文件,删除旧文件并重命名这个新文件
 * 
 * 这个函数看上去写好了,但是需要测试.
 */

int SaveConfigFile()
{
	FILE * ConfigFile = fopen("config.txt", "r, ccs=UTF-8");
	FILE * ConfigFileTemp = fopen("config.tmp", "w, ccs=UTF-8");
	if(ConfigFile == NULL)
	{
		CreateConfigFile();
		return 0;
	}
	char * Temp = NULL;
	char * HandledTemp = NULL;
	Temp = (char *)malloc(MAX_READ_BYTES * sizeof(char));
	while(!feof(ConfigFile))
	{
		*Temp = 0;
		fgets(Temp, MAX_READ_BYTES, ConfigFile);
		if(feof(ConfigFile)) break;
		HandledTemp = StrValid(Temp);
		if(strcspn(HandledTemp,"#\0")==0)
		{
			fputs(Temp, ConfigFileTemp);
			fputc('\n',ConfigFileTemp);
			continue;
		}
		for(unsigned int i = 0; i != LANG_CONFIG_ITEMS - 1; ++i)
		{
			if(strcmp(HandledTemp,LANG_CONFIG[i])!=0)
				continue;
			//free(CFG_RAW[i]); //没有需要free的项目
			fputs(Temp,ConfigFileTemp);
			fputc('\n',ConfigFileTemp);
			fgets(Temp, MAX_READ_BYTES, ConfigFile);
			HandledTemp = StrValid(Temp);
			fwrite(Temp,sizeof(char),strspn(Temp, "\t "),ConfigFileTemp);
			//strcpy(malloc(strlen(HandledTemp) * sizeof(char)), HandledTemp);
			if((strcmp(HandledTemp,u8"false")==0)||(strcmp(HandledTemp,u8"true")==0)){
				fputs(*(bool *)CONFIG[i]==false?u8"false":u8"true",ConfigFileTemp);
				fputc('\n',ConfigFileTemp);
				continue;
			}
			
			if(ParseULong(HandledTemp,NULL)==true)
			{
				fprintf(ConfigFileTemp,"%lu\n",*(unsigned long *)CONFIG[i]);
				continue;
			}
			
			fputs((char *)CONFIG[i],ConfigFileTemp);
			fputc('\n',ConfigFileTemp);
		}
	}
	free(Temp);
	int err = 0;
	err += fclose(ConfigFile);
	err += fclose(ConfigFileTemp);
	remove("config.txt");
	rename("config.tmp","config.txt");
	return err;
}
//TODO:准备迎接SegmentFault的审判吧!
