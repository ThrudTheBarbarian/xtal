/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "asmwidget.h"
#include "heatmapwidget.h"
#include "memorywidget.h"
#include "pointswidget.h"
#include "statewidget.h"
#include "vcrwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    MemoryWidget *memoryWidget;
    StateWidget *stateWidget;
    HeatMapWidget *heatMapWidget;
    QScrollArea *scrollPts;
    PointsWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QScrollArea *scrollAsm;
    AsmWidget *scrollAreaWidgetContents_2;
    VcrWidget *vcrWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1048, 1047);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        memoryWidget = new MemoryWidget(centralwidget);
        memoryWidget->setObjectName("memoryWidget");
        memoryWidget->setMinimumSize(QSize(300, 300));
        memoryWidget->setMaximumSize(QSize(300, 16777215));

        verticalLayout->addWidget(memoryWidget);

        stateWidget = new StateWidget(centralwidget);
        stateWidget->setObjectName("stateWidget");
        stateWidget->setMinimumSize(QSize(300, 80));
        stateWidget->setMaximumSize(QSize(300, 80));

        verticalLayout->addWidget(stateWidget);

        heatMapWidget = new HeatMapWidget(centralwidget);
        heatMapWidget->setObjectName("heatMapWidget");
        heatMapWidget->setMinimumSize(QSize(300, 80));

        verticalLayout->addWidget(heatMapWidget);


        horizontalLayout->addLayout(verticalLayout);

        scrollPts = new QScrollArea(centralwidget);
        scrollPts->setObjectName("scrollPts");
        scrollPts->setMinimumSize(QSize(120, 260));
        scrollPts->setWidgetResizable(true);
        scrollAreaWidgetContents = new PointsWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 349, 970));
        scrollPts->setWidget(scrollAreaWidgetContents);

        horizontalLayout->addWidget(scrollPts);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        scrollAsm = new QScrollArea(centralwidget);
        scrollAsm->setObjectName("scrollAsm");
        scrollAsm->setMinimumSize(QSize(120, 200));
        scrollAsm->setWidgetResizable(true);
        scrollAreaWidgetContents_2 = new AsmWidget();
        scrollAreaWidgetContents_2->setObjectName("scrollAreaWidgetContents_2");
        scrollAreaWidgetContents_2->setGeometry(QRect(0, 0, 347, 918));
        scrollAsm->setWidget(scrollAreaWidgetContents_2);

        verticalLayout_2->addWidget(scrollAsm);

        vcrWidget = new VcrWidget(centralwidget);
        vcrWidget->setObjectName("vcrWidget");
        vcrWidget->setMinimumSize(QSize(120, 40));

        verticalLayout_2->addWidget(vcrWidget);


        horizontalLayout->addLayout(verticalLayout_2);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1048, 24));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
