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

static void
mixxx_mappa_test_func(uint32_t target_id, float value, void *token,
		      uint32_t token_size, void *userdata)
{
	ControlProxy *cp = (ControlProxy *)userdata;
	cp->set(value);
}

static void
mixxx_mappa_source_fb_func(float *value, void *token, uint32_t token_size,
			   void *userdata)
{
	ControlProxy *cp = (ControlProxy *)userdata;
	*value = cp->get();
}

QList<Controller*> CtlraEnumerator::queryDevices()
{
	m_mappa = mappa_create(NULL, "Mixxx", "unique");
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
		float max;
		float min;
	} targets[] = {
		{"[Master]", "crossfader", .max = 1, .min = -1},
		{"[Channel1]", "volume"  , .max = 1, .min =  0},
		{"[Channel2]", "volume"  , .max = 1, .min =  0},
		{"[Channel1]", "play"    , .max = 1, .min =  0},
		{"[Channel2]", "play"    , .max = 1, .min =  0},
		{"[Channel1]", "cue"     , .max = 1, .min =  0},
		{"[Channel2]", "cue"     , .max = 1, .min =  0},
		{"[Channel1]", "rate"    , .max = 1, .min =  0},
		{"[Channel2]", "rate"    , .max = 1, .min =  0},
		/* mixer lop/hip filter knobs */
		{"[QuickEffectRack1_[Channel1]]", "super1", .max = 1, .min = 0},
		{"[QuickEffectRack1_[Channel2]]", "super1", .max = 1, .min = 0},
		/* Hotcues */
		{"[Channel1]", "hotcue_1_activate", .max = 1, .min =  0},
		{"[Channel2]", "hotcue_1_activate", .max = 1, .min =  0},
		{"[Channel1]", "hotcue_2_activate", .max = 1, .min =  0},
		{"[Channel2]", "hotcue_2_activate", .max = 1, .min =  0},
		{"[Channel1]", "hotcue_3_activate", .max = 1, .min =  0},
		{"[Channel2]", "hotcue_3_activate", .max = 1, .min =  0},
		{"[Channel1]", "hotcue_4_activate", .max = 1, .min =  0},
		{"[Channel2]", "hotcue_4_activate", .max = 1, .min =  0},
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

		int ret = mappa_target_add(m_mappa, &t, &tid, 0, 0);
		if(ret)
			printf("warning: ctlra target %s %s returns %d\n",
			       targets[i].group, targets[i].item, ret);

		ret = mappa_target_set_range(m_mappa, tid, targets[i].max,
					     targets[i].min);
		if(ret)
			printf("warning: ctlra failed to set range on TID %d\n",
			       tid);
	}

	/* register sources */
{
	struct mappa_source_t s = {0};
	s.func = mixxx_mappa_source_fb_func;
	uint32_t sid;

	struct mixxx_to_mappa_source_t {
		const char *group;
		const char *item;
		float max;
		float min;
	} sources[] = {
		{"[Channel1]", "cue_indicator", .max = 1, .min =  0},
		{"[Channel2]", "cue_indicator", .max = 1, .min =  0},
		{"[Channel1]", "play_indicator", .max = 1, .min =  0},
		{"[Channel2]", "play_indicator", .max = 1, .min =  0},
	};
	const uint32_t sources_size = sizeof(sources) / sizeof(sources[0]);
	for(int i = 0; i < sources_size; i++) {
		char buf[256];
		snprintf(buf, sizeof(buf), "mixxx%s%s", sources[i].group,
			 sources[i].item);
		s.name = buf;
		/* allocate the control proxy instance as userdata to the
		 * func. Allows casting and immidiate usage */
		s.userdata = new ControlProxy(sources[i].group, sources[i].item);

		int ret = mappa_source_add(m_mappa, &s, &sid, 0, 0);
		if(ret)
			printf("warning: ctlra source %s %s returns %d\n",
			       sources[i].group, sources[i].item, ret);

#if 0
		ret = mappa_source_set_range(m_mappa, sid, sources[i].max, sources[i].min);
		if(ret)
			printf("warning: ctlra failed to set range on TID %d\n",
			       tid);
#endif
	}

} /* register sources */


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
