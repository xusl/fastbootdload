#include "testitemsview.h"
#include "ui_testitemsview.h"
#include <QSettings>
#include <QVBoxLayout>

TestItemsView::TestItemsView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestItemsView)
{
    ui->setupUi(this);
    //this->setWindowFlags(this->windowFlags()& ~Qt::WindowMaximizeButtonHint& ~Qt::WindowMinimizeButtonHint);
    this->setWindowTitle("Edit Test Items");
    connect(ui->btn_OK,SIGNAL(clicked()),this,SLOT(onOKclicked()));

    QSettings *configIni = new QSettings("TestItem.ini", QSettings::IniFormat);
    int allTestItems = configIni->value("/AllItems/Nums").toInt();
    QVBoxLayout *vLayout = new QVBoxLayout();
    QLabel      *label = new QLabel();
    vecBox.clear();
    for(int i = 0; i < allTestItems; i++)
    {
        QString prefix = QString("/AllItems/item%1").arg(i);
        QString aItem = configIni->value(prefix).toString();
        QCheckBox *checkItem = new QCheckBox(aItem,this);
        vLayout->addWidget(checkItem);
        label->setFixedHeight(25* (i + 1));

        vecBox.push_back(checkItem);

        QString selprefix = QString("/selectItems/item%1").arg(i);
        QString selItem = configIni->value(selprefix).toString();
        if(selItem == "no")
        {
            checkItem->setChecked(false);
        }
        else
        {
            checkItem->setChecked(true);
        }
    }
    label->setLayout(vLayout);
    ui->scrollArea->setWidget(label);
}

void TestItemsView::onOKclicked()
{
    for(int i = 0; i < vecBox.size(); i++)
    {
        QCheckBox *checkItem = vecBox.at(i);
        QSettings *configIni = new QSettings("TestItem.ini", QSettings::IniFormat);
        QString prefix = QString("/selectItems/item%1").arg(i);
        if(checkItem->isChecked() == true)
        {
            configIni->setValue(prefix,"yes");
        }
        else
        {
            configIni->setValue(prefix,"no");
        }
    }

    this->close();
    emit signalTestItemsChanged();
}

TestItemsView::~TestItemsView()
{
    delete ui;
}
