#ifndef TRPARAM_H
#define TRPARAM_H

#include <QObject>
#include <QMap>

#define QMapParam QMap <QString,QString>

class trParam : public QObject
{
    Q_OBJECT
public:
    explicit trParam(QObject *parent = nullptr);

    void initialize(QString paramPth);
    void setParam(QString paramName,QString paramValue);
    QString getParam(QString paramName);
    void comParamRW(bool write = false);


signals:

public slots:

private:
//    static QString paramPath;
//    static QMapParam param;

};

extern trParam trPrm;

#endif // TRPARAM_H
