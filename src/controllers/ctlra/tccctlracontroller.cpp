#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "control/controlproxy.h"

#include "controllers/ctlra/tccctlracontroller.h"

void TccCtlraController::event_func(struct ctlra_dev_t* dev,
				     uint32_t num_events,
				     struct ctlra_event_t** events)
{
	/* forward to TCC callback here */
}
