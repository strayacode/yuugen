#pragma once

#include <QThread>
#include <nds/nds.h>

class EmuThread : public QThread {
Q_OBJECT
public:
	NDS nds;

private:
};