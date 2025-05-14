#include "http.h"

typedef struct Application {
	FIOBJ routing_table;
} Application;

typedef void HTTP_Handler(http_s *);

#define GET_APPLICATION(req) (Application *) req->udata

void init_app(Application *app);
void free_app(Application *app);

void route_request(Application *app, http_s *request);

void home_handler(http_s *request);
void on_file_request(http_s *request);
// void on_error(http_s *request);
