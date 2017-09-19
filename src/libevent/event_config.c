/*
 * evnet_config.c
 *
 *  Created on: 2017. 9. 10.
 *      Author: root
 */

#include <event2/event.h>
#include <stdio.h>


int main()
{
	struct event_config *cfg;
	struct event_base *base;
	int rc;
	int i;
	const char **plist;

	printf("support libevent base method\n");
	plist = event_get_supported_methods();
	while (plist && *plist) {
		printf("[%d] %s\n", ++i, *plist);
		plist++;
	}

	printf("=====================\n");

	for (i=0; i<2; i++) {
		cfg = event_config_new();

		// don't like "select"
//		event_config_avoid_method(cfg, "epoll");
		event_config_avoid_method(cfg, "poll");
		event_config_avoid_method(cfg, "select");

		if (i == 0) {
			// like Edge Trigger Socket buffer framework
			event_config_require_features(cfg, EV_FEATURE_ET);
		}

		base = event_base_new_with_config(cfg);
		event_config_free(cfg);

		if(base) {
			break;
		}
	}
	if (base) {
		printf("suitable method is [%s]\n", event_base_get_method(base));
	} else {
		printf("not exist suitable method\n");
	}

	event_base_free(base);
}
