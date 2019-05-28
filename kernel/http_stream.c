/**
 * File:http_stream.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include "types.h"
#include <stdlib.h>

/* Get the HTTP HEADER structure */
EX_HTTP_HEADER *INIT_HEADER()
{
    EX_HTTP_HEADER *ptr = malloc(sizeof(EX_HTTP_HEADER));
    if ( ptr == NULL ) return NULL;
    memset(ptr, 0, sizeof(EX_HTTP_HEADER));
    return ptr;
}

/* Get the substring from source string, start with the start pos. with length
 * length, mode was the trim command
 * mode is 0: nothing to do
 * mode is 1: trim result string left
 * mode is 2: trim result string right
 * mode is 3: trim result both NOTE: The return value must be freed after used. */
char *exsubstr(const char *source, size_t start, size_t length, int mode)
{
    unsigned long _i;
    
    if ( source == NULL ) return NULL;

    size_t left = 0, right = 0, source_len = strlen(source);
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
    memset(result, 0, sizeof(char) * length);
    strncpy(result, source + start + left, length - 1);
    return result;
}

/* Get the result string which the white space has been trimmed
 * mode is 0: nothing to do
 * mode is 1: trim result string left
 * mode is 2: trim result string right
 * mode is 3: trim result both NOTE: The return value must be freed after used. */
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

/* Reading the socket data from socket file descriptor
 * if return NULL means the server can close the file descriptor
 * otherwise free the return value */
char *get_socket_stream_data(int __sock_fd, size_t *stream_length)
{
    char buff[BUFFER_SIZE] = {0};
    char *result_buff = NULL;
    memset(buff, 0, sizeof(char) * BUFFER_SIZE);
    *stream_length = 0;
    
    int times = 1;
    size_t realloc_size, buff_size;
    ssize_t read_num;
    
    while ( true )
    {
        read_num = read( __sock_fd, buff, sizeof( char ) * BUFFER_SIZE );
    
        buff_size = strlen(buff);
        
        /* Peers closed the socket */
        if ( read_num == 0 )
            return NULL;
        if ( read_num == -1 )
        {
            if ( errno != EWOULDBLOCK || errno != EAGAIN )
                return NULL;
            else
                continue;
        }
        
        /* Others were ok */
        realloc_size = sizeof(char) * (BUFFER_SIZE * times + 1);
        void *ptr = realloc(result_buff, realloc_size);
        if ( ptr == NULL ) free(result_buff);
        assert(ptr != NULL);
        result_buff = ptr;
        strncpy(result_buff + ( (times - 1) * BUFFER_SIZE ), buff, BUFFER_SIZE);
        times++;
        *stream_length += buff_size;
        
        /* after check and reading if can be exited */
        if ( read_num < BUFFER_SIZE || buff_size == 0 ) break;
    }
    
    return result_buff;
}

/* Free the memory after using the http parsed stream */
static void __free_http_header__(void *header)
{
    EX_HTTP_HEADER *data = ELV_VALUE_P((EXLIST_V *)header);
    free(HEADER_KEY_P(data));
    free(HEADER_VALUE_P(data));
    free(data);
}

/* Parse the http stream
 * if http_request_stream is invalid, return NULL, otherwise return the response data. */
