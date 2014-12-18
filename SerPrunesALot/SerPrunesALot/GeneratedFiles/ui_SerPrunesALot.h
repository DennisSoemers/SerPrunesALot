/********************************************************************************
** Form generated from reading UI file 'SerPrunesALot.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERPRUNESALOT_H
#define UI_SERPRUNESALOT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SerPrunesALotClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *SerPrunesALotClass)
    {
        if (SerPrunesALotClass->objectName().isEmpty())
            SerPrunesALotClass->setObjectName(QStringLiteral("SerPrunesALotClass"));
        SerPrunesALotClass->resize(600, 400);
        menuBar = new QMenuBar(SerPrunesALotClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        SerPrunesALotClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(SerPrunesALotClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        SerPrunesALotClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(SerPrunesALotClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        SerPrunesALotClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(SerPrunesALotClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        SerPrunesALotClass->setStatusBar(statusBar);

        retranslateUi(SerPrunesALotClass);

        QMetaObject::connectSlotsByName(SerPrunesALotClass);
    } // setupUi

    void retranslateUi(QMainWindow *SerPrunesALotClass)
    {
        SerPrunesALotClass->setWindowTitle(QApplication::translate("SerPrunesALotClass", "SerPrunesALot", 0));
    } // retranslateUi

};

namespace Ui {
    class SerPrunesALotClass: public Ui_SerPrunesALotClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERPRUNESALOT_H
