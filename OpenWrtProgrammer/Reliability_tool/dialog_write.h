#ifndef DIALOG_WRITE_H
#define DIALOG_WRITE_H

#include <QDialog>

namespace Ui {
    class Dialog_Write;
}

class Dialog_Write : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Write(QWidget *parent = 0);
    QString get_Value_to_Write();
    bool get_IsWrite();
    void reset_IsWrite();
    ~Dialog_Write();
public slots:
    void OnOkClicked();

private:
    Ui::Dialog_Write *ui;
    QString Value_to_Write;
    bool IsWrite;
};

#endif // DIALOG_WRITE_H
