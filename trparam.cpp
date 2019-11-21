#include "trparam.h"

#include <QSettings>

//static trParam trPrm;
trParam trPrm;

static QString paramPath;
static QMapParam param;

trParam::trParam(QObject *parent) : QObject(parent)
{
//
}

void trParam::initialize(QString paramPth)
{

    paramPath = paramPth + "/terminal.conf";
    param.insert("paramPth",paramPth);
    param.insert("paramPath",paramPath);

    comParamRW();
}

void trParam::setParam(QString paramName, QString paramValue)
{
    if(!param.value(paramName).isEmpty())
        param.remove(paramName);

    param.insert(paramName,paramValue);
}

QString trParam::getParam(QString paramName)
{
    return param.value(paramName);
}

void trParam::comParamRW(bool write)
{
    QSettings* Sets = new QSettings(paramPath,QSettings::IniFormat);
    QString pValue;

    Sets->beginGroup("ComPortSettings");

    const QStringList portPr =
        {
            "portName",
            "portBaudRate",
            "portDataBits",
            "portParity",
            "portStopBits",
            "portFlowControl",
            "portLocalEcho",
            "timerTime",
            "timerStart"
        };

    const QStringList portVl =
        {
            "COM1",
            "115200",
            "8",
            "None",
            "1",
            "None",
            "True",
            "10",
            "False"
        };

    for (int ii=0;ii < portPr.count();ii++)
    {
        if (!write)
        {
            pValue = Sets->value(portPr[ii],portVl[ii]).toString();
            param.insert(portPr[ii],pValue);
        }
        else
        {
            Sets->setValue(portPr[ii],param.value(portPr[ii]));
        }
    }

    Sets->endGroup();

    delete Sets;
}
