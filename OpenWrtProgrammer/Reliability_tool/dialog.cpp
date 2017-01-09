#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    IsWrite=false;
    connect(ui->pushButton_SSID_Write,SIGNAL(clicked()),this,SLOT(OnOkClicked()));
}

Dialog::~Dialog()
{
    delete ui;
}
QString Dialog::get_Value_to_Write()
{
    return Value_to_Write;
}
bool Dialog::get_IsWrite()
{
    return IsWrite;
}
void Dialog::reset_IsWrite()
{
    IsWrite=false;
}

void Dialog::OnOkClicked()
{
    Value_to_Write=ui->lineEdit_SSID_Write->text();
    IsWrite=true;
}
