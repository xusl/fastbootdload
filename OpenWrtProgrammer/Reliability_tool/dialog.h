#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    QString get_Value_to_Write();
    bool get_IsWrite();
    void reset_IsWrite();
    ~Dialog();
public slots:
    void OnOkClicked();

private:
    Ui::Dialog *ui;
    QString Value_to_Write;
    bool IsWrite;
};

#endif // DIALOG_H