EXLIST *parse_http_stream(const char *http_request_stream, size_t *str_len)
{
    int http_method_kind = 0, body_empty = 0;
    size_t _i = *str_len, _colon_pos = 0, _content_length = 0;
    EXLIST *__list_data = INIT_EXLIST();
    EX_HTTP_HEADER *value_data;
    size_t _http_stream_length = strlen(http_request_stream), _i_pos = _i, _white_count = 0;
    
    /* Parse the HTTP stream's first line when meeting the first \r\n stop */
    for ( ; _i < _http_stream_length; ++_i )
    {
        char current_char = http_request_stream[_i];
        
        if ( current_char == ' ' ) {
            switch (_white_count)
            {
                case HTTP_METHOD:
                    /* Request method */
                    value_data = INIT_HEADER();
                    HEADER_KEY_P(value_data) = extrim("request_method", TRIM_NONE);
                    HEADER_VALUE_P(value_data) = exsubstr(http_request_stream, _i_pos, _i - _i_pos, TRIM_NONE);
                    push_tail(__list_data, value_data, __free_http_header__);
                    
                    /* When in GET mode the http_body can be empty */
                    if ( strncmp(HEADER_VALUE_P(value_data), "GET", 3) == 0 ) http_method_kind = GET;
                    break;
                case HTTP_URI:
                    /* Request URI */
                    value_data = INIT_HEADER();
                    HEADER_KEY_P(value_data) = extrim("request_uri", TRIM_NONE);
                    HEADER_VALUE_P(value_data) = exsubstr(http_request_stream, _i_pos, _i - _i_pos, TRIM_NONE);
                    push_tail(__list_data, value_data, __free_http_header__);
                    break;
            }
            _i_pos = ( size_t )_i + 1;
            _white_count++;
        }
        
        if ( current_char == '\r' && http_request_stream[_i + 1] == '\n' )
        {
            if ( _white_count == HTTP_VERSION )
            {
                /* Request HTTP/VERSION */
                value_data = INIT_HEADER();
                HEADER_KEY_P(value_data) = extrim("http_version", TRIM_NONE);
                HEADER_VALUE_P(value_data) = exsubstr(http_request_stream, _i_pos, _i - _i_pos, TRIM_NONE);
                push_tail(__list_data, value_data, __free_http_header__);
            }
            _i_pos = ( size_t )_i + 2;
            break;
        }
    }
    
    /* Parse the second line to last second line */
    for ( _i = _i_pos; _i < _http_stream_length; ++_i )
    {
        char current_char = http_request_stream[ _i ];
        if ( current_char == ':' ) _colon_pos = _i + 1;
        if ( current_char == '\r' && http_request_stream[ _i + 1] == '\n' && _colon_pos )
        {
            if (http_request_stream[ _i + 2 ] != '\r' && http_request_stream[ _i + 3] != '\n')
            {
                /* Request HTTP/VERSION */
                value_data = INIT_HEADER();
                HEADER_KEY_P(value_data) = exsubstr(http_request_stream, _i_pos, _colon_pos - _i_pos - 1, TRIM_RIGHT);
                HEADER_VALUE_P(value_data) = exsubstr(http_request_stream, _colon_pos, _i - _colon_pos, TRIM_LEFT);
                push_tail(__list_data, value_data, __free_http_header__);
                
                /* Get the content-length */
                if ( strncasecmp(HEADER_KEY_P(value_data), "content-length", 14) == 0 )
                {
                    _content_length = (size_t)strtol(HEADER_VALUE_P(value_data), EMPTY_PTR, 10);
                }
                
                _i_pos = ( size_t )_i + 2;
            }
            else if (http_request_stream[ _i + 2 ] == '\r' && http_request_stream[ _i + 3] == '\n')
            {
                /* Insert the last header */
                value_data = INIT_HEADER();
                HEADER_KEY_P(value_data) = exsubstr(http_request_stream, _i_pos, _colon_pos - _i_pos - 1, TRIM_RIGHT);
                HEADER_VALUE_P(value_data) = exsubstr(http_request_stream, _colon_pos, _i - _colon_pos, TRIM_LEFT);
                push_tail(__list_data, value_data, __free_http_header__);
                /* Get the content-length */
                if ( strncasecmp(HEADER_KEY_P(value_data), "content-length", 14) == 0 )
                {
                    _content_length = (size_t)strtol(HEADER_VALUE_P(value_data), EMPTY_PTR, 10);
                }
                
                /* Request HTTP_BODY */
                value_data = INIT_HEADER();
                HEADER_KEY_P(value_data) = extrim("http_body", TRIM_NONE);
                HEADER_VALUE_P(value_data) = exsubstr(http_request_stream, _i + 4, _content_length, TRIM_NONE);
                push_tail(__list_data, value_data, __free_http_header__);
                
                /* Set the last start pos.
                *str_len = _i + 4 + _content_length; */
                _i_pos = _i + 4 + _content_length; body_empty = 1;
                break;
            }
        }
    }
    
    /* if not GET request and http body was empty, the http_request is not valid */
    if ( http_method_kind != GET && !body_empty )
    {
        goto invalid_stream;
    }
    
    /* if is valid request, set the result length */
    *str_len = _i_pos;
    
    /* Return the data to the outer space,
     * NOTE that to free it after used. */
    return __list_data;
    
invalid_stream:
    destroy_exlist(__list_data);
    return NULL;
}

/* Parse the http query string it can be start with ? or not, for example:
 * query_string: ?a=b&c=d or a=b&c=d */
