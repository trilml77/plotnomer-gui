/****************************************************************************
**
** Copyright (C) 2019 Lipko <lipko@energopolis.dp.ud>
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "arhivdialog.h"
#include "dtbase.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QFileDialog>
#include <QLocale>
#include <qnetworkproxy.h>

#define MaxSize 16777215


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    dtbs = new dtbase;

    pdSeris = new QLineSeries();
    chrt = new QChart();
    setChart(chrt);
    chartView = new QChartView(chrt);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayout->addWidget(chartView);

    console = new Console;
    console->setEnabled(false);

    ui->verticalLayout->addWidget(console);

    splitter = new QSplitter();
    ui->verticalLayout->addWidget(splitter);
    splitter->addWidget(chartView);
    splitter->addWidget(console);
    splitter->setOrientation(Qt::Vertical);

    console->setHidden(true);



    //setCentralWidget(chartView);

    txt.clear();

    serial = new QSerialPort(this);
    tcpsok = new QTcpSocket(this);
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    status = new QLabel;
    ui->statusBar->addWidget(status);

    initActionsConnections();

    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(console, &Console::getData, this, &MainWindow::writeData);

    tcpsok->setProxy(QNetworkProxy::NoProxy);
    connect(tcpsok, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(tcpsok, SIGNAL(connected()), this, SLOT(onSokConnected()));
    connect(tcpsok, SIGNAL(disconnected()), this, SLOT(onSokDisconnected()));
    connect(tcpsok, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(onSokDisplayError(QAbstractSocket::SocketError)));


    connect(&tmSerial,  SIGNAL(timeout()), this, SLOT(tmSerialRun()));
    serialRead = QDateTime::currentDateTime();
    tmSerial.setInterval(1000);
    tmSerial.start();

    for(int ii=0;ii<3;ii++)
        for(int nn=0;nn<4;nn++)
            ui->tableResult->setItem(ii,nn,new QTableWidgetItem(""));
    ui->tableResult->resizeColumnsToContents();

    arhdialog = new arhivdialog();
    connect(arhdialog,&arhivdialog::tbSelect,this,&MainWindow::historySelect);

    ui->actionTimer->setChecked(settings->settings().timerEnabled);
    timerSet();

    showTrayIcon();
}


MainWindow::~MainWindow()
{
    delete settings;
    delete dtbs;
    delete ui;
}

void MainWindow::actConnect()
{
    SettingsDialog::Settings p = settings->settings();
    if (!p.nportConnection)
      openSerialPort();
    else
      openSocket();
}

void MainWindow::actDisconnect()
{
    SettingsDialog::Settings p = settings->settings();
    if (!p.nportConnection)
      closeSerialPort();
    else
      closeSocket();

    console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);

    showStatusMessage(tr("Disconnected"));
}


void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {

        console->setEnabled(true);
        console->setLocalEchoEnabled(p.localEchoEnabled);

        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);

    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        showStatusMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (serial->isOpen()) serial->close();
}

void MainWindow::openSocket()
{
    if (tcpsok->state() == QAbstractSocket::UnconnectedState)
    {
        SettingsDialog::Settings p = settings->settings();
        tcpsok->connectToHost(p.nportIPAddres,quint16(p.nportPort));
        if (tcpsok->waitForConnected(5000))
        {
            console->setEnabled(true);
            console->setLocalEchoEnabled(p.localEchoEnabled);
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
            ui->actionConfigure->setEnabled(false);
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tcpsok->errorString());
            showStatusMessage(tr("Connect error"));
        }
    }
}

void MainWindow::closeSocket()
{
    if(tcpsok->isOpen()) tcpsok->close();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About this"),
                       tr("Программа для <b>Мониторинга</b> "
                          "пульпы и содержания магнитной части."));
}

void MainWindow::writeData(const QByteArray &data)
{
    if(!prbConnected()) return;

    SettingsDialog::Settings p = settings->settings();
    if(!p.nportConnection)
    {
      serial->write(data);
      serial->flush();
    }
    else
    {
      tcpsok->write(data);
      tcpsok->flush();
    }
}

void MainWindow::readData()
{
    serialRead = QDateTime::currentDateTime();

    QByteArray data;

    SettingsDialog::Settings p = settings->settings();
    if(!p.nportConnection)
      data = serial->readAll();
    else
      data = tcpsok->readAll();

    console->putData(data);

    txt.append(QString(data));
    int nn = -1;
    if(txt.length() > 0)
        do
        {
            while ((txt[0] == char(13)) || (txt[0] == char(10))) txt.remove(0,1);
            nn = txt.indexOf(char(13));
            if(nn > 0){
                QString st = txt.left(nn);
                st.remove("\n").remove("\r");
                QStringList sl = st.split(",");
                SerialZn.clear();
                foreach (QString ln, sl) {
                    int ii = ln.indexOf("=");
                    if(ii>0) SerialZn.insert(ln.left(ii),ln.right(ln.length()-ii-1));
                }

                refreshDisplay(SerialZn);
                txt.remove(0,nn);
            }
        }
    while (txt.length() > 0 && nn > 0);
}

void MainWindow::tmSerialRun()
{
    if(prbConnected())
    {
        qint64 nn = serialRead.secsTo(QDateTime::currentDateTime());
        if(nn > 5) writeData(QByteArray("v1\r"));

        if(ui->actionTimer->isChecked())
        {
            nn = meteringTimer.secsTo(QDateTime::currentDateTime());
            if(nn > settings->settings().timerTime * 60)
            {
                meteringTimer = QDateTime::currentDateTime();
                snWork();
            }
        }
    }
}

void MainWindow::timerSet()
{
    ui->lbTimer->setEnabled(ui->actionTimer->isChecked());
    meteringTimer = QDateTime::currentDateTime();
}

bool MainWindow::prbConnected()
{
    SettingsDialog::Settings p = settings->settings();
    if(!p.nportConnection)
    {
        return serial->isOpen();
    }
    else
    {
        return tcpsok->isOpen();
    }

}

void MainWindow::snWork()
{
    if(prbConnected() && !meteringOn)
    {
        QByteArray ba = QByteArray("mt\r");
        writeData(ba);
        console->putData(ba);
        clearLabel();
    }
}

void MainWindow::snWahing()
{
    if(prbConnected() && !wahingOn)
    {
        QByteArray ba = QByteArray("cl\r");
        writeData(ba);
        console->putData(ba);
        clearLabel();
    }
}

void MainWindow::snAbort()
{
    if(prbConnected())
    {
        QByteArray ba = QByteArray("ab\r");
        writeData(ba);
        console->putData(ba);
        clearLabel();
    }
}

void MainWindow::onSokConnected()
{
    SettingsDialog::Settings p = settings->settings();
    showStatusMessage(tr("Connected to %1 : %2").arg(p.nportIPAddres).arg(p.nportPort));
}

void MainWindow::onSokDisconnected()
{
    SettingsDialog::Settings p = settings->settings();
    showStatusMessage(tr("DisConnected from %1").arg(p.nportIPAddres));
}

void MainWindow::onSokDisplayError(QAbstractSocket::SocketError socketError)
{
    SettingsDialog::Settings p = settings->settings();
    QString hostport = tr("%1 : %2").arg(p.nportIPAddres).arg(p.nportPort);

    switch(socketError)
    {
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::critical(this,"Connection : The host was not found. %s",qUtf8Printable(hostport)); break;
    case QAbstractSocket::RemoteHostClosedError:
        QMessageBox::warning(this,"Connection : The remote host is closed. %s",qUtf8Printable(hostport)); break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::warning(this,"Connection : The connection was refused. %s",qUtf8Printable(hostport)); break;
    case QAbstractSocket::SocketTimeoutError:
        QMessageBox::warning(this,"Connection : The connection timeout. %s",qUtf8Printable(hostport));
        tcpsok->close();
        break;
    default:
        QMessageBox::critical(this,"Connection : %s",qUtf8Printable(QString(tcpsok->errorString())+" : "+hostport)); break;
    }
}


void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}


void MainWindow::initActionsConnections()
{
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);

    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::actConnect);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::actDisconnect);
    connect(ui->actionConfigure, &QAction::triggered, settings, &SettingsDialog::exec);
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::consoleClear);
    connect(ui->actionTerminal, &QAction::triggered, this, &MainWindow::consoleHidden);


    connect(ui->actionWork, &QAction::triggered, this, &MainWindow::snWork);
    connect(ui->actionWahsing, &QAction::triggered, this, &MainWindow::snWahing);
    connect(ui->actionAbort, &QAction::triggered, this, &MainWindow::snAbort);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveTable);
    connect(ui->actionHistory, &QAction::triggered, this, &MainWindow::historyShow);
    connect(ui->actionTimer, &QAction::triggered, this, &MainWindow::timerSet);
}

void MainWindow::consoleClear()
{
    console->clear();
    pdSeris->clear();
}

void MainWindow::consoleHidden()
{
  console->setHidden(!ui->actionTerminal->isChecked());
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

void MainWindow::clearLabel()
{
  ui->lbMetering->setEnabled(false);
  ui->lbWashing->setEnabled(false);
  ui->lbErr->setEnabled(false);
  ui->progressBar->setValue(0);
}

void MainWindow::refreshDisplay(QMap<QString, QString> SerialZn)
{
    if(SerialZn.value("Cntrl") == "On")
    {
        clearLabel();
        writeData(QByteArray("v1\r"));
    }

    if(SerialZn.value("Metr") == "On")
    {
        meteringOn = true;
        ui->lbMetering->setEnabled(true);
    }

    if(SerialZn.value("Metr") == "Off")
    {
        meteringOn = false;
        ui->lbMetering->setEnabled(false);
        dtbs->pushtbArhiv(ui->tableResult,pdSeris);
    }

    if(SerialZn.value("Water") == "On")
    {
        wahingOn = true;
        ui->lbWashing->setEnabled(true);
    }

    if(SerialZn.value("Water") == "Off")
    {
        wahingOn = false;
        ui->lbWashing->setEnabled(false);
    }

    if(meteringOn)
    {
        if(SerialZn.value("View") == "On") iniSeries();
        if(SerialZn.value("gs") != "")
          if(SerialZn.value("gs").toInt() >= 0) appSeries(SerialZn);
    }

    if(SerialZn.value("Err") != "")
    {
        meteringOn = false;
        ui->lbErr->setEnabled(true);
        ui->lbMetering->setEnabled(false);
    }

    QString z_dp = QString(SerialZn.value("dp"));
    QString z_dn = QString(SerialZn.value("dn"));
    QString zi = QString(SerialZn.value("vs"));

    ui->lcdNumber_dp->display(z_dp.toFloat());
    ui->lcdNumber_dn->display(z_dn.toFloat());
    ui->lbSens->setEnabled(zi.toInt() != 0);

    if (SerialZn.value("tm") != "")
    {
        float zz = QString(SerialZn.value("tm")).toFloat() / 240000 * 100;
        ui->progressBar->setValue(trunc(zz));
    }

    if (SerialZn.value("tw") != "")
    {
        float zz = QString(SerialZn.value("tw")).toFloat() / 40000 * 100;
        ui->progressBar->setValue(trunc(zz));
    }

    if (SerialZn.value("mv") != "")
    {

        ui->tableResult->item(0,0)->setText(SerialZn.value("dh1"));
        ui->tableResult->item(0,1)->setText(SerialZn.value("dl1"));
        ui->tableResult->item(0,2)->setText(SerialZn.value("pr1"));

        ui->tableResult->item(1,0)->setText(SerialZn.value("dh2"));
        ui->tableResult->item(1,1)->setText(SerialZn.value("dl2"));
        ui->tableResult->item(1,2)->setText(SerialZn.value("pr2"));

        ui->tableResult->item(2,2)->setText(SerialZn.value("pr3"));
        ui->tableResult->resizeColumnsToContents();
    }
}

void MainWindow::setChart(QChart *ch)
{
    ch->legend()->hide();
    ch->addSeries(pdSeris);
    ch->setTitle("");

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("");
    axisY->setTickCount(8);
    ch->setAxisY(axisY,pdSeris);

    QValueAxis *axisX = new QValueAxis;
    axisX->setLabelFormat("%.1f");
    axisX->setTitleText("");
    axisX->setTickCount(25);
    ch->setAxisX(axisX,pdSeris);

    iniSeries();
}

void MainWindow::iniSeries()
{
    pdSeris->clear();
//    pdSeris->append(0.0,0.0);
    maxPd = 0;
    minPd = 100;
    maxTm = 0;
    chrt->axisY()->setRange(0,maxPd);
    chrt->axisX()->setRange(0,maxTm);
    chrt->setTitle("");

    for(int rw=0;rw < ui->tableResult->rowCount();rw++)
    {
        for(int cl=0;cl < ui->tableResult->columnCount();cl++)
            if(ui->tableResult->item(rw,cl))
                ui->tableResult->item(rw,cl)->setText("");
    }
}


void MainWindow::appSeries(QMap<QString,QString> zn)
{
        float addX = QString(zn.value("tm")).toFloat() / 1000;
        float addY = QString(zn.value("dn")).toFloat();

        if (addX > maxTm && addY > 0) {
          maxTm = addX;

          if (addY > maxPd) maxPd = addY;
          if (addY < minPd) minPd = addY;          
          if (maxPd < minPd) maxPd = minPd;

          chrt->axisY()->setRange(minPd-0.1,maxPd+0.1);
          chrt->axisX()->setRange(0,maxTm);
          pdSeris->append(qreal(addX), qreal(addY));
        }
//      chrt->update();
//      chartView->update();
}

void MainWindow::historyShow()
{
   arhdialog->frRefresh();
   arhdialog->exec();
}

void MainWindow::historySelect(int id)
{
   ChartMinMax mx;
   dtbs->querytbArhSl(id,ui->tableResult,pdSeris,&mx,ui->progressBar);
   chrt->axisY()->setRange(mx.minY,mx.maxY);
   chrt->axisX()->setRange(mx.minX,mx.maxX);
   chrt->setTitle(mx.title);
   chrt->update();
   chartView->update();
}

void SaveThread::run()
{
    QString html  = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html>";
    html +="<head>";
    html +="<meta charset=\"UTF-8\">";
    html +="</head>";
    html +="<body>";

    html += "<table border=\"3\" cellspacing=\"0\" cellpadding=\"0\">";

    for(int ii=0;ii<tbrs->rowCount();ii++)
    {
        html +="<tr>";
        for(int nn=0;nn<tbrs->columnCount();nn++)
            html +="<td>" + tbrs->item(ii,nn)->text().replace('.',',') + "</td>";
        html +="</tr>";
    }

    html +="</table>";


    html += "<table border=\"3\" cellspacing=\"0\" cellpadding=\"0\">";
    html += "<tr>";
    html += "<th align=\"center\" valign=\"center\">X </th>";
    html += "<th align=\"center\" valign=\"center\">Y </th>";
    html += "</tr>";

    for(int ii=0;ii<pdsr->count();ii++)
    {
        html +="<tr>";
        html +="<td>" + QString::number(pdsr->at(ii).x(),'g',3).replace('.',',') + "</td>";
        html +="<td>" + QString::number(pdsr->at(ii).y(),'g',3).replace('.',',') + "</td>";
        html +="</tr>";
    }

    html += "</table></body></html>";

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(html.toUtf8());
    file.close();
}

void MainWindow::saveTable()
{
    QRegExp rs("[^0-9]");
    QString fldt = "plotnomer-" + chrt->title().replace(rs,"") + ".xls";

    QString fileName = QFileDialog::getSaveFileName(this,
                          tr("Save table"), fldt,
                          tr("plotnomer-*.xls (*.xls);;All Files (*)"));
    if (fileName.isEmpty()) return;

    SaveThread *saveThread = new SaveThread;
    connect(saveThread, &SaveThread::finished, saveThread, &QObject::deleteLater);
    saveThread->fileName = fileName;
    saveThread->tbrs = ui->tableResult;
    saveThread->pdsr = pdSeris;
    saveThread->start();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event -> type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            this -> hide();
        }
    }
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if(isHidden()) showNormal(); else hide();
        break;
    default:
        break;
    }
}

void MainWindow::showTrayIcon()
{
    // Создаём экземпляр класса и задаём его свойства...
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage(":/trayico");
    trayIcon -> setIcon(trayImage);

    // Подключаем обработчик клика по иконке...
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    // Выводим значок...
    trayIcon -> show();
}

