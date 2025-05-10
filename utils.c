#include "utils.h"
#include "fiobj_hash.h"

int str_starts_with(FIOBJ str, const char *prefix) {
	fio_str_info_s s = fiobj_obj2cstr(str);
	return strncmp(s.data, prefix, strlen(prefix)) == 0;
}


int get_mpz_from_form(FIOBJ form, const char *name, mpz_t *number) {
	FIOBJ num = fiobj_hash_get(form, fiobj_str_new(name, strlen(name)));
	if (num == FIOBJ_INVALID) {
		return NOT_FOUND_IN_FORM;
	}

	fio_str_info_s s = fiobj_obj2cstr(num);
	int err = mpz_set_str(*number, s.data, 10);
	if (err) {
		printf("Invalid number '%s'\n", s.data);
		return INVALID_NUMBER;
	}

	return 0;
}
