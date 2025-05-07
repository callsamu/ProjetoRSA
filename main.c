#include "fiobj_hash.h"
#include "fiobj_mustache.h"
#include "fiobject.h"
#include "http.h" /* the HTTP facil.io extension */
#include "mustache_parser.h"

#include "rsa.h"

void on_request(http_s *request);

int main(int argc, char const **argv) {
	printf("Starting facil.io server...\n");
	int c = add(1, 2);

	http_listen("3000", "localhost", .on_request = on_request, .log = 1);
	fio_start(.threads = 1);
}

void on_request(http_s *request) {
	http_set_header(request, HTTP_HEADER_CONTENT_TYPE, http_mimetype_find("html", 4));

	mustache_error_en err;
	const char *templates_file = "./templates/main.html";

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
