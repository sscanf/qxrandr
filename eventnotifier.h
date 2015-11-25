#ifndef EVENTNOTIFIER_H
#define EVENTNOTIFIER_H
#include <QThread>
#include <X11/Xlib.h>
#include </usr/include/X11/extensions/Xrandr.h>


class eventNotifier : public QThread
{
public:
    eventNotifier(Display *pDpy, int screen);
    void run();

private:
    void prologue       (XEvent *eventp, const QString &event_name);
    void do_RRNotify    (XEvent *eventp);
private:

    Display *m_pDpy;
    int      m_screen;
    bool     m_bStop;
};

#endif // EVENTNOTIFIER_H
