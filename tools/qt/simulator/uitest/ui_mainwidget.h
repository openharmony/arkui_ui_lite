/********************************************************************************
 ** Form generated from reading UI file 'mainwidget.ui'
 **
 ** Created by: Qt User Interface Compiler version 5.14.2
 **
 ** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWIDGET_H
#define UI_MAINWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class UiMainWidget {
public:
    static void setupUi(QWidget* MainWidget)
    {
        if (MainWidget->objectName().isEmpty())
            MainWidget->setObjectName(QString::fromUtf8("MainWidget"));
        MainWidget->resize(800, 600);

        retranslateUi(MainWidget);

        QMetaObject::connectSlotsByName(MainWidget);
    } // setupUi

    static void retranslateUi(QWidget* MainWidget)
    {
        MainWidget->setWindowTitle(QCoreApplication::translate("MainWidget", "MainWidget", nullptr));
    } // retranslateUi
};

namespace Ui {
class MainWidget : public UiMainWidget {
};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIDGET_H