EXLIST *parse_query_string(const char *query_string)
{
    if ( query_string == NULL ) return NULL;
    
    size_t  _i = 0,
            string_len = strlen(query_string),
            start_pos = _i,
            equal_pos = _i;
    if ( *query_string == '?' ) start_pos = _i = 1;
    
    EX_EQUAL_DATA *data;
    EXLIST *result_list = INIT_EXLIST();
    
    for ( ; _i < string_len; ++_i )
    {
        if ( *(query_string + _i) == '=' ) equal_pos = _i;
        if ( *(query_string + _i) == '&')
        {
            data = INIT_HEADER();
            HEADER_KEY_P(data) = exsubstr(query_string, start_pos, equal_pos - start_pos, TRIM_NONE);
            HEADER_VALUE_P(data) = exsubstr(query_string, equal_pos + 1, _i - equal_pos - 1, TRIM_NONE);
            push_tail(result_list, data, __free_http_header__);
            start_pos = _i + 1;
        }
        else if ( _i == string_len - 1 )
        {
            data = INIT_HEADER();
            HEADER_KEY_P(data) = exsubstr(query_string, start_pos, equal_pos - start_pos, TRIM_NONE);
            HEADER_VALUE_P(data) = exsubstr(query_string, equal_pos + 1, _i - equal_pos, TRIM_NONE);
            push_tail(result_list, data, __free_http_header__);
        }
    }
    
    return result_list;
}

/* Concatenate source string to the end of the dest string
 * origin_size must be the first dest string's all allocated size
 * used_num were the outer pointer let the kernel know the origin_size - used_num
 * is the left space to store the source string, if not enough allocated more */
void
_ex_strncat_(char **dest, const char *source, size_t *origin_size, size_t *used_num)
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

/* Generate the response string
 * Parameter body's follower were http headers
 * NOTE: return value need to be freed after used */
char *generate_response_string(int code, char *msg, char *body, int n, ...)
{
    char code_str[5] = {0}, *header;
    va_list args;
    size_t total_size = 1, used_size = 0;
    char *response_stream = malloc(sizeof(char));
    if ( response_stream == NULL ) return NULL;
    memset(response_stream, 0, sizeof( char ));
    
    /* HTTP protocol */
    _ex_strncat_(&response_stream, "HTTP/1.1 ", EX_CON(total_size, used_size));
    
    /* HTTP status code  */
    sprintf(code_str, "%d ", code);
    _ex_strncat_(&response_stream, code_str, EX_CON(total_size, used_size));
    
    /* HTTP status msg */
    _ex_strncat_(&response_stream, msg, EX_CON(total_size, used_size));
    _ex_strncat_(&response_stream, "\r\n", EX_CON(total_size, used_size));
    
    /* Add the HTTP headers */
    va_start(args, n);
    while(true)
    {
        if ( !n-- ) break;
        header = va_arg(args, char *);
        _ex_strncat_(&response_stream, header, EX_CON(total_size, used_size));
        _ex_strncat_(&response_stream, "\r\n", EX_CON(total_size, used_size));
    }
    va_end(args);
    
    /* HTTP body */
    _ex_strncat_(&response_stream, "\r\n", EX_CON(total_size, used_size));
    _ex_strncat_(&response_stream, body, EX_CON(total_size, used_size));
    
    return response_stream;
}

/* Generate the request string
 * Parameter body's follower were http headers
 * NOTE: return value need to be freed after used */
char *generate_request_string(char *method, char *url, char *body, int n, ...)
{
    char *header;
    va_list args;
    size_t total_size = 1, used_size = 0;
    char *response_stream = malloc(sizeof(char));
    if ( response_stream == NULL ) return NULL;
    memset(response_stream, 0, sizeof( char ));
    
    /* HTTP method */
    _ex_strncat_(&response_stream, method, EX_CON(total_size, used_size));
    _ex_strncat_(&response_stream, " ", EX_CON(total_size, used_size));
    
    /* HTTP url  */
    _ex_strncat_(&response_stream, url, EX_CON(total_size, used_size));
    _ex_strncat_(&response_stream, " ", EX_CON(total_size, used_size));
    
    /* HTTP version */
    _ex_strncat_(&response_stream, "HTTP/1.1", EX_CON(total_size, used_size));
    _ex_strncat_(&response_stream, "\r\n", EX_CON(total_size, used_size));
    
    /* Add the HTTP headers */
    va_start(args, n);
    while(true)
    {
        if ( !n-- ) break;
        header = va_arg(args, char *);
        _ex_strncat_(&response_stream, header, EX_CON(total_size, used_size));
        _ex_strncat_(&response_stream, "\r\n", EX_CON(total_size, used_size));
    }
    va_end(args);
    
    /* HTTP body */
    _ex_strncat_(&response_stream, "\r\n", EX_CON(total_size, used_size));
    _ex_strncat_(&response_stream, body, EX_CON(total_size, used_size));
    
    return response_stream;
}























