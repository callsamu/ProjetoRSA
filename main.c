#include "fiobj_hash.h"
#include "fiobj_mustache.h"
#include "fiobj_str.h"
#include "fiobject.h"
#include "http.h" /* the HTTP facil.io extension */

#include "rsa.h"
#include <stdio.h>

#include "application.h"

void on_request(http_s *request) {
	Application *app = GET_APPLICATION(request);
	route_request(app, request);
}

int main(int argc, char const **argv) {
	Application app;

	init_app(&app);
	printf("Starting facil.io server...\n");



	http_listen(
		"3000", 
		"localhost", 
		.on_request = on_request, 
		.udata = (void *) &app,
		.log = 1
	);

	fio_start(.threads = 1);

	free_app(&app);
}
