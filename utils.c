#include "utils.h"

int str_starts_with(FIOBJ str, const char *prefix) {
	fio_str_info_s s = fiobj_obj2cstr(str);
	return strncmp(s.data, prefix, strlen(prefix)) == 0;
}
