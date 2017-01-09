#include "dialog_write.h"
#include "ui_dialog_write.h"

Dialog_Write::Dialog_Write(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_Write)
{
    ui->setupUi(this);
    IsWrite=false;
    connect(ui->pushButton_Write,SIGNAL(clicked()),this,SLOT(OnOkClicked()));
}

Dialog_Write::~Dialog_Write()
{
    delete ui;
}

QString Dialog_Write::get_Value_to_Write()
{
    return Value_to_Write;
}
bool Dialog_Write::get_IsWrite()
{
    return IsWrite;
}
void Dialog_Write::reset_IsWrite()
{
    IsWrite=false;
}

void Dialog_Write::OnOkClicked()
{
    Value_to_Write=ui->lineEdit_write->text();
    IsWrite=true;
}
