#ifndef DTBASE_H
#define DTBASE_H

#include <QObject>
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QTableWidget>
#include <QtCharts>
#include <QtCharts/QLineSeries>
#include <QProgressBar>


struct ChartMinMax
{
    qreal maxX;
    qreal minX;
    qreal maxY;
    qreal minY;
    QString title;
};

class dtbase : public QObject
{
    Q_OBJECT
public:
    explicit dtbase(QObject *parent = nullptr);
    ~dtbase();

    bool databaseConnect(QSqlDatabase* db);
    void databaseDisConnect(QSqlDatabase* db);
    void querytbArhiv(QDate dt,int slPeriod,QTableWidget *tbArh);
    void pushtbArhiv(QTableWidget *tbRes,QLineSeries *pdSeris);
    void querytbArhSl(int id,QTableWidget *tbRes,QLineSeries *pdSeris,ChartMinMax *mx,QProgressBar *pbar);
    void deltbArhiv(int id);

signals:

public slots:

private:

};

class PushThread : public QThread
{
    Q_OBJECT
    void run() override;
public:
    QSqlDatabase *db;
    QTableWidget *tbRes;
    QLineSeries *pdSeris;
};


#endif // DTBASE_H
