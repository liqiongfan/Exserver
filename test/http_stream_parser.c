/**
 * File:http_stream_parser.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include "../kernel/exlist.h"
#include "../kernel/types.h"

int main(int argc, char *argv[])
{
    size_t start_pos = 0;
    char *request_stream = "PUT / HTTP/1.1\r\n"
                           "Content-Type: application/json;charset=UTF8\r\n"
                           "Content-Length: 10\r\n"
                           "\r\n"
                           "{\"a\":\"aa\"}"
                           "POST /index.php?a=b HTTP/1.1\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: 5\r\n"
                           "\r\n"
                           "hello"
                           "GET /index.do HTTP/1.1\r\n"
                           "Accept: application/json;charset=UTF8\r\n"
                           "Connection: keep-alive\r\n"
                           "\r\n";
    
    EXLIST_V *ptr;
    while (1)
    {
        EXLIST *http_headers = parse_http_stream(request_stream, &start_pos);
        EXLIST_FOREACH(http_headers, ptr) {
            EX_HTTP_HEADER *data = ELV_VALUE_P(ptr);
            printf("[%s]:[%s]\n", HEADER_KEY_P(data), HEADER_VALUE_P(data));
        } EXLIST_FOREACH_END();
        destroy_exlist(http_headers);
        if ( start_pos == strlen(request_stream) ) break;
        printf("\n");
    }
    return 0;
}