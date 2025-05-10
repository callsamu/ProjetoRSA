#include "handlers.h"
#include "fiobj_hash.h"
#include "fiobj_str.h"
#include "mustache_parser.h"
#include "fiobject.h"
#include "http.h"
#include <gmp.h>

#include "utils.h"
#include "rsa.h"

#define HTTP_BAD_REQUEST 400
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

void write_template(http_s *request, const char *filename, FIOBJ data) {
	mustache_s *template = load_template(filename);

	if (!template) {
		fiobj_mustache_free(template);
		http_send_error(request, HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	FIOBJ html = fiobj_mustache_build(template, data);
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
	write_template(request, "templates/home.html", fiobj_null());
}

void key_form_handler(http_s *request) {
	write_template(request, "templates/publickey.html", fiobj_null());
}

void key_handler(http_s *request) {
	http_parse_body(request);

	mpz_t p;
	mpz_init(p);
	get_mpz_from_form(request->params, "p", &p);

	mpz_t q;
	mpz_init(q);
	get_mpz_from_form(request->params, "q", &q);

	mpz_t e;
	mpz_init(e);
	get_mpz_from_form(request->params, "e", &e);

	PublicKey chave;
	generate_public_key(p, q, e, &chave);

	char cwd[1024];
	getcwd(cwd, sizeof(cwd));

	char tmp_message[4098];
	sprintf(tmp_message, "Chave Pública salva em %s/texts/public_key.txt", cwd);

	FIOBJ data = fiobj_hash_new();
	fiobj_hash_set(data, fiobj_str_new("message", 7), fiobj_str_new(tmp_message, strlen(tmp_message)));
	write_template(request, "templates/message.html", data);

	fiobj_free(data);
	mpz_clears(p, q, e, NULL);
}

void encrypt_form_handler(http_s *request) {
	write_template(request, "templates/encrypt-form.html", fiobj_null());
}

void parse_key(FIOBJ str, PublicKey *key){
	fio_str_info_s s = fiobj_obj2cstr(str);

	char *buffer = malloc(s.len);	

	int i = 0;

	for (; s.data[i] != '\n' && s.data[i] != '\0';  i++) {
		buffer[i] = s.data[i];
	}

	buffer[i] = '\0';
	i++;

	mpz_set_str(key->n, buffer, 10);
	memset(buffer, 0, s.len);

	int j = 0;
	for (; s.data[i] != '\0';  i++, j++) {
		buffer[j] = s.data[i];
	}
	buffer[j] = '\0';
	i++;

	mpz_set_str(key->e, buffer, 10);
	free(buffer);
}

void encrypt_handler(http_s *request) {
	http_parse_body(request);

	FIOBJ message = fiobj_hash_get(request->params, fiobj_str_new("message", 7));
	if (message == FIOBJ_INVALID) {
		http_send_error(request, HTTP_BAD_REQUEST);
		return;
	}

	FIOBJ key = fiobj_hash_get(request->params, fiobj_str_new("key", 3));
	if (key == FIOBJ_INVALID) {
		http_send_error(request, HTTP_BAD_REQUEST);
		return;
	}

	PublicKey chave;
	mpz_init(chave.n);
	mpz_init(chave.e);

	parse_key(key, &chave);

	fio_str_info_s s = fiobj_obj2cstr(message);

	encrypt_message(s.data, &chave, "./texts/encrypted.txt");

	char cwd[1024];
	getcwd(cwd, sizeof(cwd));

	char tmp_message[4098];
	sprintf(tmp_message, "Mensagem encriptada salva em %s/texts/encrypted.txt", cwd);

	FIOBJ data = fiobj_hash_new();
	fiobj_hash_set(data, fiobj_str_new("message", 7), fiobj_str_new(tmp_message, strlen(tmp_message)));
	write_template(request, "templates/message.html", data);

	fiobj_free(data);
	mpz_clears(chave.n, chave.e, NULL);
}
