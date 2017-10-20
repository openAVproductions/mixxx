#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "control/controlproxy.h"

#include "controllers/ctlra/tccctlracontroller.h"
#include "controllers/ctlra/tccctlracontroller.h"

#include "ctlra.h"
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
	/* Check if we need to recompile script based on modified time of
	 * the script file, comparing with the compiled modified time. If
	 * we need to update, swap the pointer to handle events here */
	time_t new_time;
	int err = file_modify_time(filepath, &new_time);
	if(err) {
		printf("%s: error getting file modified time\n", __func__);
	}
	if(new_time > time_modified) {
		printf("tcc: recompiling %s\n", filepath);
		int ret = compile();
		if(ret) {
			printf("tcc: error recompiling %s\n", filepath);
			dyn_event_func = 0;
		}
	}

	/* Handle event */
	if(dyn_event_func)
		dyn_event_func(dev, num_events, events, instance_ud);
}


void TccCtlraController::feedback_func(struct ctlra_dev_t *dev)
{
	if(dyn_feedback_func)
		dyn_feedback_func(dev, instance_ud);
}


TccCtlraController::TccCtlraController(const struct ctlra_dev_info_t* info) :
	CtlraController(info),
	program(0),
	instance_ud(0),
	time_modified(0),
	dyn_get_vid_pid(0),
	dyn_init_func(0),
	dyn_event_func(0),
	dyn_feedback_func(0)
{
	this->info = (struct ctlra_dev_info_t*)
		malloc(sizeof(struct ctlra_dev_info_t));
	if(!this->info) {
		printf("error allocating tccCtlraController info\n");
		return;
	}
	*this->info = *info;

	/* call function to scan available scripts */
	printf("%s: %s: %s\n", __PRETTY_FUNCTION__, info->vendor, info->device);

	/* match vendor/device against available scripts, select filepath */
	filepath = "ni_d2_script.c";

	if((strcmp(this->info->vendor, "Native Instruments") == 0) &&
	   (strcmp(this->info->device, "Kontrol Z1") == 0)) {
		filepath = "ni_z1_script.c";
	}
	if((strcmp(this->info->vendor, "Native Instruments") == 0) &&
	   (strcmp(this->info->device, "Kontrol S2 Mk2") == 0)) {
		filepath = "ni_s2_script.c";
	}
	if((strcmp(this->info->vendor, "Native Instruments") == 0) &&
	   (strcmp(this->info->device, "Kontrol X1 Mk2") == 0)) {
		filepath = "ni_x1_script.c";
	}
	if((strcmp(this->info->vendor, "3DConnexion") == 0) &&
	   (strcmp(this->info->device, "SpaceMouse Pro") == 0)) {
		filepath = "spacemouse_script.c";
	}

}

TccCtlraController::~TccCtlraController()
{
}

int TccCtlraController::compile()
{
	printf("tcc_script example: compiling %s\n", filepath);

	TCCState *s = tcc_new();
	if(!s) {
		error("failed to create tcc context\n");
		return -ENOMEM;
	}

	tcc_set_error_func(s, NULL, error_func);
	tcc_set_options(s, "-g");
	tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

	int ret = tcc_add_file(s, filepath);
	if(ret < 0) {
		printf("CtlraController: TCC Error, removing script from use.\n");
		tcc_delete(s);
		/* TODO: Show a QT Warning dialog saying script has a
		 * compile error? */
		return -EINVAL;
	}
	/* Add a C linkage function to set a value */
	tcc_add_symbol(s, "mixxx_config_key_set", (void *)mixxx_config_key_set);
	if(ret < 0) {
		error("failed to insert MCK_set() symbol\n");
		return -EINVAL;
	}
	tcc_add_symbol(s, "mixxx_config_key_get", (void *)mixxx_config_key_get);
	if(ret < 0) {
		error("failed to insert MCK_get() symbol\n");
		return -EINVAL;
	}
	tcc_add_symbol(s, "mixxx_config_key_toggle", (void *)mixxx_config_key_toggle);
	if(ret < 0) {
		error("failed to insert MCK_toggle() symbol\n");
		return -EINVAL;
	}
	tcc_add_symbol(s, "ctlra_dev_light_set", (void *)ctlra_dev_light_set);
	if(ret < 0) {
		error("failed to insert ctlra light set() symbol\n");
		return -EINVAL;
	}
	tcc_add_symbol(s, "ctlra_dev_light_flush", (void *)ctlra_dev_light_flush);
	if(ret < 0) {
		error("failed to insert ctlra light set() symbol\n");
		return -EINVAL;
	}

	if(program)
		free(program);

	program = calloc(1, tcc_relocate(s, NULL));
	if(!program)
		error("failed to alloc mem for program\n");
	ret = tcc_relocate(s, program);
	if(ret < 0)
		error("failed to relocate code to program memory\n");

	dyn_get_vid_pid = (script_get_vid_pid)
	                      tcc_get_symbol(s, "script_get_vid_pid");
	if(!dyn_get_vid_pid)
		error("failed to find script_get_vid_pid function\n");

	dyn_event_func = (script_event_func)
	                      tcc_get_symbol(s, "script_event_func");
	if(!dyn_event_func) {
		error("failed to find script_event_func function\n");
		return -EINVAL;
	}

	dyn_init_func = (script_init_func)
	                      tcc_get_symbol(s, "script_init_func");
	if(!dyn_init_func)
		error("failed to find script_init_func function\n");

	dyn_feedback_func = (script_feedback_func)
	                      tcc_get_symbol(s, "script_feedback_func");
	if(!dyn_feedback_func)
		error("failed to find script_feedback_func function\n");

	tcc_delete(s);

	int err = file_modify_time(filepath, &time_modified);
	if(err) {
		printf("%s: error getting file modified time\n", __func__);
		return -ENOENT;
	}

	/* Call the init func of the script, allocs mem for device trackin*/
	if(instance_ud)
		free(instance_ud);

	if(dyn_init_func) {
		int size = dyn_init_func();
		printf("script instance dyn_init_func() size = %d\n", size);
		instance_ud = calloc(1, size);
	} else {
		instance_ud = 0x0;
	}

	return 0;
}
