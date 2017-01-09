/********************************************************************************
** Form generated from reading UI file 'testitemsview.ui'
**
** Created: Thu Sep 4 16:03:58 2014
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTITEMSVIEW_H
#define UI_TESTITEMSVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TestItemsView
{
public:
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QLabel *label;
    QPushButton *btn_OK;

    void setupUi(QDialog *TestItemsView)
    {
        if (TestItemsView->objectName().isEmpty())
            TestItemsView->setObjectName(QString::fromUtf8("TestItemsView"));
        TestItemsView->resize(389, 445);
        scrollArea = new QScrollArea(TestItemsView);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setGeometry(QRect(0, 64, 389, 331));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 387, 329));
        scrollArea->setWidget(scrollAreaWidgetContents);
        label = new QLabel(TestItemsView);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(23, 23, 331, 20));
        QFont font;
        font.setFamily(QString::fromUtf8("Times New Roman"));
        font.setPointSize(12);
        label->setFont(font);
        btn_OK = new QPushButton(TestItemsView);
        btn_OK->setObjectName(QString::fromUtf8("btn_OK"));
        btn_OK->setGeometry(QRect(150, 410, 80, 23));

        retranslateUi(TestItemsView);

        QMetaObject::connectSlotsByName(TestItemsView);
    } // setupUi

    void retranslateUi(QDialog *TestItemsView)
    {
        TestItemsView->setWindowTitle(QApplication::translate("TestItemsView", "Edit Test Item", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("TestItemsView", "Note:you can test one item by checking it!!!", 0, QApplication::UnicodeUTF8));
        btn_OK->setText(QApplication::translate("TestItemsView", "OK", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TestItemsView: public Ui_TestItemsView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTITEMSVIEW_H
