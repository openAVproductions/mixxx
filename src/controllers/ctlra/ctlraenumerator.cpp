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

CtlraReader::CtlraReader(struct mappa_t *c)
	: QThread()
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

QList<Controller*> CtlraEnumerator::queryDevices()
{
	m_mappa = mappa_create(NULL);
	if(m_mappa == 0) {
		printf("Ctlra error creating context!\n");
		goto ret;
	}

	m_reader = new CtlraReader(m_mappa);
	if(m_reader == nullptr) {
		printf("CtlraEnumerator error creating m_reader!\n");
		goto ret;
	}

	// Controller input needs to be prioritized since it can affect the
	// audio directly, like when scratching
	m_reader->start(QThread::HighPriority);

ret:
	return m_devices;
}
