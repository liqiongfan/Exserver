/**
 * Copyright @2019 Exserver All rights reserved
 */

#include <ex_log.h>
#include <ex_string.h>

void ex_logger(int level, char *info, ...)
{
	int _i, sp;
	long    lv;
	FILE   *fp;
	char s, *s1, tf[20];
	va_list list;

	switch (level)
	{
		case LOG_ERROR:
			fp = fopen(EX_LOG_ERROR_FILE_NAME, "a+");
			break;
		case LOG_INFO:
			fp = fopen(EX_LOG_INFO_FILE_NAME, "a+");
			break;
		default:
			return ;
	}

	va_start(list, info);

	for ( _i = 0, sp = 0; ; _i++ )
	{
		s = *(info + _i);
		if ( s == '\0' ) break;

		switch (s)
		{
			case '%': {
				if ( !sp )
				{
					sp = 1;
					break;
				}
			}
			default:
				if ( sp )
				{
					switch (s)
					{
						case 'd':
							lv = va_arg(list, long);
							ex_memzero(tf, sizeof(tf));
							sprintf(tf, "%ld", lv);
							fwrite(tf, sizeof(char), strlen(tf), fp);
							break;
						case 's':
							s1 = va_arg(list, char *);
							fwrite(s1, sizeof(char), strlen(s1), fp);
							break;
						case '%':
							fwrite(&s, sizeof(char), 1, fp);
							break;
						default:
							printf("Unsupported synbol: %c\n", s);
							break;
					}
					sp = 0;
				}
				else
				{
					fwrite(&s, sizeof(char), 1, fp);
				}
				break;
		}
	}

	va_end(list);

	s = '\n';
	fwrite(&s, sizeof(char), 1, fp);
	fclose(fp);
}




