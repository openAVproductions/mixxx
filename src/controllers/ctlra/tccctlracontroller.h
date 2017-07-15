/**
  * @file ctlracontroller_ni_d2.h
  * @author Harry van Haaren harryhaaren@gmail.com
  * @date Thu Dec 22 2016
  * @brief Ctlra backend header
  */

#ifndef TCCCTLRACONTROLLER_H
#define TCCCTLRACONTROLLER_H

#include "controllers/ctlra/ctlracontroller.h"

#include "tcc_mixxx_api.h"

class TccCtlraController : public CtlraController
{
	Q_OBJECT
public:
	TccCtlraController(const struct ctlra_dev_info_t* info);
	virtual ~TccCtlraController();

	/* overridden to forward events to TCC instance of handle func */
	virtual void event_func(struct ctlra_dev_t* dev,
				uint32_t num_events,
				struct ctlra_event_t** events) override;


	/* overridden to provide feedback events to the device */
	virtual void feedback_func(struct ctlra_dev_t *dev) override;

private:
	/* A pointer to memory malloced for the generated code */
	void *program;
	/* pointer to the script-allocated data */
	void *instance_ud;
	/* The path of the script file */
	char *filepath;
	/* Time the script file was last modified */
	time_t time_modified;

	/* calling this will cause filepath to be recompiled, and the
	 * pointers dyn_*_func to be updated */
	int compile();

	/* Function pointer to get the USB device this script supports */
	script_get_vid_pid dyn_get_vid_pid;
	/* Function that will initialize the instance memory */
	script_init_func dyn_init_func;
	/* Function pointer to the scripts event handling function */
	script_event_func dyn_event_func;
	/* Function pointer to the scripts feedback handling function */
	script_feedback_func dyn_feedback_func;
};

#endif
