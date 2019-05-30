/**
 * File:static_server.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include <stdio.h>

#include "kernel/net/http_socket.h"
#include "kernel/exlist.h"
#include "kernel/types.h"
#include "kernel/net/socket.h"

#include <sys/stat.h>
#ifdef __linux__
#include <sys/sendfile.h>
#else
#include <sys/uio.h>
#endif

void server_callback(int fd, EX_REQUEST_T *request_t)
{
    int _server_index, _index, is_match = 0;
    long  server_port = 0;
    char *web_root = NULL, *server_host = NULL, match_root[256], *real_url = NULL;
    EXJSON *server_data_ptr = exjson_get_val_from_key(configs, "server");
    if ( server_data_ptr == NULL
         || E_TYPE_P(server_data_ptr) != EX_ARRAY )
    {
        printf("server node empty or config syntax wrong!");
        exit(0);
    }
    
    /* cut request_url */
    char *query_pos = strchr(request_t->request_url, '?');
    if ( query_pos ) real_url = exsubstr(request_t->request_url, 0, query_pos - request_t->request_url, TRIM_NONE);
    else real_url = request_t->request_url;
    
    for (_server_index = 0; _server_index < E_NUM_P(server_data_ptr); ++_server_index)
    {
        memset(match_root, 0, sizeof(match_root));
        
        EXJSON *server = EV_VALUE_P(E_DATA_P(server_data_ptr) + _server_index);
        if ( E_TYPE_P(server) != EX_OBJECT )
        {
            printf("config syntax wrong!");
            exit(0);
        }
        for (_index = 0; _index < E_NUM_P(server); ++_index)
        {
            EXJSON_V *webserver = E_DATA_P(server) + _index;
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_WEBROOT) == 0 )
            {
                web_root = (char *)EV_VALUE_P(webserver);
            }
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_HOST) == 0 )
            {
                server_host = (char *)EV_VALUE_P(webserver);
            }
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_LISTEN) == 0 )
            {
                server_port = *(long *)EV_VALUE_P(webserver);
            }
        }
        
        if ( !web_root || !server_host || !server_port )
        {
            printf("Config file syntax wrong!");
            exit(0);
        }
        
        if ( server_port == 80 ) sprintf(match_root, "%s", server_host);
        else sprintf(match_root, "%s:%ld", server_host, server_port);
        
        /* See the matching webroot */
        if ( strncasecmp(request_t->server_host, match_root, strlen(match_root)) == 0 )
        {
            is_match = 1; break;
        }
    }
    
    if ( !is_match )
    {
        printf("Not match: %s, %s, %s\n", request_t->server_host, server_host, match_root);
        return send_404_response(fd);
    }
    
    /* if empty query string */
    int data_fd;
    if (*(real_url + 1) == '\0' && *real_url == '/') {
        sprintf(match_root, "%s/index.html", web_root);
    } else {
        sprintf(match_root, "%s/%s", web_root, real_url);
    }
    if ( query_pos ) free(real_url);
    
    data_fd = open(match_root, O_RDONLY);
    if ( data_fd == -1 )
    {
        return send_404_response(fd);
    }
    
    struct stat stat_buf;
    fstat( data_fd, &stat_buf );
    
    char *response, content_length[30];
    sprintf(content_length, "Content-Length: %d", stat_buf.st_size);
    
    if ( request_t->keep_alive )
        response = generate_response_string(
            200, "OK", "",
            4,
            "Server: Exserver/1.0",
            "Content-Type: text/html",
            "Connection: keep-alive",
            content_length
        );
    else
        response = generate_response_string(
            200, "OK", "",
            3,
            "Content-Type: text/html",
            "Server: Exserver/1.0",
            "Connection: close",
            content_length
        );
    
    write(fd, response, strlen(response));
#ifdef __linux__
    sendfile(fd, data_fd, 0, stat_buf.st_size);
#else
    /*struct iovec io_vec;
    io_vec.iov_base = response;
    io_vec.iov_len = strlen(response);
    struct sf_hdtr sf_hdtr1;
    sf_hdtr1.headers = &io_vec;
    sf_hdtr1.hdr_cnt = 1;*/
    
    sendfile(data_fd, fd, 0, stat_buf.st_size, NULL, 0);
#endif
    
    free(response);
}

void delete_configs()
{
    destroy_exjson(configs);
}

int main(int argc, char *argv[])
{
    int server_fd;
    
    server_fd = http_server_init("0.0.0.0", 8181, 20000);
    generate_worker(3, server_callback);
    master_process(server_fd);
    
    atexit(delete_configs);
    
    return 0;
}