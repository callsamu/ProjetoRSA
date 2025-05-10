#include "fiobj_hash.h"
#include "fiobject.h"
#include <stdlib.h>

#include "application.h"
#include "utils.h"
#include "handlers.h"

#define STATIC_DIRECTORY "/static/"

#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_NOT_ALLOWED 405

static FIOBJ HTTP_GET;
static FIOBJ HTTP_POST;

static void add_route(
	FIOBJ table, 
	FIOBJ method,
	const char *path, 
	HTTP_Handler *handler
) {
	FIOBJ method_table = fiobj_hash_get(table, method);

	if (method_table == FIOBJ_INVALID) {
		puts("Could not register routes: unrecognized method used");
		exit(1);
	}

	fiobj_hash_set(
		method_table, 
		fiobj_str_new(path, strlen(path)), 
		fiobj_num_new((intptr_t) handler)
	);
}

Application *init_app() {
	Application *app = malloc(sizeof(Application));
	FIOBJ table = fiobj_hash_new();

	HTTP_GET = fiobj_str_new("GET", 3);
	HTTP_POST = fiobj_str_new("POST", 4);

	fiobj_hash_set(table, HTTP_GET, fiobj_hash_new());
	fiobj_hash_set(table, HTTP_POST, fiobj_hash_new());

	add_route(table, HTTP_GET, "/", home_handler);
	add_route(table, HTTP_GET, "/encrypt", encrypt_form_handler);

	app->routing_table = table;
	return app;
}

void free_app(Application *app) {
	fiobj_free(app->routing_table);
	fiobj_free(HTTP_GET);
	fiobj_free(HTTP_POST);
	free(app);
}

int check_route_exists(FIOBJ method_table, void *arg) {
	return fiobj_hash_get(
		method_table, 
		(FIOBJ) arg
	) == FIOBJ_INVALID  ? -1 : 0;
}

void route_request(Application *app, http_s *request) {
	if (str_starts_with(request->path, STATIC_DIRECTORY)) {
		if (!fiobj_iseq(request->method, HTTP_GET)) {
			http_send_error(request, HTTP_METHOD_NOT_ALLOWED);
			return;
		}

		on_file_request(request);
		return;
	};

	FIOBJ method_table = fiobj_hash_get(app->routing_table, request->method);;

	if (method_table == FIOBJ_INVALID) {
		http_send_error(request, HTTP_NOT_FOUND);
		return;
	}

	FIOBJ handler = fiobj_hash_get(method_table, request->path);

	if (handler == FIOBJ_INVALID) {
		http_send_error(request, HTTP_NOT_FOUND);

		return;
	}

	HTTP_Handler *handler_fn = (HTTP_Handler *) fiobj_obj2num(handler);
	handler_fn(request);
}


void on_file_request(http_s *request) {
	FIOBJ static_path = fiobj_str_new(".", 1);
	fiobj_str_concat(static_path, request->path);

	FILE *file = fopen(fiobj_obj2cstr(static_path).data, "r");

	if (file) {
		int desc = fileno(file);

		fseek(file, 0L, SEEK_END);
		size_t size = ftell(file);
		rewind(file);

		int ret = http_sendfile(request, desc, size, 0);
		if (ret < 0) {
			printf("Error sending file: %s\n", strerror(errno));
		}

		fclose(file);
	}

	fiobj_free(static_path);
}
