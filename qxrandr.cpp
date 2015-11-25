#include <QDebug>
#include <QAbstractEventDispatcher>
#include "qxrandr.h"
#include "eventnotifier.h"

QXRandr::QXRandr(QString strDisplay, RROutput output, QObject *parent) :
    QObject(parent),
    m_output (output)
{

    m_pDpy      = XOpenDisplay  (strDisplay.toLocal8Bit().data());
//    qDebug() << m_pDpy;

    m_screen    = DefaultScreen (m_pDpy);
    m_root      = RootWindow    (m_pDpy, m_screen);
    m_pRes      = XRRGetScreenResources(m_pDpy, m_root);

    m_crtc = getCrtc();

    m_pNotifier = new eventNotifier (m_pDpy, m_root);
}

QXRandr::~QXRandr()
{
    XSync (m_pDpy,false);
    if (m_pRes)
        XRRFreeScreenResources(m_pRes);

    XCloseDisplay(m_pDpy);

}

XRRScreenResources *QXRandr::pRes() const
{
    return m_pRes;
}

Display *QXRandr::pDpy() const
{
    return m_pDpy;
}

Window QXRandr::root() const
{
    return m_root;
}

int QXRandr::screen() const
{
    return m_screen;
}

Rotation QXRandr::getRotate() const
{
    Rotation rotate=-1;
    if (m_crtc){
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);
        if (pInfo){
            rotate = pInfo->rotation & ~(RR_Reflect_X|RR_Reflect_Y);
            XRRFreeCrtcInfo (pInfo);
        }
    }
    return rotate;
}

Rotation QXRandr::getReflect() const
{
    int reflect=-1;

    if (m_crtc){
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);
        if (pInfo){
            reflect = pInfo->rotation & (RR_Reflect_X|RR_Reflect_Y);
            XRRFreeCrtcInfo (pInfo);
        }
    }
    return reflect;
}

QPoint QXRandr::getOffset() const
{
    QPoint offset;
    if (m_crtc){
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);
        if (pInfo){
            offset = QPoint (pInfo->x, pInfo->y);
            XRRFreeCrtcInfo (pInfo);
        }
    }
    return offset;
}



RRMode QXRandr::getMode() const
{
    RRMode mode=0;
    if (m_crtc){
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);
        if (pInfo){
            mode = pInfo->mode;
            XRRFreeCrtcInfo (pInfo);
        }
    }
    return mode;
}

QSize QXRandr::getScreenSize() const
{
    int screen = DefaultScreen (m_pDpy);
    return QSize (DisplayWidth (m_pDpy, screen), DisplayHeight (m_pDpy, screen));
}


int QXRandr::setScreenSize(const int &width, const int &height, bool bForce /*= false*/)
{
    int mwidth  = DisplayWidth  (m_pDpy,0);
    int mheight = DisplayHeight (m_pDpy,0);

    if (!bForce){
        if (height > mheight) mheight = height;
        if (width  > mwidth)  mwidth  = width;
    }else{
        mwidth  = width;
        mheight = height;
    }

    float dpi = (25.4 * DisplayHeight(m_pDpy, 0)) / DisplayHeightMM(m_pDpy, 0);
    int widthMM =  (int) ((25.4 * mwidth) / dpi);
    int heightMM = (int) ((25.4 * mheight) / dpi);

    XRRSetScreenSize( m_pDpy,
                      m_root,
                      mwidth, mheight,
                      widthMM, heightMM);
    XSync (m_pDpy,false);
    return EXIT_SUCCESS;
}

int QXRandr::setReflect(Rotation reflection)
{
    int ret = -1;
    Rotation reflect;
    if (m_crtc){
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);
        if (pInfo){
            reflection <<= 4;
            reflect = pInfo->rotation & ~(RR_Reflect_X|RR_Reflect_Y);
            reflect |= reflection;
            ret = setRotate (reflect);
             XRRFreeCrtcInfo (pInfo);
        }
    }

    return ret;
}

