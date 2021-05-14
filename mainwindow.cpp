#include <QDebug>
#include <QClipboard>
#include <QAbstractItemModel>
#include <string>
#include<QMenu>
#include <thread>
#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QLabel *permanent=new QLabel(this);
    permanent->setFrameStyle(QFrame::Box|QFrame::Sunken);
    permanent->setText(tr("<a href=\"http://changruowang.github.io/\">个人主页</a>"));
    permanent->setOpenExternalLinks(true);//设置可以打开网站链接
    ui->statusbar->addPermanentWidget(permanent);//显示永久信息

    m_mdEditor = new MdEdit(m_basePath.toStdString());
    if(!m_mdEditor->readJsonCathe()){    //没有缓存文件就以当前路径为基础扫描
        m_basePath = QDir::currentPath();
        m_mdEditor->setFilePath(m_basePath.toStdString());
        ui->statusbar->showMessage(tr("可以在设置页中设置笔记根路径！"));
    }else{
        m_basePath = QString(m_mdEditor->getFilePath().c_str());
    }
    yamInform = QString(m_mdEditor->getYamlHead().c_str() );
    m_mdEditor->makeMemoryCathe();
    m_mdEditor->makeJsonCathe();   // 第一次 合并历史记录和当前读取的信息后 重新存储

    QStringList filter;
    filter << "*.md";

    m_fileModel = new MyFileSytemModel(this);
    m_fileModel->setNameFilters(filter);
    m_fileModel->setNameFilterDisables(false);
    m_fileModel->setRootPath(m_basePath);
    m_fileModel->setReadOnly(false);

    treeView = new QTreeView(ui->centralwidget);
    treeView->setModel(m_fileModel);
    treeView->setSortingEnabled(true);
    treeView->setRootIndex(m_fileModel->index(m_basePath));  // 显示管理的目录
    treeView->setDragEnabled(true);
    treeView->setDropIndicatorShown(true);
    treeView->setDragDropMode(QAbstractItemView::InternalMove);
    treeView->setAcceptDrops(true);
    QObject::connect(treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(itemSelected(const QModelIndex &)));
    ui->verticalLayout->addWidget(treeView);

    m_freshTimer = new QTimer(this);
    connect(m_freshTimer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    m_freshTimer->start(600000);  // 十分钟更新一次
}


MainWindow::~MainWindow()
{
    m_mdEditor->makeMemoryCathe();
    m_mdEditor->makeJsonCathe();
    if(!param) delete param;
    delete m_mdEditor;
    delete ui;
}


void MainWindow::onTimeOut(){
    m_mdEditor->makeMemoryCathe();  // 扫描文件变动，并缓存
    m_mdEditor->makeJsonCathe();
    ui->statusbar->showMessage(tr("完成缓存更新！"), 2000);
    m_freshTimer->start(600000);
}


void MainWindow::addYamlHead(){
    QModelIndex &&index = treeView->currentIndex();
    string pathstr = m_fileModel->filePath(index).toStdString();
    m_mdEditor->insertYamlHeadUTF8(pathstr.c_str());
    ui->statusbar->showMessage(tr("已添加笔记头信息！"), 2000);
}


void MainWindow::selLinkFile(){
    QModelIndex &&index = treeView->currentIndex();
    static string path_last;

    if(m_linkState){
        path_last = m_fileModel->filePath(index).toStdString();
        m_linkState = false;
        ui->statusbar->showMessage(tr("单击要引用的文件..."));
    }else{
        m_linkState = true;

        string dstpath = m_fileModel->filePath(index).toStdString();
        string linkpath = m_mdEditor->getFileLink(path_last, dstpath);

        QClipboard *clip = QApplication::clipboard();
        clip->setText(QString(linkpath.c_str()));

        ui->statusbar->showMessage("引用链接已复制到粘贴板！", 2000);
    }

}

void MainWindow::itemSelected(const QModelIndex &index){

    if(!index.isValid() || ("md File" != m_fileModel->type(index))){
        ui->statusbar->clearMessage();
        m_linkState = true;
        return;
    }

    if(m_linkState){
        QMenu menu;
        menu.addAction("添加笔记信息", this, SLOT(addYamlHead()));
        menu.addAction("添加笔记引用", this, SLOT(selLinkFile()));
        menu.exec(QCursor::pos());
    }else{
        selLinkFile();
    }
}

void MainWindow::getDialogInform(){

    if(yamInform != param->head){
        yamInform = param->head;
        m_mdEditor->setYamlHead(yamInform.toStdString());
    }
    if(m_basePath != param->path){   //管理的根目录改了 需要删除缓存文件重新扫描建立
        m_basePath = param->path;

        m_mdEditor->setFilePath(m_basePath.toStdString());

        m_mdEditor->removeCatheFile();
        m_mdEditor->makeMemoryCathe();
        m_mdEditor->makeJsonCathe();   // 第一次 合并历史记录和当前读取的信息后 重新存储
        ui->statusbar->showMessage(tr("配置信息更新完成, 请重启程序"), 2000);
        return;
    }
    vector<string>& cmds = m_mdEditor->getPushCmds();
    cmds.clear();
    for(auto& s : param->cmds)
        cmds.push_back(s.toStdString());

    ui->statusbar->showMessage(tr("配置信息更新完成！"), 2000);
}



// 打开设置对话框
void MainWindow::on_action_sets_triggered()
{
    if(param == nullptr)
        param = new DialogParams(yamInform, m_basePath);

    param->cmds.clear();
    vector<string> cmds = m_mdEditor->getPushCmds();
    for(auto& c : cmds)
        param->cmds.push_back(QString(c.c_str()));

    m_yamlDialog = new Dialog(this, param);
    QObject::connect(m_yamlDialog, SIGNAL(sendSaveParamsSig()), this, SLOT(getDialogInform()));

    m_yamlDialog->show();
}



// 更新按钮按下  扫描文件变动 并存储
void MainWindow::on_action_fresh_triggered()
{
    m_mdEditor->makeMemoryCathe();
    m_mdEditor->makeJsonCathe();
    ui->statusbar->showMessage(tr("文档内容更新完成！"), 2000);
}

void MainWindow:: cmdThread(){
    vector<string>& cmds = m_mdEditor->getPushCmds();

    ui->statusbar->showMessage(tr("正在执行命令..."));
    for(auto& c : cmds){
        QString tmp = c.c_str();
        tmp.replace("\n", " & ");
        system( tmp.toStdString().c_str());
    }
    ui->statusbar->showMessage(tr("执行完毕！"), 2000);
}
// 编译上传按钮按下
void MainWindow::on_action_upload_triggered()
{
    std::thread t(&MainWindow::cmdThread, this);
    t.detach();
}
