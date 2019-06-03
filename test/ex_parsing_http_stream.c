/**
 * Copyright @2019 Exserver All rights reserved
 */

#include <stdio.h>
#include <ex_string.h>
#include <ex_socket.h>

#include <ex_list.h>
#include <ex_http_stream.h>


int main(int argc, char *argv[])
{
    int  cd, fd;
    long le, np;
    
    fd = ex_create_socket("0.0.0.0", 8181);
    
    ex_listen_socket(fd, 1000);
    
    cd = accept(fd, NULL, NULL);
    
    char *s = ex_read_requests(cd, &le);
    
    while(true)
    {
        EXLIST_V *ptr;
        EXLIST   *stream = ex_parse_http_stream( s, le, &np );
        
        EXLIST_FOREACH( stream, ptr )
        {
            EX_HTTP_HEADER *header = ELV_VALUE_P( ptr );
            
            printf( "[%s]:[%s]\n", HEADER_KEY_P( header ), HEADER_VALUE_P( header ) );
            
            if ( strncmp( HEADER_KEY_P( header ), EX_STRL( "http_body" ) ) == 0 ) {
                FILE *ttp = fopen( "temp-bk.log", "a+" );
                fwrite( HEADER_VALUE_P( header ), sizeof( char ), 4335530, ttp );
                fclose( ttp );
            }
        }
        EXLIST_FOREACH_END();
        destroy_exlist( stream );
        
        write( cd, "HTTP/1.1 200 OK\r\nContent-Length:5\r\n\r\nHello", 42 );
        
        if ( le == np ) {
            break;
        }
    }
    close(fd);
    
    return 0;
}


