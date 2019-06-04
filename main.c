/**
 * Copyright @2019 Exserver All Rights Reserved.
 */

#include <ex_http_server.h>

int main(int argc, char *argv[])
{
	ex_gen_worker(4, NULL);
	ex_http_server_from_config();

	return 0;
}

