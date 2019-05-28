/**
 * File:generate_http_stream.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include "../kernel/exlist.h"
#include "../kernel/types.h"


int main(int argc, char *argv[])
{
    char *response_stream = generate_response_string(
        200, "OK", "{\"a\":\"aa\"}",
        2,
        "Content-Type: application/json;charset=utf8",
        "dd: dd"
    );
    printf("[%s]\n", response_stream);
    free(response_stream);
    
    
    char *request_tream = generate_request_string(
        "GET", "/index.php", "",
        1,
        "Accept: application/json;charset=utf8"
    );
    printf("[%s]\n", request_tream);
    free(request_tream);
    return 0;
}