int QXRandr::setOffset(QPoint offset)
{
    Status ret = -1;
    QSize screenSize;
    QSize crtcSize;

    if (m_crtc){
//        setPanning(QSize(0,0));
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);

        screenSize = this->getScreenSize();
        crtcSize = QSize (pInfo->width+offset.x(),pInfo->height+offset.y());

        if (crtcSize.width() > screenSize.width())
            screenSize.setWidth(crtcSize.width());

        if (crtcSize.height() > screenSize.height())
            screenSize.setHeight(crtcSize.height());

        this->setScreenSize(screenSize.width(),screenSize.height());

        if (pInfo){
            ret = XRRSetCrtcConfig (m_pDpy,
                                    m_pRes,
                                    m_crtc,
                                    CurrentTime,
                                    offset.x(),
                                    offset.y(),
                                    pInfo->mode,
                                    pInfo->rotation,
                                    pInfo->outputs,
                                    pInfo->noutput);

            XSync (m_pDpy,false);
            XRRFreeCrtcInfo(pInfo);
        }
        this->feedScreen();
    }
    return ret;
}

int QXRandr::setMode(QSize size)
{
    int ret = -1;
    if (m_crtc){
        XRRCrtcInfo *crtc_info = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);

        if (crtc_info){
            RRMode rrmode = getXRRModeInfo (size.width(), size.height());
            if (rrmode){

                //disable();
                setScreenSize(size.width(),size.height());

                ret = XRRSetCrtcConfig (m_pDpy,
                                        m_pRes,
                                        m_crtc,
                                        CurrentTime,
                                        0,0,
                                        rrmode,
                                        1,
                                        crtc_info->outputs,
                                        crtc_info->noutput);

                if (ret == RRSetConfigSuccess)
                    setPanning (size);

                XRRFreeCrtcInfo (crtc_info);
            }
            else
                qDebug() << "Modo " << size << "No soportado";
        }
    }
    this->feedScreen();
    XSync (m_pDpy, false);
    return (int)ret;
}

int QXRandr::disable()
{
    Status ret = -1;
    ret = XRRSetCrtcConfig (m_pDpy,
                            m_pRes,
                            m_crtc,
                            CurrentTime,
                            0,0,
                            None,
                            RR_Rotate_0,
                            NULL,
                            0);
    return ret;
}

int QXRandr::setRotate(Rotation rotation)
{
    int ret=-1;
    int val = rotation & 0x07;

    if (val != RR_Rotate_0 && val != RR_Rotate_90 && val != RR_Rotate_180 && val != RR_Rotate_270){
        return ret;
    }

    if (m_crtc){
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,m_crtc);

        if (pInfo){
            int height = pInfo->height;
            int width = pInfo->width;

            if ( (rotation & 0x0f) == RR_Rotate_90 || ( rotation && 0x0f) == RR_Rotate_180){
                if (pInfo->width > pInfo->height){
                    width = pInfo->height;
                    height = pInfo->width;
                }
            }
            else if (pInfo->width < pInfo->height){
                    width = pInfo->height;
                    height = pInfo->width;
                }

            this->setScreenSize(width,height);

            ret = XRRSetCrtcConfig (m_pDpy,
                                    m_pRes,
                                    m_crtc,
                                    CurrentTime,
                                    0,
                                    0,
                                    pInfo->mode,
                                    rotation,
                                    pInfo->outputs,
                                    pInfo->noutput);

            this->setOffset (QPoint(pInfo->x, pInfo->y));
            if ( ret != RRSetConfigSuccess ) {
                qCritical() << "Error setRotate : rotation = " << rotation << "return = " << ret;
            }
            XRRFreeCrtcInfo (pInfo);
        }
    }
    return ret;
}

int QXRandr::setPanning(QSize size)
{
    int ret                 = -1;
    XRRPanning *panning     = NULL;
    panning                 = XRRGetPanning (m_pDpy, m_pRes, m_crtc);
    panning->left           = 0;
    panning->top            = 0;
    panning->track_height   = size.height ();
    panning->track_width    = size.width  ();
    panning->width          = size.width  ();
    panning->height         = size.height ();

    ret = XRRSetPanning ( m_pDpy,
                          m_pRes,
                          m_crtc,
                          panning);

    XRRFreePanning (panning);
    return ret;
}

void QXRandr::startEvents()
{
    m_pNotifier->start();
}

