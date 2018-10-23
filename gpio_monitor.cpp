

#include "gpio_monitor.h"
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <QDebug>
#include <unistd.h>

gpio_monitor::gpio_monitor(QObject *parent)
                : QThread(parent)
{
    stopped=false;
}

gpio_monitor::~gpio_monitor()
{
    stopped=true;
}

bool gpio_monitor::add(gpio *gpi)
{
    if ((gpi->getDirection() == GPIO_INPUT))
        gpioList.append(gpi);

    for (int i = 0; i < gpioList.size(); i++) {
        qDebug() << i << ": " << gpioList[i]->getName();
    }
    return true;
}

void gpio_monitor::run()
{
    struct pollfd *fds;
    char val;
    int ret;
    int retVal;

    fds = (pollfd*) malloc(sizeof(pollfd) * gpioList.size());

    for (int i = 0; i < gpioList.size(); i++) {
        fds[i].fd = gpioList[i]->getHandle();
        fds[i].events = POLLPRI | POLLERR;
    }

    stopped = false;
    while (!stopped) {
        ret = ::poll(fds, gpioList.size(), 2000);

        if (ret > 0) {
            for (int i=0; i<gpioList.size(); i++) {
                if ((fds[i].revents & POLLPRI) || (fds[i].revents & POLLERR)) {
                    ::lseek(fds[i].fd,0, SEEK_SET);
                    retVal = ::read(fds[i].fd, &val, 1);
                    if (val == '1') emit RaisingEdge(gpioList[i]->getName(), i); else emit FallingEdge(gpioList[i]->getName(),i);
                    if (retVal == -1)
                        qDebug() << "read failed";
                }
            }
        } else if (ret < 0) qDebug() << "poll errno : " << errno;
    }
    free(fds);
}

void gpio_monitor::stop()
{
    stopped=true;
}
