#include "fiobject.h"
#include <gmp.h>

enum UTIL_ERRORS { 
	OK,
	NOT_FOUND_IN_FORM,
	INVALID_NUMBER
};

int str_starts_with(FIOBJ str, const char *prefix);
int get_mpz_from_form(FIOBJ form, const char *name, mpz_t *number);