int QXRandr::feedScreen()
{
    QList<RRCrtc >crtcs;
    crtcs = getCrtcs();
    QSize size(0,0);

    for (int crtc=0; crtc<crtcs.count();crtc++){
        RRCrtc rrcrtc = crtcs[crtc];
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,rrcrtc);
        if (pInfo){
            for (int m = 0; m < m_pRes->nmode; m++) {
                if (pInfo->mode == m_pRes->modes[m].id){
                    XRRModeInfo *mode = &m_pRes->modes[m];

                    unsigned int height = mode->height + pInfo->y;
                    unsigned int width = mode->width + pInfo->x;

                    if (height > (unsigned int)size.height())
                        size.setHeight(size.height()+(height-size.height()));

                    if (width  > (unsigned int)size.width())
                        size.setWidth(size.width()+(width - size.width()));
                }
            }
        }
    }
    this->setScreenSize(size.width(),size.height(),true);
    qDebug() << size;

    return 0;
}

RRMode QXRandr::getXRRModeInfo(int width, int height)
{
    RRMode rrmode= 0;
    if (m_output) {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo) {
            for (int j = 0; j < pOutputInfo->nmode; j++) {
                for (int m = 0; m < m_pRes->nmode; m++) {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id){
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        if (mode->width == (unsigned int)width && mode->height == (unsigned int)height){
                            rrmode = pOutputInfo->modes[j];
                            XRRFreeOutputInfo (pOutputInfo);
                            goto end;
                        }
                    }
                }
            }
            XRRFreeOutputInfo (pOutputInfo);
        }
    }
end:
    return rrmode;
}

XRROutputInfo *QXRandr::GetOutputInfo()
{
    return XRRGetOutputInfo(m_pDpy,m_pRes,m_output);
}

QList<QSize> QXRandr::getOutputModes()
{
    QList<QSize >modes;

    if (m_output) {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo) {
            for (int j = 0; j < pOutputInfo->nmode; j++) {
                for (int m = 0; m < m_pRes->nmode; m++) {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id){
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        modes.append (QSize (mode->width,mode->height));
                    }
                }
            }
        }
    }
    return modes;
}

QSize QXRandr::getOutputSize()
{
    QSize size (-1,-1);
    RRMode mode;

    if (m_output) {
        XRROutputInfo *pOutputInfo = GetOutputInfo();

        if (pOutputInfo) {
            if (pOutputInfo->crtc) {
                XRRCrtcInfo *pCrtcInfo = XRRGetCrtcInfo (m_pDpy, m_pRes, pOutputInfo->crtc);
                if (pCrtcInfo) {
                    mode = pCrtcInfo->mode;

                    for (int m = 0; m < m_pRes->nmode; m++) {
                        if (mode == m_pRes->modes[m].id){
                            XRRModeInfo *mode = &m_pRes->modes[m];
                            size = QSize(mode->width,mode->height);
                            break;
                        }
                    }
                }
            }
        }
        if (pOutputInfo)
            XRRFreeOutputInfo (pOutputInfo);
    }
    return size;
}

bool QXRandr::isEnabled()
{
    bool bEnabled = false;
    QSize size = this->getScreenSize ();
    bEnabled = (size.width()>0 && size.width()>0);
    return bEnabled;
}

QString QXRandr::getName()
{
    QString strName;
    if (m_output){
        XRROutputInfo *pOutputInfo = GetOutputInfo ();
        if (pOutputInfo){
            strName = QString (pOutputInfo->name);
            XRRFreeOutputInfo (pOutputInfo);
        }
    }
    return strName;
}

RRCrtc QXRandr::getCrtc()
{
    RRCrtc rrcrtc = 0;
    if (m_output){
        XRROutputInfo *pOutputInfo = GetOutputInfo ();
        if (pOutputInfo->crtc){
            rrcrtc = pOutputInfo->crtc;
            XRRFreeOutputInfo (pOutputInfo);
        }
    }
    return rrcrtc;
}

QList<RRCrtc> QXRandr::getAllCrtc()
{
    QList <RRCrtc >crtcs;
    for (int ncrtc=0;ncrtc<m_pRes->ncrtc;ncrtc++){
        crtcs.append (m_pRes->crtcs[ncrtc]);
    }
    return crtcs;
}

