/**
 * Copyright @2019 Exserver All rights reserved
 */

#include <ex_types.h>
#include <assert.h>
#include <ex_string.h>

char *exsubstr(const char *source, long start, long length, int mode)
{
    long _i;
    
    if ( source == NULL ) return NULL;
    
    size_t left = 0, right = 0, source_len = strlen(source);
    if ( length < 0 )
    {
        length = (-1) * length; start = source_len - length - 1;
    }
    
    if ( length > source_len ) length = source_len;
    
    if ( TRIM_LEFT == mode || TRIM_BOTH == mode )
    {
        /* Trim left */
        for ( _i = 0; _i < source_len - start; ++_i )
        {
            if ( *(source + start + _i) == ' ') left++;
            else break;
        }
    }
    
    if ( TRIM_RIGHT == mode || TRIM_BOTH == mode )
    {
        /* Trim right */
        for ( _i = length - 1; _i > 0; --_i )
        {
            if ( *(source + start + _i) == ' ') right++;
            else break;
        }
    }
    length = length - left - right + 1;
    char *result = malloc(sizeof(char) * length);
    assert(result != NULL);
    ex_memzero(result, sizeof(char) * length);
    strncpy(result, source + start + left, length - 1);
    return result;
}

char *extrim(const char *source, int mode)
{
    unsigned long _i;
    
    if ( source == NULL ) return NULL;
    
    size_t left = 0, right = 0, result_length, source_len = strlen(source);
    
    if ( TRIM_LEFT == mode || TRIM_BOTH == mode )
    {
        /* Trim left */
        for ( _i = 0; _i < source_len; ++_i )
        {
            if ( *(source + _i) == ' ') left++;
            else break;
        }
    }
    
    if ( TRIM_RIGHT == mode || TRIM_BOTH == mode )
    {
        /* Trim right */
        for ( _i = source_len - 1; _i > 0; --_i )
        {
            if ( *(source + _i) == ' ') right++;
            else break;
        }
    }
    
    result_length = source_len - left - right + 1;
    char *result = malloc(sizeof(char) * result_length);
    assert(result != NULL);
    memset(result, 0, sizeof(char) * result_length);
    strncpy(result, source + left, result_length - 1);
    return result;
}

long ex_strstr(const char *source, const char *str, size_t n)
{
	char    d, s;
	size_t  i, j;

	for ( i = 0; ; i++ )
	{
		d = *(source + i);

		if ( d == '\0' )
		{
			break;
		}

		if ( d == *str )
		{
			j = 0;
			while ( j++ < n )
			{
				d = *(source + i + j);
				s = *(str + j);
				if ( d == '\0' && j != '\0' )
				{
					return -1;
				}
				if ( s != d ) break;
				if ( j == n - 1 ) return i;
			}
		}
	}

	return -1;
}

long ex_strncasestr(const char *source, const char *str, size_t n)
{
	char    d, s;
	size_t  i, j;

	for ( i = 0; ; i++ )
	{
		d = *(source + i);
		s = *str;

		d = ex_tolower(d);
		s = ex_tolower(s);

		if ( d == '\0' )
		{
			break;
		}

		if ( d == s )
		{
			j = 0;
			while ( j++ < n )
			{
				d = *(source + i + j);
				s = *(str + j);

				d = ex_tolower(d);
				s = ex_tolower(s);

				if ( d == '\0' && j != '\0' )
				{
					return -1;
				}
				if ( s != d ) break;
				if ( j == n - 1 ) return i;
			}
		}
	}

	return -1;
}

long ex_strrncasestr(const char *str, const char *res, int rl)
{
    char s, r;
    long i, l, j;
    
    if ( rl == 0 ) return -1;
    
    l  = strlen(str);
    rl = rl - 1; /* at least 1 char(rl = 0) */
    
    for ( i = l; i > 0 ; i-- )
    {
        s = ex_tolower(*(str + i));
        r = ex_tolower(*(res + rl));
        
        if ( s == r )
        {
            for ( j = rl; j >= 0; j--, i--)
            {
                s = ex_tolower(*(str + i));
                r = ex_tolower(*(res + j));
                
                if ( s != r )
                {
                    break;
                }
                
                if ( j == 0 ) return i;
            }
        }
    }
    
    /* not found */
    return -1;
}

long ex_strirncasestr(const char *str, long sp, const char *res, int rl)
{
    char s, r;
    long i, l, j;
    
    if ( rl == 0 ) return -1;
    
    l  = sp;
    rl = rl - 1; /* at least 1 char(rl = 0) */
    
    for ( i = l; i > 0 ; i-- )
    {
        s = ex_tolower(*(str + i));
        r = ex_tolower(*(res + rl));
        
        if ( s == r )
        {
            for ( j = rl; j >= 0; j--, i--)
            {
                s = ex_tolower(*(str + i));
                r = ex_tolower(*(res + j));
                
                if ( s != r )
                {
                    break;
                }
                
                if ( j == 0 ) return i;
            }
        }
    }
    
    /* not found */
    return -1;
}

long ex_get_cl(const char *s, long i)
{
    char c, tf[20];
    long j, k;
    
    k = 0;
    
    for ( j = i; ;j++ )
    {
        c = *(s + j);
        if ( c == '\0' )
        {
            return -1;
        }
        
        if ( ex_isdigit(c) && !k )
        {
            k = j;
        }
        else if ( ex_isdigit(c) && k )
        {
            continue;
        }
        
        if ( (c == '\r' || c == '\n') && k )
        {
            ex_memzero(tf, sizeof(tf));
            memcpy(tf, s + k, j - k);
            return strtol(tf, NULL, 10);
        }
        else if ( k && !ex_isdigit(c) )
        {
            return -1;
        }
    }
    
}

void _ex_strncat_(char **dest, const char *source, size_t *origin_size, size_t *used_num)
{
    if ( !dest ) return ;
    
    size_t  remain_size = *origin_size - *used_num,
            source_len = strlen(source);
    
    if ( remain_size > source_len )
    {
        strncat(*dest, source, source_len);
        *used_num += source_len;
    }
    else if ( remain_size < source_len && source_len < BUFFER_ALLOCATE_SIZE )
    {
        /* Remained space were not enough */
        char *ptr = realloc(*dest, sizeof(char) * (*origin_size + BUFFER_ALLOCATE_SIZE) );
        if ( ptr == NULL ) return ;
        
        *dest = ptr;
        
        *origin_size = *origin_size + BUFFER_ALLOCATE_SIZE;
        strncat( *dest, source, source_len);
        *used_num = *used_num + source_len;
    }
    else
    {
        /* Remained space were not enough */
        char *ptr = realloc(*dest, sizeof(char) * (*origin_size + source_len - remain_size + 1) );
        if ( ptr == NULL ) return ;
        
        *dest = ptr;
        
        *origin_size = *origin_size + source_len - remain_size + 1;
        strncat( *dest, source, source_len);
        *used_num = *used_num + source_len;
    }
}


























