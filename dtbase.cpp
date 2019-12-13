#include "dtbase.h"
#include "trparam.h"

static QSqlDatabase db;
static int dtbase_count = 0;

dtbase::dtbase(QObject *parent) : QObject(parent)
{
    dtbase_count++;
}

dtbase::~dtbase()
{
    dtbase_count--;
    if (dtbase_count <= 0)
        databaseDisConnect(&db);
}

bool dtbase::databaseConnect(QSqlDatabase *db)
{
    bool ok = db->isOpen();
    if (!ok)
    {
        if(!db->isValid()){
            QString databaseName = "termBase";
            QString pth = trPrm.getParam("paramPth")+"/terminal.db";
            *db = QSqlDatabase::addDatabase("QSQLITE",databaseName);
            db->setDatabaseName(pth);
        }

        ok = db->open();
        if (!ok)
        {
            QString sErr = db->lastError().text();
            qCritical("Dattabase Error: %s",qUtf8Printable(sErr));
        }
    }

    return ok;
}

void dtbase::databaseDisConnect(QSqlDatabase *db)
{
    db->close();
    QString databaseName = "termBase";
    QSqlDatabase::removeDatabase(databaseName);
}

void dtbase::querytbArhiv(QDate dt, int slPeriod, QTableWidget *tbArh)
{
    if(databaseConnect(&db))
    {

        while(tbArh->rowCount() > 0) tbArh->removeRow(0);

        QSqlQuery qr = QSqlQuery(db);
        qr.prepare("SELECT * FROM tbArhiv WHERE dttm between :db and :de ORDER BY id");
        if (slPeriod == 0)
        {
            qr.bindValue(":db", dt);
            qr.bindValue(":de", dt.addDays(1));
        }
        else
        {
            QDate dtb = QDate(dt.year(),dt.month(),1);
            qr.bindValue(":db", dtb);
            qr.bindValue(":de", dtb.addMonths(1));
        }

        int rw = 0;
        if (qr.exec()) {
            while (qr.next()) {
                if (tbArh->rowCount()<=rw) tbArh->insertRow(rw);

                QTableWidgetItem* tw;

                tw = new QTableWidgetItem(qr.value("dttm").toDateTime().toString("dd.MM.yy hh:mm"));
                tbArh->setItem(rw,0,tw);

                tw = new QTableWidgetItem(QString::number(qr.value("pr3").toFloat(),'f',2));
                tbArh->setItem(rw,1,tw);

                tw = new QTableWidgetItem(QString::number(qr.value("dh1").toFloat(),'f',2));
                tbArh->setItem(rw,2,tw);
                tw = new QTableWidgetItem(QString::number(qr.value("dl1").toFloat(),'f',2));
                tbArh->setItem(rw,3,tw);
                tw = new QTableWidgetItem(QString::number(qr.value("pr1").toFloat(),'f',2));
                tbArh->setItem(rw,4,tw);

                tw = new QTableWidgetItem(QString::number(qr.value("dh2").toFloat(),'f',2));
                tbArh->setItem(rw,5,tw);
                tw = new QTableWidgetItem(QString::number(qr.value("dl2").toFloat(),'f',2));
                tbArh->setItem(rw,6,tw);
                tw = new QTableWidgetItem(QString::number(qr.value("pr2").toFloat(),'f',2));
                tbArh->setItem(rw,7,tw);

                tw = new QTableWidgetItem(QString::number(qr.value("id").toInt()));
                tbArh->setItem(rw,8,tw);

                rw++;
            }
            tbArh->setColumnHidden(8,true);
        }
    }
}


