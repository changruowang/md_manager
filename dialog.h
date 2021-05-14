#ifndef DIALOG_H
#define DIALOG_H
#include <QPlainTextEdit>
#include <QAbstractButton>
#include <QDialog>
#include <vector>
#include <iostream>
namespace Ui {
class Dialog;
}


struct DialogParams{
    DialogParams(QString h, QString p, QVector<QString> c): head(h), path(p), cmds(c){};
    DialogParams(QString h, QString p): head(h), path(p){};
    QString head;
    QString path;
    QVector<QString> cmds;
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr, DialogParams* p=nullptr);
    ~Dialog();

    DialogParams* m_param;
//    QString yamInform;
//    QPlainTextEdit *plainTextEdit;
signals:
    void sendSaveParamsSig();

private slots:


    void on_pushButton_clicked();

    void on_pushButton_3_clicked();


private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
