#include <QDebug>
#include <QPushButton>
#include <QDialogButtonBox>
#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent, DialogParams* p) :
    QDialog(parent), m_param(p),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    ui->pathTextEdit->setText(m_param->path);
    ui->yawTextEdit->setPlainText(QApplication::translate("Yaml Head", m_param->head.toStdString().c_str(), nullptr));

    ui->hexoTextEdit->setPlainText( m_param->cmds[0]);
    ui->gitTextEdit->setPlainText( m_param->cmds[1]);

}

Dialog::~Dialog()
{
    delete ui;
}



// 确认按钮
void Dialog::on_pushButton_clicked()
{
    m_param->head = ui->yawTextEdit->toPlainText();
    m_param->path = ui->pathTextEdit->text();

    QVector<QString> ans;
    ans.push_back(ui->hexoTextEdit->toPlainText());
    ans.push_back(ui->gitTextEdit->toPlainText());

    m_param->cmds = ans;

    emit(sendSaveParamsSig());

    this->close();
}
// 对话框取消按钮
void Dialog::on_pushButton_3_clicked()
{
    qDebug() << "取消 关闭对话框 " << endl;
    this->close();
}

