// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include "mongoose/mongoose.h"
#include "cJSON/cJSON.h"

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
static int keep_listening = 1;

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
  return uri->len >= prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static void ev_handler(struct mg_connection *nc, int ev, void *p)
{

	struct http_message *httpm = (struct http_message *)p;
	const struct mg_str api_prefix = MG_MK_STR("/api");

	if (ev == MG_EV_HTTP_REQUEST)
	{
		if(has_prefix(&httpm->uri, &api_prefix)){
			printf("\n -- api call: \n");
			printf(httpm->method.p);
		}
		mg_serve_http(nc, httpm, s_http_server_opts);
	}
}

static void signal_handler(int signal_code)
{
	printf("	--exiting program\n");
	keep_listening = 0;
}

int main(void)
{
	struct mg_mgr mgr;
	struct mg_connection *nc;

	signal(SIGINT, signal_handler);

	mg_mgr_init(&mgr, NULL);
	printf("Starting web server on port %s\n", s_http_port);
	nc = mg_bind(&mgr, s_http_port, ev_handler);
	if (nc == NULL)
	{
		printf("Failed to create listener\n");
		return 1;
	}

	// Set up HTTP server parameters
	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = "./remote_farm_client/build"; // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";

	while (keep_listening)
	{
		mg_mgr_poll(&mgr, 1000);
	}

	mg_mgr_free(&mgr);

	return 0;
}
