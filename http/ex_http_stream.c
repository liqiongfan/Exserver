/**
 * File:http_stream.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include <stdlib.h>
#include <ex_string.h>
#include <ex_log.h>
#include <ex_http_stream.h>

/* Get the HTTP HEADER structure */
EX_HTTP_HEADER *INIT_HEADER()
{
	EX_HTTP_HEADER *ptr = malloc(sizeof(EX_HTTP_HEADER));
	if ( ptr == NULL ) return NULL;
	ex_memzero(ptr, sizeof(EX_HTTP_HEADER));
	return ptr;
}

/* Free the memory after using the http parsed stream */
static void __free_http_header__(void *header)
{
	EX_HTTP_HEADER *data = ELV_VALUE_P((EXLIST_V *)header);
	free(HEADER_KEY_P(data));
	free(HEADER_VALUE_P(data));
	free(data);
}

EXLIST *ex_parse_http_stream(char *http_request_stream, long stream_length, long *str_len)
{
    char *b;
	int http_method_kind = 0, body_empty = 0, set_colon = 0;
	long _i = *str_len, _colon_pos = 0, _content_length = 0;
	EXLIST *__list_data = INIT_EXLIST();
	EX_HTTP_HEADER *value_data;
	long _http_stream_length = stream_length, _i_pos = _i, _white_count = 0;

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
		if ( current_char == ':' && !set_colon ) { _colon_pos = _i + 1; set_colon = 1; }
		if ( current_char == '\r' && http_request_stream[ _i + 1] == '\n' && _colon_pos )
		{
			set_colon = 0;

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
				if ( _content_length )
				{
					/*HEADER_VALUE_P(value_data) = exsubstr(http_request_stream, _i + 4, _content_length, TRIM_NONE);*/
					b = malloc(sizeof(char) * _content_length);
					ex_copymem(b, http_request_stream + (_i + 4), _content_length * sizeof(char));
					HEADER_VALUE_P(value_data) = b;
				} else HEADER_VALUE_P(value_data) = NULL;
                /* push_tail(__list_data, value_data, __free_http_header__); */
				append_head(__list_data, value_data, __free_http_header__);

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

EXLIST *ex_parse_query_string(const char *qs)
{
	if ( qs == NULL ) return NULL;

	size_t  _i, sl, sp, ep;
	
	sp = ep = _i = 0;
	sl = strlen(qs);
	
	if ( *qs == '?' ) sp = _i = 1;

	EX_EQUAL_DATA *data;
	EXLIST *result_list = INIT_EXLIST();

	for ( ; _i < sl; ++_i )
	{
		if ( *(qs + _i) == '=' ) ep = _i;
		if ( *(qs + _i) == '&')
		{
			data = INIT_HEADER();
			HEADER_KEY_P(data) = exsubstr(qs, sp, ep - sp, TRIM_NONE);
			HEADER_VALUE_P(data) = exsubstr(qs, ep + 1, _i - ep - 1, TRIM_NONE);
			push_tail(result_list, data, __free_http_header__);
			sp = _i + 1;
		}
		else if ( _i == sl - 1 )
		{
			data = INIT_HEADER();
			HEADER_KEY_P(data) = exsubstr(qs, sp, ep - sp, TRIM_NONE);
			HEADER_VALUE_P(data) = exsubstr(qs, ep + 1, _i - ep, TRIM_NONE);
			push_tail(result_list, data, __free_http_header__);
		}
	}

	return result_list;
}

char *generate_response_string(int code, char *msg, char *body, int n, ...)
{
	char code_str[35] = {0}, *header;
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

	/* HTTP Content-Length */
/*
    memset(code_str, 0, sizeof(code_str));
    sprintf(code_str, "Content-Length: %ld\r\n", strlen(body));
    _ex_strncat_(&response_stream, code_str, EX_CON(total_size, used_size));
*/

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
	if ( body )
		_ex_strncat_(&response_stream, body, EX_CON(total_size, used_size));

	return response_stream;
}

void send_404_response(int _fd, int keep)
{
	char *response;
	if ( keep ) {
		response = generate_response_string(
				404, "Not Found", "Not Found",
				3,
				"Connection: keep-alive",
				"Content-Type: text/html",
				"Content-Length: 9"
		);
	}
	else
	{
		response = generate_response_string(
				404, "Not Found", "Not Found",
				3,
				"Connection: close",
				"Content-Type: text/html",
				"Content-Length: 9"
		);
	}
	write(_fd, response, strlen(response));
	free(response);
}

char *generate_request_string(char *method, char *url, char *body, int n, ...)
{
	char *header, code_str[35] = {0};
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

	/* HTTP Content-Length */
	memset(code_str, 0, sizeof(code_str));
	sprintf(code_str, "Content-Length: %ld\r\n", strlen(body));
	_ex_strncat_(&response_stream, code_str, EX_CON(total_size, used_size));

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

#ifdef __linux__
static char *
/* Common function to read linux proc system file */
parse_proc_file(const char *filename)
{
	char temp_data[100];
	size_t length, total_size = 1, used_num=0, all_seek=0;
	char *str = malloc(sizeof(char));

	FILE *fp = fopen(filename, "r");
	while ( true )
	{
		if ( feof(fp) ) break;
		memset(temp_data, 0, sizeof(temp_data));
		fread(temp_data, sizeof(char), sizeof(temp_data), fp);
		length = strlen(temp_data);
		if ( length == 0 ) break;
		all_seek += length + 1;
		fseek(fp, all_seek, SEEK_SET);
		_ex_strncat_(&str, temp_data, EX_CON(total_size, used_num));
		_ex_strncat_(&str, " ", EX_CON(total_size, used_num));
	}

	return str;
}

/* Remember to free the returned string
 * Only support Linux proc system
 * Feature: read process cmdline which store in /proc/PID/cmdline */
char *parse_proc_cmdline(int pid)
{
	char file_temp[100];
	sprintf(file_temp, "/proc/%d/cmdline", pid);
	char *str = parse_proc_file(file_temp);
	return str;
}
#endif

/* Binary safe */
char *ex_copy_data_from_file(char *file, long *size)
{
	FILE    *fp;
	size_t   bl, al,  ul;
	char    *r, *tr,  bf[BUFFER_SIZE];

	r  = NULL;
	ul = al = 0;

	fp = fopen(file, "r+");
	if ( fp == NULL ) return NULL;

	while (true)
	{
		if ( feof(fp) ) break;
		ex_memzero(bf, sizeof(bf));
		bl = fread(bf, sizeof(char), sizeof(bf), fp);
		if ( bl == 0 ) break;
		al += bl;
		*size += bl;
		fseek(fp, al, SEEK_SET);
		tr = realloc(r, sizeof(char) * al);
		if ( tr == NULL ) {
			free(r);
			break;
		}
		r = tr;
		ex_copymem(r + ul, bf, sizeof(char) * bl);
		ul += bl;
	}

	fclose(fp);
	return r;
}

/* Not binary safe */
char *get_file_data(char *filename)
{
	FILE *fp = fopen(filename, "r");
	if ( fp == NULL ) return NULL;

	size_t total_size = 1, used_num = 0, str_len, all_seek_pos = 0;
	char temp_str[100], *result = malloc(sizeof(char));
	if ( result == NULL ) return NULL;
	memset(result, 0, sizeof(char));

	while ( true )
	{
		memset(temp_str, 0, sizeof(temp_str));
		fread(temp_str, sizeof(char), sizeof(temp_str) - 1, fp);
		str_len = strlen(temp_str);
		all_seek_pos += str_len;
		if ( str_len == 0 ) break;
		fseek(fp, all_seek_pos, SEEK_SET);
		_ex_strncat_(&result, temp_str, EX_CON(total_size, used_num));
	}
	fclose(fp);
	return result;
}

void ex_init_request(EX_HTTP_HEADER *header, EXLIST_V *ptr, EX_REQUEST_T *req)
{
    if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_HOST) ) == 0 )
    {
        req->server_host = HEADER_VALUE_P(header);
    }
    else if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_METHOD) ) == 0 )
    {
        req->request_method = HEADER_VALUE_P(header);
    }
    else if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_URL) ) == 0 )
    {
        req->request_url = HEADER_VALUE_P(header);
    }
    else if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_C_TYPE)) == 0 )
    {
        req->content_type = HEADER_VALUE_P(header);
    }
    else if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_BODY) ) == 0 )
    {
        req->http_body = HEADER_VALUE_P(header);
    }
    else if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_VERSION) ) == 0 )
    {
        req->http_version = HEADER_VALUE_P(header);
        if ( strncasecmp(req->http_version, EX_STRL(EX_HTTP_1_0)) == 0 )
        {
            req->http_version_i = 0;
            req->keep_alive     = 0;
        }
        else if ( strncasecmp(req->http_version, EX_STRL(EX_HTTP_1_1)) == 0 )
        {
            req->http_version_i = 1;
            req->keep_alive     = 1;
        }
        else if (strncasecmp(req->http_version, EX_STRL(EX_HTTP_2_0)) == 0 )
        {
            req->http_version_i = 2;
            req->keep_alive     = 1;
        }

        /* Next is HTTP headers */
        req->headers = ELV_NEXT_P(ptr);
    }
    else if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_LENGTH)) == 0 )
    {
        req->http_body_length = strtol(HEADER_VALUE_P(header), NULL, 10);
    }
    else if ( strncasecmp(HEADER_KEY_P(header), EX_STRL(EX_CONNECT)) == 0 )
    {
        if ( strncasecmp(HEADER_VALUE_P(header), EX_STRL(EX_KEEP)) == 0 )
        {
            req->keep_alive = 1;
        }
        else
        {
            req->keep_alive = 0;
        }
    }
}



















