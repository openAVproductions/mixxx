/**
  * @file ctlracontroller_ni_d2.h
  * @author Harry van Haaren harryhaaren@gmail.com
  * @date Thu Dec 22 2016
  * @brief Ctlra backend header
  */

#ifndef TCCCTLRACONTROLLER_H
#define TCCCTLRACONTROLLER_H

#include "controllers/ctlra/ctlracontroller.h"

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
};

#endif
