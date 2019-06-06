/**
 * Copyright @2019 Exserver All Rights Reserved.
 */

#include <ex_http.h>

int main(int argc, char *argv[])
{
    ex_http_worker_init(3, NULL);
    
    ex_http_server_init();

	return 0;
}

