// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include "mongoose/mongoose.h"
#include "cJSON/cJSON.h"
#include <pthread.h>
#include <unistd.h>
#include <math.h>

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
static int keep_listening = 2;

static double sin_value;
static pthread_mutex_t sin_mutex;

// functions
static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix);
static const char *create_value_JSON(float value);
static void ev_handler(struct mg_connection *conn, int ev, void *p);
static void send_sin_value(struct mg_connection *conn);
static void signal_handler(int signal_code);

//threds
void *sin_generateor(void *arg);


int main(void)
{
	//ptrhread initialization
	pthread_t sin_generator_thread;
	
	pthread_mutex_init(&sin_mutex, NULL);
	pthread_create(&sin_generator_thread,NULL, sin_generateor, NULL);

	//mongoose initialization
	struct mg_mgr mgr;
	struct mg_connection *nc;

	signal(SIGINT, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGTERM, signal_handler);

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
	s_http_server_opts.document_root = "./client/build"; // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";

	while (keep_listening)
	{
		mg_mgr_poll(&mgr, 1000);
	}

	pthread_mutex_destroy(&sin_mutex);
	mg_mgr_free(&mgr);

	return 0;
}

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix)
{
	return uri->len >= prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

void *sin_generateor(void *arg)
{
	double time = 0;
	while (keep_listening)
	{
		pthread_mutex_lock(&sin_mutex);
		sin_value = sin(time);
		pthread_mutex_unlock(&sin_mutex);

		time += 0.01;
		time = (time > 2 * M_PI) ? (time - 2 * M_PI) : time;
		usleep(10 * 1000); //10 ns
	}

	return;
}

static const char *create_value_JSON(float value)
{
	char *jsonAsString;
	cJSON *numberValue = NULL;

	cJSON *object = cJSON_CreateObject();
	if (object == NULL)
		return NULL;

	numberValue = cJSON_CreateNumber(value);
	if (numberValue == NULL)
		return NULL;
	cJSON_AddItemToObject(object, "value", numberValue);

	jsonAsString = cJSON_Print(object);
	cJSON_Delete(object);

	return (const char *)jsonAsString;
}

static void ev_handler(struct mg_connection *conn, int ev, void *p)
{

	struct http_message *httpm = (struct http_message *)p;
	const struct mg_str api_prefix = MG_MK_STR("/api");

	if (ev == MG_EV_HTTP_REQUEST)
	{
		if (has_prefix(&httpm->uri, &api_prefix))
		{
			send_sin_value(conn);
		}
		mg_serve_http(conn, httpm, s_http_server_opts);
	}
}

static void send_sin_value(struct mg_connection *conn)
{
	printf("\n -- api call: \n");

	pthread_mutex_lock(&sin_mutex);
	double tsin_value = sin_value;
	pthread_mutex_unlock(&sin_mutex);
	char *v_json = create_value_JSON(tsin_value);

	mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: application/json\r\n\r\n");
	mg_printf_http_chunk(conn, v_json);
	mg_send_http_chunk(conn, "", 0); /* Send empty chunk, the end of response */
}

static void signal_handler(int signal_code)
{
	printf("	--exiting program\n");
	keep_listening = 0;
}