QList<QSize> QXRandr::getModes()
{
    QList<QSize >modes;
    if (m_output) {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo) {
            for (int j = 0; j < pOutputInfo->nmode; j++) {
                for (int m = 0; m < m_pRes->nmode; m++) {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id){
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        modes.append (QSize (mode->width,mode->height));
                    }
                }
            }
        }
    }
    return modes;
}

QSize QXRandr::getPreferredMode()
{
    QList<QSize >modes;
    if (m_output) {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo) {
            for (int j = 0; j < pOutputInfo->nmode; j++) {
                for (int m = 0; m < m_pRes->nmode; m++) {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id){
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        modes.append (QSize (mode->width,mode->height));
                    }
                }
            }
        }
        //Atencion!
        //Esto se ha de probar en monitores con diferentes resoluciones nominales.
        return modes.at(pOutputInfo->npreferred-1);
    }

    return QSize (0,0);
}


bool QXRandr::isPrimary()
{
    RROutput rroutput = XRRGetOutputPrimary(m_pDpy,m_root);
    return ( rroutput == m_output);
}

void QXRandr::setPrimary()
{
    if (!isPrimary())
        XRRSetOutputPrimary (m_pDpy,m_root,m_output);
}

Status QXRandr::enable(QSize size)
{
    Status ret=-1;
    if (m_output){

        RRMode rrmode = getXRRModeInfo(size.width(),size.height());
        if (!rrmode)
            return ret;

        QList<RRCrtc >crtcs;
        crtcs = getCrtcs();

        for (int crtc=0; crtc<crtcs.count();crtc++){
            RRCrtc rrcrtc = crtcs[crtc];
            XRRCrtcInfo *pInfo = XRRGetCrtcInfo (m_pDpy,m_pRes,rrcrtc);

            for (int poss = 0; poss<pInfo->npossible;poss++){
                RROutput rr_output = pInfo->possible[poss];

                if (pInfo->noutput == 0 && pInfo->possible[poss] == m_output){

                    //XRROutputInfo *pOutInfo = XRRGetOutputInfo(m_pDpy,m_pRes,rr_output);
                    //qDebug() << QString(pOutInfo->name);

                    RROutput *rr_outputs;
                    rr_outputs = (RROutput *)calloc (1, sizeof (RROutput));
                    rr_outputs[0] = rr_output;

                    setScreenSize(size.width(),size.height());

                    ret = XRRSetCrtcConfig (  m_pDpy,
                                              m_pRes,
                                              rrcrtc,
                                              CurrentTime,
                                              pInfo->x,
                                              pInfo->y,
                                              rrmode,
                                              RR_Rotate_0,
                                              rr_outputs,
                                              1);
                    return ret;
                }
            }
        }
    }
    return ret;

}

bool QXRandr::isConnected()
{
    bool bConnected= false;
    if (m_output){
        XRROutputInfo *pOutputInfo = GetOutputInfo ();
        if (pOutputInfo){
            bConnected = (pOutputInfo->connection == 0) ? true : false;
            XRRFreeOutputInfo (pOutputInfo);
        }
    }
    return bConnected;
}

QList<RROutput> QXRandr::getOutputs()
{
    QList <RROutput > outputs;
    for (int nout=0;nout<m_pRes->noutput;nout++){
        outputs.append (m_pRes->outputs[nout]);
    }
    return outputs;
}

QList<RRCrtc > QXRandr::getCrtcs()
{
    QList <RRCrtc >crtcs;
    for (int ncrtc=0;ncrtc<m_pRes->ncrtc;ncrtc++){
        crtcs.append (m_pRes->crtcs[ncrtc]);
    }
    return crtcs;
}

RROutput QXRandr::getOutputByName(QString strName)
{
    RROutput output = 0;

    for (int nout=0;nout<m_pRes->noutput;nout++){
        XRROutputInfo *pInfo = XRRGetOutputInfo (m_pDpy, m_pRes, m_pRes->outputs[nout]);
        if ( strName == pInfo->name){
            output = m_pRes->outputs[nout];
            XRRFreeOutputInfo (pInfo);
            break;
        }
        XRRFreeOutputInfo (pInfo);
    }
    return output;
}



