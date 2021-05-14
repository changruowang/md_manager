#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <QMainWindow>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QMouseEvent>
#include <QFileSystemModel>
#include <QLabel>
#include <stdlib.h>
#include "mytreeview.h"
#include "dialog.h"
#include "mdedit.h"

class MyFileSytemModel : public QFileSystemModel
{
    Q_OBJECT
public:
    MyFileSytemModel(QObject *parent = nullptr): QFileSystemModel(parent){};
    //第1列显示名称，其他几列都屏蔽掉
    int columnCount(const QModelIndex &parent) const{
        return 1;
    }
    QVariant  headerData(int section, Qt::Orientation orientation, int role) const {

        if ((section == 0) && (role == Qt::DisplayRole)) {
            return "笔记列表";
        } else {
            return QFileSystemModel::headerData(section,orientation,role);
        }
    }
};



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString m_basePath;

    QLabel* label;
    QTreeView* treeView;
    QString yamInform;
    QFileSystemModel *m_fileModel;
    QTimer* m_freshTimer;

    DialogParams* param = nullptr;
private:

    MdEdit* m_mdEditor;
    Ui::MainWindow *ui;
    Dialog *m_yamlDialog;
    bool m_linkState = true;

    void cmdThread();
private slots:
    //void onFileChanged(const QString &path);
    //void onDirectoryChanged(const QString &path);
    // void onFileInserted(const QString &path);
    void onTimeOut();

    void addYamlHead();
    void selLinkFile();
    void itemSelected(const QModelIndex &item);

    void on_action_sets_triggered();
    void on_action_fresh_triggered();
    void on_action_upload_triggered();
    void getDialogInform();
};
#endif // MAINWINDOW_H
