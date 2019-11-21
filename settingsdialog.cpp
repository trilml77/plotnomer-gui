/****************************************************************************
**
** Copyright (C) 2012 Lipko <lipko@energopolis.dp.ua>
**
****************************************************************************/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>

#include "trparam.h"

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    intValidator = new QIntValidator(0, 4000000, this);

    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->applyButton, &QPushButton::clicked,
            this, &SettingsDialog::apply);
    connect(ui->serialPortInfoListBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::showPortInfo);
    connect(ui->baudRateBox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomBaudRatePolicy);
    connect(ui->serialPortInfoListBox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomDevicePathPolicy);

    fillPortsParameters();
    fillPortsInfo();

    updateSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

SettingsDialog::Settings SettingsDialog::settings() const
{
    return currentSettings;
}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SettingsDialog::apply()
{
    updateSettings();
    hide();
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
}

void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

void SettingsDialog::mShow()
{

}

void SettingsDialog::fillPortsParameters()
{
    int nn = 0;
    ui->baudRateBox->addItem("9600", QSerialPort::Baud9600);
    ui->baudRateBox->addItem("19200", QSerialPort::Baud19200);
    ui->baudRateBox->addItem("38400", QSerialPort::Baud38400);
    ui->baudRateBox->addItem("115200", QSerialPort::Baud115200);
//    ui->baudRateBox->addItem(tr("Custom"));
    nn = ui->baudRateBox->findText(trPrm.getParam("portBaudRate"),Qt::MatchCaseSensitive);
    ui->baudRateBox->setCurrentIndex(nn);

    ui->dataBitsBox->addItem("5", QSerialPort::Data5);
    ui->dataBitsBox->addItem("6", QSerialPort::Data6);
    ui->dataBitsBox->addItem("7", QSerialPort::Data7);
    ui->dataBitsBox->addItem("8", QSerialPort::Data8);
    nn = ui->dataBitsBox->findText(trPrm.getParam("portDataBits"),Qt::MatchCaseSensitive);
    ui->dataBitsBox->setCurrentIndex(nn);

    ui->parityBox->addItem("None", QSerialPort::NoParity);
    ui->parityBox->addItem("Even", QSerialPort::EvenParity);
    ui->parityBox->addItem("Odd", QSerialPort::OddParity);
    ui->parityBox->addItem("Mark", QSerialPort::MarkParity);
    ui->parityBox->addItem("Space", QSerialPort::SpaceParity);
    nn = ui->parityBox->findText(trPrm.getParam("portParity"),Qt::MatchCaseSensitive);
    ui->parityBox->setCurrentIndex(nn);

    ui->stopBitsBox->addItem("1", QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem("1.5", QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem("2", QSerialPort::TwoStop);
    nn = ui->stopBitsBox->findText(trPrm.getParam("portStopBits"),Qt::MatchCaseSensitive);
    ui->stopBitsBox->setCurrentIndex(nn);

    ui->flowControlBox->addItem("None", QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem("RTS/CTS", QSerialPort::HardwareControl);
    ui->flowControlBox->addItem("XON/XOFF", QSerialPort::SoftwareControl);
    nn = ui->flowControlBox->findText(trPrm.getParam("portFlowControl"),Qt::MatchCaseSensitive);
    ui->flowControlBox->setCurrentIndex(nn);

    bool chk = trPrm.getParam("portLocalEcho") == "True";
    ui->localEchoCheckBox->setChecked(chk);

    ui->spTimer->setValue(trPrm.getParam("timerTime").toInt());

    bool cht = trPrm.getParam("timerStart") == "True";
    ui->chTimer->setChecked(cht);

    bool chNc = trPrm.getParam("nportConnection") == "True";
    ui->nportCheckBox->setChecked(chNc);

    ui->nportIPAddres->setText(trPrm.getParam("nportIPAddres"));
    ui->nportPort->setValue(trPrm.getParam("nportPort").toInt());

}

void SettingsDialog::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    int nn = ui->serialPortInfoListBox->findText(trPrm.getParam("portName"),Qt::MatchCaseSensitive);
    if (nn < 0 ) nn = 0;
    ui->serialPortInfoListBox->setCurrentIndex(nn);

//    ui->serialPortInfoListBox->addItem(tr("Custom"));
}

void SettingsDialog::updateSettings()
{
    currentSettings.name = ui->serialPortInfoListBox->currentText();

    if (ui->baudRateBox->currentIndex() == 4) {
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    } else {
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

    currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();
    currentSettings.timerEnabled = ui->chTimer->isChecked();
    currentSettings.timerTime = ui->spTimer->value();

    currentSettings.nportConnection = ui->nportCheckBox->isChecked();
    currentSettings.nportIPAddres = ui->nportIPAddres->text();
    currentSettings.nportPort = ui->nportPort->value();

    trPrm.setParam("portName",ui->serialPortInfoListBox->currentText());
    trPrm.setParam("portBaudRate",ui->baudRateBox->currentText());
    trPrm.setParam("portDataBits",ui->dataBitsBox->currentText());
    trPrm.setParam("portParity",ui->parityBox->currentText());
    trPrm.setParam("portStopBits",ui->stopBitsBox->currentText());
    trPrm.setParam("portFlowControl",ui->flowControlBox->currentText());

    trPrm.setParam("portLocalEcho",ui->localEchoCheckBox->isChecked() ? "True" : "False");
    trPrm.setParam("timerTime",QString::number(ui->spTimer->value()));
    trPrm.setParam("timerStart",ui->chTimer->isChecked() ? "True" : "False");

    trPrm.setParam("nportConnection",ui->nportCheckBox->isChecked() ? "True" : "False");
    trPrm.setParam("nportIPAddres",ui->nportIPAddres->text());
    trPrm.setParam("nportPort",QString::number(ui->nportPort->value()));

    trPrm.comParamRW(true);
}
