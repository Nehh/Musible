// fxxk you string
#include "str.h"

bool ParseULong(const char *str, unsigned long *restrict val)
{
    char *temp;
    bool rc = true;
	if(val!=NULL){
		*val = strtoul(str, &temp, 0);
	}else{
		strtoul(str, &temp, 0);
	}
	
    if (temp == str || *temp != '\0')
        rc = false;

    return rc;
}

char * StrValid(char * str)
{
	*(strpbrk(str,"\r\n\0")) = 0;
	return &str[strspn(str, "\t ")]; 
}
