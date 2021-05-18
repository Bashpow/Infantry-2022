#include "shellio.h"

unsigned char Str_Compare(char * str1, char * str2, unsigned short int len)
{
	for(unsigned short int i = 0; i < len; i++)
	{
		if(str1[i] != str2[i])
		{
			return 0;
		}
	}
	return 1;
}
