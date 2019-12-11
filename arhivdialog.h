#ifndef ARHIVDIALOG_H
#define ARHIVDIALOG_H

#include <QDialog>

namespace Ui {
class arhivdialog;
}

class dtbase;

class arhivdialog : public QDialog
{
    Q_OBJECT

public:
    explicit arhivdialog(QWidget *parent = nullptr);
    ~arhivdialog();

public slots:
    void frRefresh();
    void selectRow();
    void delMetering();

signals:
    void tbSelect(int id);

private:
    Ui::arhivdialog *ui;

private:
    dtbase *dtbs;
    int getcurID();

};

#endif // ARHIVDIALOG_H
