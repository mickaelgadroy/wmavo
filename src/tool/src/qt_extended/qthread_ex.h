#ifndef QTHREAD_EX_H
#define QTHREAD_EX_H

#include "warning_disable_begin.h"
#include <QThread>
#include "warning_disable_end.h"

class QThread_ex : public QThread
{
protected:
    void run() { exec(); }
};

#endif // QTHREAD_EX_H
