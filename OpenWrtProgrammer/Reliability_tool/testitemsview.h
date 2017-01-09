#ifndef TESTITEMSVIEW_H
#define TESTITEMSVIEW_H

#include <QDialog>
#include <QVector>
#include <QCheckBox>

namespace Ui {
    class TestItemsView;
}

class TestItemsView : public QDialog
{
    Q_OBJECT

public:
    explicit TestItemsView(QWidget *parent = 0);
    ~TestItemsView();

public slots:
    void onOKclicked();

signals:
    void signalTestItemsChanged();

private:
    Ui::TestItemsView *ui;
    QVector<QCheckBox*> vecBox;

};

#endif // TESTITEMSVIEW_H
