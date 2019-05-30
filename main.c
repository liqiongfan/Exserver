#include <stdio.h>

#include "kernel/net/http_socket.h"
#include "kernel/exlist.h"
#include "kernel/types.h"
#include "kernel/net/socket.h"

void delete_configs()
{
	destroy_exjson(configs);
}

int main(int argc, char *argv[])
{
	generate_worker(3, NULL);
	static_server_start();

	atexit(delete_configs);
	return 0;
}