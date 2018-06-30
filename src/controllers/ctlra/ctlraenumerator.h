/**
* @file ctlraenumerator.h
* @author Harry van Haaren <harryhaaren@gmail.com>
* @date Sat Jun 30 2018
* @brief This class handles discovery and enumeration of DJ controllers
* supported by the Ctlra library as developed by OpenAV.
*/

#ifndef CTLRAENUMERATOR_H
#define CTLRAENUMERATOR_H

#include "controllers/controllerenumerator.h"

struct mappa_t;

/* A reader thread to poll the device, execute actions based on the
 * recieved events, and post them to Mixxx using ControlObjects */
class CtlraReader : public QThread
{
	Q_OBJECT
public:
	CtlraReader(struct mappa_t *mappa);
	virtual ~CtlraReader();
	void stop() {
		m_stop = 1;
	}

protected:
	void run();

private:
	struct mappa_t *mappa;
	QAtomicInt m_stop;
};


class CtlraEnumerator : public ControllerEnumerator {
  public:
    CtlraEnumerator();
    virtual ~CtlraEnumerator();

    QList<Controller*> queryDevices();

  private:
    QList<Controller*> m_devices;

    // This is the main mappa instance for Mixxx. It is contained within
    // the Enumerator class as when devices are hotpluggeed, this class is
    // responsible for adding/removing a CtlraController to/from the list.
    // Hence it makes most sense that the Enumerator owns the Mappa context
    struct mappa_t *m_mappa;

    // The reader for the Ctlra devices
    CtlraReader* m_reader;
};

#endif
