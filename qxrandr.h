#ifndef XRANDR_H
#define XRANDR_H
#include <QDebug>
#include <QObject>
#include <QPoint>
#include <QSize>
#include <QCoreApplication>
#include <QAbstractNativeEventFilter>
#ifndef Bool
    #define Bool int
#endif

#include </usr/include/X11/extensions/Xrandr.h>

class eventNotifier;

/**
 * @brief The //XRandr class
 *
 * XRandr class for Qt \n
 */
class QXRandr : public QObject
{
    Q_OBJECT

public:
    explicit QXRandr(QString strDisplay, RROutput output = 0, QObject *parent = 0);
    ~QXRandr ();

    QList<QSize>        getOutputModes  ();
    bool                isConnected     ();
    bool                isEnabled       ();
    QString             getName         ();
    QList<QSize>        getModes        ();
    QSize               getPreferredMode();
    bool                isPrimary       ();
    void                setPrimary      ();
    Status              enable          (QSize size);
    QSize               getOutputSize   ();
    RROutput            getOutputByName (QString strName);
    QList<RROutput>     getOutputs();

    Status              getXRandrVersion (int *ver, int *rev) const;
    Rotation            getRotate       () const;
    Rotation            getReflect      () const;
    QPoint              getOffset       () const;
    RRMode              getMode         () const;
    Status              disable         ();
    QSize               getScreenSize   () const;
    Status              setScreenSize   (const int &width, const int &height, bool bForce = false);
    int                 setReflect      (Rotation reflection);
    Status              setOffset       (QPoint offset);
    int                 setMode         (QSize size);
    int                 setRotate       (Rotation rotation);
    int                 setPanning      (QSize size);
    void                startEvents     ();
    Status              feedScreen      ();

private:
    int                 screen          () const;
    QList<RRCrtc>       getAllCrtc      ();
    XRRScreenResources *pRes            () const;
    Display            *pDpy            () const;
    Window              root            () const;
    RRMode              getXRRModeInfo  (int width, int height);
    XRROutputInfo      *GetOutputInfo   ();
    QList<RRCrtc>       getCrtcs();
    RRCrtc              getCrtc         ();

private:

    XRRScreenResources  *m_pRes;
    Display             *m_pDpy;
    Window               m_root;
    int                  m_screen;
    RRCrtc               m_crtc;
    RROutput             m_output;
    eventNotifier       *m_pNotifier;

signals:

public slots:
};

#endif // XRANDR_H
