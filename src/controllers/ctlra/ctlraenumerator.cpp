/**
* @file mappaenumerator.cpp
* @author Harry van Haaren <harryhaaren@gmail.com>
* @date Sat Jun 30 2018
* @brief This class handles discovery and enumeration of DJ controllers
* supported by the Ctlra library as developed by OpenAV.
*/

#include "controllers/ctlra/ctlraenumerator.h"

#include "control/controlproxy.h"
//#include "controllers/ctlra/ctlracontroller.h"

#include <ctlra/ctlra.h>
#include <ctlra/event.h>
#include <ctlra/mappa.h>

CtlraReader::CtlraReader(struct mappa_t *m)
	: QThread(),
	mappa(m)
{
}

CtlraReader::~CtlraReader()
{
}

void CtlraReader::run()
{
	m_stop = 0;
	while (load_atomic(m_stop) == 0) {
		mappa_iter(mappa);
		usleep(1 * 1000);
	}
}

#if 0
	// here we add the CtlraController instance to the GUI
	CtlraController *c = new CtlraController(a->info);
	m_devices.push_back(c);
#endif

CtlraEnumerator::CtlraEnumerator() : ControllerEnumerator()
{
	qDebug() << "CtlraEnumerator\n";
}

CtlraEnumerator::~CtlraEnumerator()
{
	qDebug() << "Deleting Ctlra devices...";
	while (m_devices.size() > 0) {
		delete m_devices.takeLast();
	}

	mappa_destroy(m_mappa);
}

/* struct that passes metadata trough to the handle function. The
 * token is *per target* - so each can scale values whatever way it
 * wants! Value is always a 0-1 ranged float:
 *  Final value = (value * range) + offset;
 *
 *  Eg: desired range is -1 to 1 (crossfader, pan dials..)
 *      range = 2, offset = -1:
 *  final = (value * 2) - 1;
 */
struct mixxx_scale_value_t {
	float range;
	float offset;
};

static void
mixxx_mappa_test_func(uint32_t target_id, float value, void *token,
		      uint32_t token_size, void *userdata)
{
	ControlProxy *cp = (ControlProxy *)userdata;
	/* TODO: scaling of float value to expected range: eg crossfader */
	if(token_size != sizeof(struct mixxx_scale_value_t)) {
		printf("TOKEN SIZE ERROR! no scale value t available\n");
	}
	struct mixxx_scale_value_t *scale =
		(struct mixxx_scale_value_t *)token;
	cp->set((value * scale->range) + scale->offset);
}

QList<Controller*> CtlraEnumerator::queryDevices()
{
	m_mappa = mappa_create(NULL);
	if(m_mappa == 0) {
		printf("Ctlra error creating context!\n");
		return m_devices;
	}

	struct mappa_target_t t = {0};
	t.func = mixxx_mappa_test_func;
	uint32_t tid;

	struct mixxx_to_mappa_target_t {
		const char *group;
		const char *item;
		struct mixxx_scale_value_t scale;
	} targets[] = {
		{"[Master]", "crossfader", .scale = { .range = 2, .offset = -1}},
		{"[Channel1]", "volume"  , .scale = { .range = 1, .offset =  0}},
		{"[Channel2]", "volume"  , .scale = { .range = 1, .offset =  0}},
		{"[Channel1]", "play"    , .scale = { .range = 1, .offset =  0}},
		{"[Channel2]", "play"    , .scale = { .range = 1, .offset =  0}},
	};
	const uint32_t targets_size = sizeof(targets) / sizeof(targets[0]);

	for(int i = 0; i < targets_size; i++) {
		char buf[256];
		snprintf(buf, sizeof(buf), "mixxx%s%s", targets[i].group,
			 targets[i].item);
		t.name = buf;
		/* allocate the control proxy instance as userdata to the
		 * func. Allows casting and immidiate usage */
		t.userdata = new ControlProxy(targets[i].group, targets[i].item);

		int ret = mappa_target_add(m_mappa, &t, &tid, &targets[i].scale,
					   sizeof(struct mixxx_scale_value_t));
		if(ret)
			printf("warning: ctlra target %s %s returns %d\n",
			       targets[i].group, targets[i].item, ret);

		/*
		mappa_target_set_range(m_mappa, tid,
				       targets[i].scale.range,
		*/
	}


	printf("load bindings\n");
	int ret = mappa_load_bindings(m_mappa, "mixxx_z1.ini");
	if(ret)
		printf("%s %d: load bindings failed, ret %d\n",
		       __func__, __LINE__, ret);

	m_reader = new CtlraReader(m_mappa);
	if(m_reader == nullptr) {
		printf("CtlraEnumerator error creating m_reader!\n");
		return m_devices;
	}

	// Controller input needs to be prioritized since it can affect the
	// audio directly, like when scratching
	m_reader->start(QThread::HighPriority);

	return m_devices;
}
