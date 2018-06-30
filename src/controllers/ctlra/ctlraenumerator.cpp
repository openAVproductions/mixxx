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
	printf("%s\n", __func__);
	float v = (value * 2) - 1;
	ControlProxy("[Master]", "crossfader").set(v);
}

QList<Controller*> CtlraEnumerator::queryDevices()
{
	m_mappa = mappa_create(NULL);
	if(m_mappa == 0) {
		printf("Ctlra error creating context!\n");
		return m_devices;
	}

	struct mappa_target_t t = {0};
	t.name = "mixxx_crossfader";
	t.func = mixxx_mappa_test_func;
	t.userdata = this;
	uint32_t tid;
	int ret = mappa_target_add(m_mappa, &t, &tid, 0, 0);
	qDebug() << "CTLRA MIXXX TARGET REG returns " << ret;

	printf("load bindings\n");
	ret = mappa_load_bindings(m_mappa, "mixxx_z1.ini");
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
