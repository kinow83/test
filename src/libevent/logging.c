/*
 * logging.c
 *
 *  Created on: 2017. 9. 10.
 *      Author: root
 */

#include <event2/event.h>
#include <stdio.h>


/**
 * event_set_log_callback
 */

static void discard_db(int severity, const char *msg)
{
	/* This callback does nothing */
}

static FILE *__logfile = NULL;
static void write_to_file_cb(int sevrity, const char *msg)
{
	const char *s;
	if (!__logfile) {
		return;
	}

	switch (sevrity) {
	case EVENT_LOG_DEBUG:
		s = "debug";
		break;
	case EVENT_LOG_MSG:
		s = "msg";
		break;
	case EVENT_LOG_WARN:
		s = "warn";
		break;
	case EVENT_LOG_ERR:
		s = "error";
		break;
	default:
		s = "?";
	}
	fprintf(__logfile, "[%s] %s\n", s, msg);
}

void suppress_logging(void)
{
	event_set_log_callback(discard_db);
}

void set_file_logging(FILE *f)
{
	__logfile = f;
	event_set_log_callback(write_to_file_cb);
}

evthread_set_lock_callbacks

int main()
{
	FILE *fp;
	struct event_base *base;

	fp = fopen("./test.log", "w+");
	set_file_logging(fp);
	event_enable_debug_logging(EVENT_DBG_ALL);

	base = event_base_new();
	event_base_priority_init(base, 2);


	event_base_free(base);
}
