#include <QProcessEnvironment>
#include <QDebug>
#include "qxrandr.h"

int main(int argc, char *argv[])
{

    QString display =  qgetenv("DISPLAY");

    if (!display.isEmpty()){

        //Get all monitors
        QXRandr xrandr (display);
        QList<RROutput>  outputs = xrandr.getOutputs();


        //Show modes and primary
        foreach (RROutput output, outputs){
            QXRandr rroutput (display,output);

            QString strStatus = (rroutput.isConnected()) ? "Connected" : "Not connected";
            QString strPrimary = (rroutput.isPrimary()) ? "Primary" : "";
            qDebug() << rroutput.getName() << strStatus << strPrimary;

            if (strStatus == "Connected"){
                QList <QSize> modes = rroutput.getOutputModes();

                foreach ( QSize item, modes){
                    qDebug() << item.width() << "x" << item.height();
                }

            }
        }
    }
    else
    {
        qDebug() << "No DISPLAY variable environment";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

