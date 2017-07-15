#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "control/controlproxy.h"

#include "controllers/ctlra/tccctlracontroller.h"
#include "controllers/ctlra/tccctlracontroller.h"

#include "libtcc.h"

static void error_func(void *userdata, const char *msg)
{
	printf("CtlraController: TCC says: %s\n", msg);
}

static void error(const char *msg)
{
	printf("%s\n", msg);
}

static int file_modify_time(const char *path, time_t *new_time)
{
	if(new_time == 0)
		return -1;
	struct stat file_stat;
	int err = stat(path, &file_stat);
	if (err != 0) {
		return -2;
	}
	*new_time = file_stat.st_mtime;
	return 0;
}

void mixxx_config_key_set(const char *group, const char *name, float v)
{
	ControlProxy cp(group, name);
	cp.set(v);
}

void mixxx_config_key_toggle(const char *group, const char *name)
{
	ControlProxy cp(group, name);
	cp.set( ! cp.get() );
}

float mixxx_config_key_get(const char *group, const char *name)
{
	ControlProxy cp(group, name);
	return cp.get();
}

static void tcc_event_proxy(struct ctlra_dev_t* dev, uint32_t num_events,
			    struct ctlra_event_t** events, void *userdata)
{
	TccCtlraController *t = (TccCtlraController *)userdata;
	t->event_func(dev, num_events, events);
}

void TccCtlraController::event_func(struct ctlra_dev_t* dev,
				     uint32_t num_events,
				     struct ctlra_event_t** events)
{
#if 0
	/* Check if we need to recompile script based on modified time of
	 * the script file, comparing with the compiled modified time. If
	 * we need to update, swap the pointer to handle events here */
	time_t new_time;
	int err = file_modify_time(filepath,
				   &new_time);
	if(err) {
		printf("%s: error getting file modified time\n", __func__);
	}
	if(new_time > time_modified) {
		printf("tcc: recompiling %s\n", filepath);
		int ret = compile(script);
		if(ret)
			printf("tcc: error recompiling %s\n", filepath);
	}

	/* Handle events */
	if(tcc_event_func)
		tcc_event_func(dev, num_events, events, instance_ud);
#endif
}

TccCtlraController::TccCtlraController(const struct ctlra_dev_info_t* info) :
	CtlraController(info)
{
	/* initialize the TCC context here */
}


TccCtlraController::~TccCtlraController()
{
}
