#include "arhivdialog.h"
#include "ui_arhivdialog.h"
#include "dtbase.h"
#include <QDate>
#include <QPushButton>
#include <QTableWidget>

arhivdialog::arhivdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::arhivdialog)
{
    ui->setupUi(this);

    dtbs = new dtbase();
    ui->slDate->setDate(QDate::currentDate());
    ui->slPeriod->setCurrentIndex(0);

    connect(ui->btExit,&QPushButton::clicked,this, &arhivdialog::close);
    connect(ui->btRefresh,&QPushButton::clicked,this,&arhivdialog::frRefresh);
    connect(ui->tbArhiv,&QTableWidget::doubleClicked,this,&arhivdialog::selectRow);
    connect(ui->btChart,&QPushButton::clicked,this,&arhivdialog::selectRow);
    connect(ui->btDel,&QPushButton::clicked,this,&arhivdialog::delMetering);


    frRefresh();
}

arhivdialog::~arhivdialog()
{
    delete dtbs;
    delete ui;
}

#define columnID 8 //column id = 8
int arhivdialog::getcurID()
{
    int rw = ui->tbArhiv->currentRow();
    return ui->tbArhiv->item(rw,columnID)->text().toInt();
}

void arhivdialog::frRefresh()
{
    dtbs->querytbArhiv(ui->slDate->date(),ui->slPeriod->currentIndex(),ui->tbArhiv);
}

void arhivdialog::selectRow()
{
  close();
  emit tbSelect(getcurID());
}

void arhivdialog::delMetering()
{
    if(ui->tbArhiv->currentRow() >= 0)
    {
        dtbs->deltbArhiv(getcurID());
        frRefresh();
    }
}

