/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_5;
    QLabel *titleLabel;
    QHBoxLayout *horizontalLayout_1;
    QLabel *label_1;
    QComboBox *classComboBox;
    QLabel *label_2;
    QLineEdit *searchEdit;
    QSpacerItem *horizontalSpacer_1;
    QPushButton *exportBtn;
    QPushButton *noticeManagerBtn;
    QPushButton *settingsBtn;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_1;
    QFrame *currentCourseFrame;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_3;
    QLabel *currentCourseName;
    QLabel *currentCourseTeacher;
    QLabel *currentCourseTime;
    QLabel *countdownLabel;
    QFrame *nextCourseFrame;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_4;
    QLabel *nextCourseName;
    QLabel *nextCourseTeacher;
    QLabel *nextCourseTime;
    QSpacerItem *verticalSpacer_1;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_5;
    QTableView *classTableView;
    QLabel *marqueeLabel;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        verticalLayout_5 = new QVBoxLayout(centralWidget);
        verticalLayout_5->setSpacing(10);
        verticalLayout_5->setContentsMargins(15, 15, 15, 15);
        verticalLayout_5->setObjectName("verticalLayout_5");
        titleLabel = new QLabel(centralWidget);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_5->addWidget(titleLabel);

        horizontalLayout_1 = new QHBoxLayout();
        horizontalLayout_1->setSpacing(10);
        horizontalLayout_1->setObjectName("horizontalLayout_1");
        label_1 = new QLabel(centralWidget);
        label_1->setObjectName("label_1");

        horizontalLayout_1->addWidget(label_1);

        classComboBox = new QComboBox(centralWidget);
        classComboBox->setObjectName("classComboBox");
        classComboBox->setMinimumSize(QSize(300, 0));

        horizontalLayout_1->addWidget(classComboBox);

        label_2 = new QLabel(centralWidget);
        label_2->setObjectName("label_2");

        horizontalLayout_1->addWidget(label_2);

        searchEdit = new QLineEdit(centralWidget);
        searchEdit->setObjectName("searchEdit");

        horizontalLayout_1->addWidget(searchEdit);

        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_1->addItem(horizontalSpacer_1);

        exportBtn = new QPushButton(centralWidget);
        exportBtn->setObjectName("exportBtn");

        horizontalLayout_1->addWidget(exportBtn);

        noticeManagerBtn = new QPushButton(centralWidget);
        noticeManagerBtn->setObjectName("noticeManagerBtn");

        horizontalLayout_1->addWidget(noticeManagerBtn);

        settingsBtn = new QPushButton(centralWidget);
        settingsBtn->setObjectName("settingsBtn");

        horizontalLayout_1->addWidget(settingsBtn);


        verticalLayout_5->addLayout(horizontalLayout_1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(10);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        verticalLayout_1 = new QVBoxLayout();
        verticalLayout_1->setSpacing(10);
        verticalLayout_1->setObjectName("verticalLayout_1");
        currentCourseFrame = new QFrame(centralWidget);
        currentCourseFrame->setObjectName("currentCourseFrame");
        currentCourseFrame->setFrameShape(QFrame::StyledPanel);
        currentCourseFrame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(currentCourseFrame);
        verticalLayout_2->setSpacing(10);
        verticalLayout_2->setContentsMargins(15, 15, 15, 15);
        verticalLayout_2->setObjectName("verticalLayout_2");
        label_3 = new QLabel(currentCourseFrame);
        label_3->setObjectName("label_3");
        label_3->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(label_3);

        currentCourseName = new QLabel(currentCourseFrame);
        currentCourseName->setObjectName("currentCourseName");
        currentCourseName->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(currentCourseName);

        currentCourseTeacher = new QLabel(currentCourseFrame);
        currentCourseTeacher->setObjectName("currentCourseTeacher");
        currentCourseTeacher->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(currentCourseTeacher);

        currentCourseTime = new QLabel(currentCourseFrame);
        currentCourseTime->setObjectName("currentCourseTime");
        currentCourseTime->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(currentCourseTime);

        countdownLabel = new QLabel(currentCourseFrame);
        countdownLabel->setObjectName("countdownLabel");
        countdownLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(countdownLabel);


        verticalLayout_1->addWidget(currentCourseFrame);

        nextCourseFrame = new QFrame(centralWidget);
        nextCourseFrame->setObjectName("nextCourseFrame");
        nextCourseFrame->setFrameShape(QFrame::StyledPanel);
        nextCourseFrame->setFrameShadow(QFrame::Raised);
        verticalLayout_3 = new QVBoxLayout(nextCourseFrame);
        verticalLayout_3->setSpacing(10);
        verticalLayout_3->setContentsMargins(15, 15, 15, 15);
        verticalLayout_3->setObjectName("verticalLayout_3");
        label_4 = new QLabel(nextCourseFrame);
        label_4->setObjectName("label_4");
        label_4->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(label_4);

        nextCourseName = new QLabel(nextCourseFrame);
        nextCourseName->setObjectName("nextCourseName");
        nextCourseName->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(nextCourseName);

        nextCourseTeacher = new QLabel(nextCourseFrame);
        nextCourseTeacher->setObjectName("nextCourseTeacher");
        nextCourseTeacher->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(nextCourseTeacher);

        nextCourseTime = new QLabel(nextCourseFrame);
        nextCourseTime->setObjectName("nextCourseTime");
        nextCourseTime->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(nextCourseTime);


        verticalLayout_1->addWidget(nextCourseFrame);

        verticalSpacer_1 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_1->addItem(verticalSpacer_1);


        horizontalLayout_2->addLayout(verticalLayout_1);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(10);
        verticalLayout_4->setObjectName("verticalLayout_4");
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName("label_5");
        label_5->setAlignment(Qt::AlignCenter);

        verticalLayout_4->addWidget(label_5);

        classTableView = new QTableView(centralWidget);
        classTableView->setObjectName("classTableView");
        classTableView->setMinimumSize(QSize(600, 0));
        classTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        classTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout_4->addWidget(classTableView);


        horizontalLayout_2->addLayout(verticalLayout_4);


        verticalLayout_5->addLayout(horizontalLayout_2);

        marqueeLabel = new QLabel(centralWidget);
        marqueeLabel->setObjectName("marqueeLabel");
        marqueeLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_5->addWidget(marqueeLabel);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 1200, 26));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);
        QObject::connect(classComboBox, SIGNAL(currentIndexChanged(int)), MainWindow, SLOT(onClassSelected(int)));
        QObject::connect(searchEdit, SIGNAL(textChanged(QString)), MainWindow, SLOT(onSearchTextChanged(QString)));
        QObject::connect(exportBtn, SIGNAL(clicked()), MainWindow, SLOT(onExportBtnClicked()));
        QObject::connect(noticeManagerBtn, SIGNAL(clicked()), MainWindow, SLOT(onNoticeManagerBtnClicked()));
        QObject::connect(settingsBtn, SIGNAL(clicked()), MainWindow, SLOT(onSettingsBtnClicked()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\346\225\231\345\256\244\347\217\255\347\211\214\344\277\241\346\201\257\345\261\225\347\244\272\347\263\273\347\273\237 V1.0", nullptr));
        titleLabel->setText(QCoreApplication::translate("MainWindow", "\346\225\231\345\256\244\347\217\255\347\211\214\344\277\241\346\201\257\345\261\225\347\244\272\347\263\273\347\273\237", nullptr));
        titleLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 24px; font-weight: bold; color: #2c3e50;", nullptr));
        label_1->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\347\217\255\347\272\247\357\274\232", nullptr));
        label_1->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 14px;", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "\346\220\234\347\264\242\357\274\232", nullptr));
        label_2->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 14px;", nullptr));
        searchEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\350\276\223\345\205\245\347\217\255\347\272\247/\346\225\231\345\256\244/\351\231\242\347\263\273\345\205\263\351\224\256\350\257\215...", nullptr));
        exportBtn->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\207\272\346\225\260\346\215\256", nullptr));
        noticeManagerBtn->setText(QCoreApplication::translate("MainWindow", "\351\200\232\347\237\245\347\256\241\347\220\206", nullptr));
        settingsBtn->setText(QCoreApplication::translate("MainWindow", "\347\263\273\347\273\237\350\256\276\347\275\256", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "\345\275\223\345\211\215\350\257\276\347\250\213", nullptr));
        label_3->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 16px; font-weight: bold;", nullptr));
        currentCourseName->setText(QCoreApplication::translate("MainWindow", "\346\232\202\346\227\240\350\257\276\347\250\213", nullptr));
        currentCourseName->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 20px; color: #e74c3c;", nullptr));
        currentCourseTeacher->setText(QCoreApplication::translate("MainWindow", "\344\273\273\350\257\276\346\225\231\345\270\210\357\274\232\346\232\202\346\227\240", nullptr));
        currentCourseTime->setText(QCoreApplication::translate("MainWindow", "\344\270\212\350\257\276\346\227\266\351\227\264\357\274\232--:-- \350\207\263 --:--", nullptr));
        countdownLabel->setText(QCoreApplication::translate("MainWindow", "\345\200\222\350\256\241\346\227\266\357\274\23200:00:00", nullptr));
        countdownLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 18px; font-weight: bold; color: #e74c3c;", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "\344\270\213\344\270\200\350\212\202\350\257\276", nullptr));
        label_4->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 16px; font-weight: bold;", nullptr));
        nextCourseName->setText(QCoreApplication::translate("MainWindow", "\346\232\202\346\227\240\350\257\276\347\250\213", nullptr));
        nextCourseName->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 20px; color: #3498db;", nullptr));
        nextCourseTeacher->setText(QCoreApplication::translate("MainWindow", "\344\273\273\350\257\276\346\225\231\345\270\210\357\274\232\346\232\202\346\227\240", nullptr));
        nextCourseTime->setText(QCoreApplication::translate("MainWindow", "\344\270\212\350\257\276\346\227\266\351\227\264\357\274\232--:-- \350\207\263 --:--", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "\346\234\254\345\221\250\350\257\276\350\241\250", nullptr));
        label_5->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 18px; font-weight: bold;", nullptr));
        marqueeLabel->setText(QCoreApplication::translate("MainWindow", "\346\254\242\350\277\216\344\275\277\347\224\250\346\225\231\345\256\244\347\217\255\347\211\214\344\277\241\346\201\257\345\261\225\347\244\272\347\263\273\347\273\237 - \346\232\202\346\227\240\346\273\232\345\212\250\351\200\232\347\237\245", nullptr));
        marqueeLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #fff3cd; color: #856404; padding: 10px; border-radius: 5px;", nullptr));
        statusBar->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #f8f9fa;", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
