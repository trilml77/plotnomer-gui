/****************************************************************************
**
** Copyright (C) 2012 Lipko <lipko@energopolis.dp.ua>
**
****************************************************************************/

#include <QApplication>
#include <QTextStream>
#include <QStyleFactory>

#include "mainwindow.h"
#include "trparam.h"

static QString logPath = "";
static const int logLevel = 1;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    app.setOrganizationName("enplterm");
//    app.setApplicationName("enplterm");
    app.setDesktopSettingsAware(false);
    app.setStyle(QStyleFactory::create("Fusion"));

    trPrm.initialize(app.applicationDirPath());
    logPath = app.applicationDirPath();
    // Registry message log
    qInstallMessageHandler(myMessageOutput);

    MainWindow w;
    w.show();
    return app.exec();
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString stype = "Error";
    int logLv = 0;

    switch (type) {
    case QtDebugMsg:
        stype = "Debug"; logLv = 4;
        break;
    case QtInfoMsg:
        stype = "Info"; logLv = 3;
        break;
    case QtWarningMsg:
        stype = "Warning"; logLv = 2;
        break;
    case QtCriticalMsg:
        stype = "Critical"; logLv = 1;
        break;
    case QtFatalMsg:
        stype = "Fatal"; logLv = 0;
        break;
    }

    if (logLv > logLevel) return;

    QString FlName = logPath + "/terminal-" + QDateTime::currentDateTime().toString("yyyy-MM") + ".log";
    QFile logFile(FlName);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) return;

//    QTextStream qout(stdout);
    QTextStream out(&logFile);

    QByteArray localMsg = msg.toLocal8Bit();

    QString msg;
    if (logLevel == 4)
    {
        msg = QString(" : %1 (%2)\n").arg(
                    QString(localMsg.constData()),
                    //                    QString(context.file),
                    //                    QString::number(context.line),
                    QString(context.function));
    }
    else msg =  QString(" : %1\n").arg(localMsg.constData());

    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
    out << stype << msg;
    out.flush();

//    qout << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
//    qout << stype << msg;
//    qout.flush();

    logFile.close();

    if (type == QtFatalMsg) abort();
}
