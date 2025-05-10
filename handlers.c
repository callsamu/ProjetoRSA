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

FIOBJ add_content_to_layout(FIOBJ content) {
	mustache_s *template = load_template("templates/base.html");

	if (!template) {
		fiobj_mustache_free(template);
		return FIOBJ_INVALID;
	}

	FIOBJ data = fiobj_hash_new();
	fiobj_hash_set(data, fiobj_str_new("content", 7), content);

	FIOBJ html = fiobj_mustache_build(template, data);
	fiobj_mustache_free(template);

	return html;
}

void write_template(http_s *request, const char *filename) {
	mustache_s *template = load_template(filename);

	if (!template) {
		fiobj_mustache_free(template);
		http_send_error(request, HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	FIOBJ html = fiobj_mustache_build(template, fiobj_hash_new());
	fiobj_mustache_free(template);

	FIOBJ htmx = fiobj_hash_get(request->headers, fiobj_str_new("hx-request", 10));

	if (!fiobj_iseq(htmx, fiobj_str_new("true", 4))) {
		puts("Not an HX request");
		FIOBJ _html = add_content_to_layout(html);
		fiobj_free(html);

		if (_html == FIOBJ_INVALID) {
			http_send_error(request, HTTP_INTERNAL_SERVER_ERROR);
			return;
		}

		html = _html;
	}

	fio_str_info_s html_string = fiobj_obj2cstr(html);
	http_send_body(request, html_string.data, html_string.len);

	fiobj_free(html);
}

void home_handler(http_s *request) {
	write_template(request, "templates/home.html");
}

void encrypt_form_handler(http_s *request) {
	write_template(request, "templates/encrypt.html");
}
