#include "mem.h"

int LoadFileIntoMemory(const char *name, void **buff, int *length)
{
    FILE *file = fopen(name, "rb");
    ert(file);
    
    int err = fseek(file, 0, SEEK_END);
    ert(err == 0);
    
    int len = ftell(file);
    ert(len >= 0);
    
    rewind(file);
    
    void *mem = malloc(len);
    ert(len);
    
    int read = fread(mem, 1, len, file);
    ert(read == len);
    
    err = fclose(file);
    ert(err == 0);

    *buff = mem;
    *length = len;
	
	return 0;
}
