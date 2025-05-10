#include "handlers.h"
#include "mustache_parser.h"
#include "fiobject.h"
#include "http.h"

#define HTTP_INTERNAL_SERVER_ERROR 500

mustache_s *load_template(const char *filename) {
	mustache_error_en err;

	mustache_s *template = fiobj_mustache_new(
		.filename = filename,
		.filename_len = strlen(filename),
		.err = &err
	);

	if (err) {
		printf("Error loading template '%s': ", filename);

		switch (err) {
		case MUSTACHE_ERR_FILE_NOT_FOUND:
			printf("not found\n");
			break;
		case MUSTACHE_ERR_UNKNOWN:
			printf("syntax error\n");
		default:
			printf("unknown error\n");
			break;
		}

		return NULL;
	}

	return template;
}

void home_handler(http_s *request) {
	mustache_s *template = load_template("templates/main.html");
	if (!template) {
		http_send_error(request, HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	FIOBJ html = fiobj_mustache_build(template, fiobj_hash_new());
	fio_str_info_s html_string = fiobj_obj2cstr(html);

	http_send_body(request, html_string.data, html_string.len);

	fiobj_mustache_free(template);
	fiobj_free(html);
}

void encrypt_form_handler(http_s *request) {
	mustache_s *template = load_template("templates/encrypt.html");
	if (!template) {
		http_send_error(request, HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	FIOBJ html = fiobj_mustache_build(template, fiobj_hash_new());
	fio_str_info_s html_string = fiobj_obj2cstr(html);

	http_send_body(request, html_string.data, html_string.len);

	fiobj_mustache_free(template);
	fiobj_free(html);
}
