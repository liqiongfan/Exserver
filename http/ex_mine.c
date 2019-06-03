/**
 * Copyright @2019 Exserver All Rights Reserved.
 */

#include <ex_mine.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <ex_string.h>

char *ex_get_mine_type(char *f)
{
	FILE *fp;
	char  code[5];

	ex_memzero(code, sizeof(code));

	if ( ex_strstr(f, EX_STRL(".css")) != -1 )
	{
		return MIMES[1];
	}
	else if ( ex_strstr(f, EX_STRL(".html")) != -1 )
	{
		return MIMES[0];
	}
	else if ( ex_strstr(f, EX_STRL(".xml")) != -1 )
	{
		return MIMES[2];
	}
	else if ( ex_strstr(f, EX_STRL(".txt")) != -1 )
	{
		return MIMES[7];
	}
	else if ( ex_strstr(f, EX_STRL(".js")) != -1 )
	{
		return MIMES[5];
	}
	else if ( ex_strstr(f, EX_STRL(".json")) != -1 )
	{
		return MIMES[6];
	}
	else
	{
		fp = fopen(f, "r");
		fread(code, sizeof(char), sizeof(code), fp);

		if ( code[0] == -1 && code[1] == -40 && code[2] == -1 )
		{
			return MIMES[4];
		}
		else if ( code[0] == 71 && code[1] == 73 && code[2] == 70 )
		{
			return MIMES[3];
		}
		else if ( code[0] == -119 && code[1] == 80 && code[2] == 78 )
		{
			return MIMES[8];
		}
		else if ( code[0] == 73 && code[1] == 68 && code[2] == 51 )
		{
			return MIMES[17];
		}
		else if ( code[0] == 80 && code[1] == 75 && code[2] == 3 && code[3] == 4 && code[4] == 10 )
		{
			return MIMES[19];
		}
		else if ( code[0] == 0 && code[4] == 102 )
		{
			return MIMES[14];
		}
#if EX_DEBUG
		printf("%d %d %d %d %d \n%s", code[0], code[1], code[2], code[3], code[4], code);
#endif
		fclose(fp);
	}

	/* Unkown type return application/octet-stream */
	return MIMES[18];
}