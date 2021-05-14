#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H
#include <QMouseEvent>
#include <QTreeView>
#include<QDragMoveEvent>

class MyTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit MyTreeView(QWidget *parent = nullptr);

protected:
    //void dragMoveEvent(QDragMoveEvent *event);
    // void mouseMoveEvent(QMouseEvent * event);
    void dropEvent(QDropEvent* event);
private:
};

#endif // MYTREEVIEW_H