void PushThread::run()
{
    QSqlQuery qr = QSqlQuery(*db);

    if(qr.exec("SELECT MAX(id) as id FROM tbArhiv"))
    {
        qr.first();
        int idx = qr.value("id").toInt();
        idx +=1;

        qr.prepare("INSERT INTO tbArhiv (id,dttm,dh1,dl1,pr1,dh2,dl2,pr2,pr3) "
                    "VALUES (:id,:dttm,:dh1,:dl1,:pr1,:dh2,:dl2,:pr2,:pr3)");

        qr.bindValue(":id", idx);
        qr.bindValue(":dttm", QDateTime::currentDateTime());

        qr.bindValue(":dh1", tbRes->item(0,0)->text().toFloat());
        qr.bindValue(":dl1", tbRes->item(0,1)->text().toFloat());
        qr.bindValue(":pr1", tbRes->item(0,2)->text().toFloat());

        qr.bindValue(":dh2", tbRes->item(1,0)->text().toFloat());
        qr.bindValue(":dl2", tbRes->item(1,1)->text().toFloat());
        qr.bindValue(":pr2", tbRes->item(1,2)->text().toFloat());

        qr.bindValue(":pr3", tbRes->item(2,2)->text().toFloat());

        bool ok = qr.exec();
        if (!ok)
        {
            QString sErr = qr.lastError().text();
            qCritical("tbArhiv Error: %s", qUtf8Printable(sErr));
        }

        int cnt = pdSeris->count();
        qreal xx = -1.1;
        for (int ii=0;ii<cnt;ii++)
        {
            if (pdSeris->at(ii).x() > xx + 1)
            {
                qr.prepare("INSERT INTO tbGraph (id,X,Y) VALUES (:id,:X,:Y)");
                qr.bindValue(":id",idx);
                qr.bindValue(":X",pdSeris->at(ii).x());
                qr.bindValue(":Y",pdSeris->at(ii).y());

                bool ok = qr.exec();
                if (!ok)
                {
                    QString sErr = qr.lastError().text();
                    qCritical("tbGraph Error: %s", qUtf8Printable(sErr));
                }
                xx = pdSeris->at(ii).x();
            }
        }
        db->commit();
    }
}

void dtbase::pushtbArhiv(QTableWidget *tbRes, QLineSeries *pdSeris)
{
    if(databaseConnect(&db))
    {
        PushThread *pushThread = new PushThread;
        connect(pushThread, &PushThread::finished, pushThread, &QObject::deleteLater);
        pushThread->db = &db;
        pushThread->tbRes = tbRes;
        pushThread->pdSeris = pdSeris;
        pushThread->start();
    }
}

void dtbase::querytbArhSl(int id, QTableWidget *tbRes, QLineSeries *pdSeris,ChartMinMax *mx,QProgressBar *pbar)
{
    if(databaseConnect(&db))
    {
        QSqlQuery qr = QSqlQuery(db);

        qr.prepare("SELECT * FROM tbArhiv WHERE id=:id");
        qr.bindValue(":id",id);
        if(qr.exec())
        {
            qr.first();

            tbRes->item(0,0)->setText(QString::number(qr.value("dh1").toFloat(),'f',2));
            tbRes->item(0,1)->setText(QString::number(qr.value("dl1").toFloat(),'f',2));
            tbRes->item(0,2)->setText(QString::number(qr.value("pr1").toFloat(),'f',2));

            tbRes->item(1,0)->setText(QString::number(qr.value("dh2").toFloat(),'f',2));
            tbRes->item(1,1)->setText(QString::number(qr.value("dl2").toFloat(),'f',2));
            tbRes->item(1,2)->setText(QString::number(qr.value("pr2").toFloat(),'f',2));

            tbRes->item(2,2)->setText(QString::number(qr.value("pr3").toFloat(),'f',2));
            mx->title = qr.value("dttm").toDateTime().toString("dd.MM.yy hh:mm");

        }


        qr.prepare("SELECT COUNT(id) as cnt FROM tbGraph WHERE id=:id ORDER BY X");
        qr.bindValue(":id",id);
        qr.exec();
        qr.first();
        int cnt = qr.value("cnt").toInt();

        mx->maxX = 0;
        mx->minX = 0;
        mx->maxY = -1000;
        mx->minY = 1000;

        qr.prepare("SELECT * FROM tbGraph WHERE id=:id ORDER BY X");
        qr.bindValue(":id",id);
        if(qr.exec())
        {
            int ii = 0;
            pdSeris->clear();
            while (qr.next())
            {
                int pb = ii * 100 / cnt;
                pbar->setValue(pb);
                qreal XX = qr.value("x").toFloat();
                qreal YY = qr.value("y").toFloat();
                pdSeris->append(XX,YY);

                if( XX > mx->maxX) mx->maxX = XX;
                if( XX < mx->minX) mx->minX = XX;

                if( YY > mx->maxY) mx->maxY = YY;
                if( YY < mx->minY) mx->minY = YY;
                ii++;
            }
        }

        if(mx->maxY < mx->minY) mx->maxY = mx->minY;

        mx->maxY = mx->maxY + 0.1;
        mx->minY = mx->minY - 0.1;

        pbar->setValue(100);
    }
}

void dtbase::deltbArhiv(int id)
{
    if(databaseConnect(&db))
    {
        QSqlQuery qr = QSqlQuery(db);

        qr.prepare("DELETE FROM tbArhiv WHERE id=:id");
        qr.bindValue(":id",id);
        qr.exec();

        qr.prepare("DELETE FROM tbGraph WHERE id=:id");
        qr.bindValue(":id",id);
        qr.exec();

        db.commit();
    }
}
