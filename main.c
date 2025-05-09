#include "fiobj_hash.h"
#include "fiobj_mustache.h"
#include "fiobj_str.h"
#include "fiobject.h"
#include "http.h" /* the HTTP facil.io extension */
#include "mustache_parser.h"

#include "rsa.h"
#include <stdio.h>


#define HTTP_METHOD_NOT_ALLOWED 405

static FIOBJ HTTP_METHOD_GET;
#define IS_GET(request) fiobj_iseq(request->method, HTTP_METHOD_GET)

#define STATIC_DIRECTORY "/static"

void on_request(http_s *request);
int str_starts_with(FIOBJ str, const char *prefix);
void on_file_request(http_s *request);

int main(int argc, char const **argv) {
	printf("Starting facil.io server...\n");

	HTTP_METHOD_GET = fiobj_str_new("GET", 3);

	http_listen("3000", "localhost", .on_request = on_request, .log = 1);
	fio_start(.threads = 1);

	fiobj_free(HTTP_METHOD_GET);
}

int str_starts_with(FIOBJ str, const char *prefix) {
	fio_str_info_s s = fiobj_obj2cstr(str);
	return strncmp(s.data, prefix, strlen(prefix)) == 0;
}

void on_request(http_s *request) {
	mustache_error_en err;
	const char *templates_file = "./templates/main.html";

	if (str_starts_with(request->path, STATIC_DIRECTORY)) {
		if (!IS_GET(request)) {
			http_send_error(request, HTTP_METHOD_NOT_ALLOWED);
			return;
		}

		on_file_request(request);
		return;
	};

	mustache_s *template = fiobj_mustache_new(
		.filename = templates_file,
		.filename_len = strlen(templates_file),
		.err = &err
	);

	if (err) {
		switch (err) {
		case MUSTACHE_ERR_FILE_NOT_FOUND:
			printf("Template not found\n");
			break;
		case MUSTACHE_ERR_UNKNOWN:
			printf("Syntax error\n");
		default:
			printf("Unknown error\n");
			break;
		}

		http_send_error(request, err);
		return;
	}

	FIOBJ html = fiobj_mustache_build(template, fiobj_hash_new());
	fio_str_info_s html_string = fiobj_obj2cstr(html);

	http_send_body(request, html_string.data, html_string.len);

	fiobj_mustache_free(template);
	fiobj_free(html);
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
