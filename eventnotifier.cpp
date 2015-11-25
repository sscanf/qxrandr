#include <QDebug>
#include "eventnotifier.h"

eventNotifier::eventNotifier(Display *pDpy, int screen)
{
    m_bStop=false;
    m_pDpy = pDpy;
    m_screen = screen;
}

void eventNotifier::prologue(XEvent *eventp, const QString &event_name)
{
    XAnyEvent *e = (XAnyEvent *) eventp;
    qDebug ("%s event, serial %ld, synthetic %s, window 0x%lx,\n",
        event_name.toLocal8Bit().data(), e->serial, e->send_event ? "Yes" : "No", e->window);
}

void eventNotifier::do_RRNotify (XEvent *eventp)
{
    XRRNotifyEvent *e = (XRRNotifyEvent *) eventp;
    XRRScreenResources *screen_resources;

    XRRUpdateConfiguration (eventp);
    screen_resources = XRRGetScreenResources (m_pDpy, m_screen);
    prologue (eventp, "RRNotify");
    switch (e->subtype) {
      case RRNotify_OutputChange:
        qDebug() << "OutputChange";
//          do_RRNotify_OutputChange (eventp, screen_resources); break;
      break;

      case RRNotify_CrtcChange:
        qDebug() << "CrtcChange";
//         do_RRNotify_CrtcChange (eventp, screen_resources); break;
      break;

      case RRNotify_OutputProperty:
        qDebug() << "OutputProperty";
//          do_RRNotify_OutputProperty (eventp, screen_resources); break;
      break;

      default:
          printf ("    subtype %d\n", e->subtype);
    }
    XRRFreeScreenResources (screen_resources);
}

void eventNotifier::run()
{
    int rr_event_base;
    int rr_error_base;

    XRRQueryExtension (m_pDpy, &rr_event_base, &rr_error_base);
    XRRSelectInput( m_pDpy, m_screen,   RRCrtcChangeNotifyMask   |
                                        RROutputChangeNotifyMask |
                                        RROutputPropertyNotifyMask );
    while (!m_bStop){
        XEvent event;
        XNextEvent (m_pDpy, &event);

        if (event.type == rr_event_base + RRNotify){
            prologue (&event, "RRScreenChangeNotify");
            do_RRNotify(&event);
        }
    }
}
