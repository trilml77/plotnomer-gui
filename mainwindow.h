/****************************************************************************
**
** Copyright (C) 2012 Lipko <lipko@energopolis.dp.ua>
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QTcpSocket>
#include <QSystemTrayIcon>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QTimer>
#include <QSplitter>
#include <QDateTime>



QT_BEGIN_NAMESPACE
QT_CHARTS_USE_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;
class arhivdialog;
class dtbase;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void about();

    //определим слоты для обработки сигналов сериал порта
    void actConnect();
    void actDisconnect();

    void handleError(QSerialPort::SerialPortError error);

    //определим слоты для обработки сигналов сокета
    void onSokConnected();
    void onSokDisconnected();
    void onSokDisplayError(QAbstractSocket::SocketError socketError);

    void writeData(const QByteArray &data);
    void readData();

    void tmSerialRun();
    void timerSet();

    void consoleClear();
    void consoleHidden();

    void snWork();
    void snWahing();
    void snAbort();

    void saveTable();
    void historyShow();
    void historySelect(int id);

    void changeEvent(QEvent *event);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void showTrayIcon();


private:
    void initActionsConnections();

    void openSerialPort();
    void closeSerialPort();

    void openSocket();
    void closeSocket();

    void setChart(QChart *ch);
    void iniSeries();
    void appSeries(QMap<QString,QString> zn);
    void showStatusMessage(const QString &message);
    void refreshDisplay(QMap<QString,QString> SerialZn);
    QString createHtml();
    void clearLabel();

private:
    Ui::MainWindow *ui;
    QLabel *status;
    Console *console;
    SettingsDialog *settings;
    arhivdialog *arhdialog;
    QSerialPort *serial;
    QTcpSocket *tcpsok;
    QSplitter *splitter;
    QChart  *chrt;
    QChartView *chartView;
    QLineSeries *pdSeris;
    float maxPd = 0;
    float minPd = 100;
    float maxTm = 0;
    QString txt;

    QTimer tmSerial;
    bool meteringOn = false;
    bool wahingOn = false;
    QMap<QString,QString> SerialZn;
    QDateTime serialRead;
    QDateTime meteringTimer;

    dtbase *dtbs;
    QSystemTrayIcon *trayIcon;

};

#endif // MAINWINDOW_H
