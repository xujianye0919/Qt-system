/********************************************************************************
** Form generated from reading UI file 'NoticeManager.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NOTICEMANAGER_H
#define UI_NOTICEMANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_NoticeManager
{
public:
    QVBoxLayout *verticalLayout;
    QTableView *noticeTableView;
    QHBoxLayout *buttonLayout;
    QPushButton *addBtn;
    QPushButton *deleteBtn;
    QPushButton *scrollBtn;
    QPushButton *exportNoticeBtn;
    QSpacerItem *horizontalSpacer;
    QPushButton *closeBtn;

    void setupUi(QDialog *NoticeManager)
    {
        if (NoticeManager->objectName().isEmpty())
            NoticeManager->setObjectName("NoticeManager");
        NoticeManager->resize(800, 600);
        verticalLayout = new QVBoxLayout(NoticeManager);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        noticeTableView = new QTableView(NoticeManager);
        noticeTableView->setObjectName("noticeTableView");
        noticeTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        noticeTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout->addWidget(noticeTableView);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(6);
        buttonLayout->setObjectName("buttonLayout");
        addBtn = new QPushButton(NoticeManager);
        addBtn->setObjectName("addBtn");

        buttonLayout->addWidget(addBtn);

        deleteBtn = new QPushButton(NoticeManager);
        deleteBtn->setObjectName("deleteBtn");

        buttonLayout->addWidget(deleteBtn);

        scrollBtn = new QPushButton(NoticeManager);
        scrollBtn->setObjectName("scrollBtn");

        buttonLayout->addWidget(scrollBtn);

        exportNoticeBtn = new QPushButton(NoticeManager);
        exportNoticeBtn->setObjectName("exportNoticeBtn");

        buttonLayout->addWidget(exportNoticeBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        buttonLayout->addItem(horizontalSpacer);

        closeBtn = new QPushButton(NoticeManager);
        closeBtn->setObjectName("closeBtn");

        buttonLayout->addWidget(closeBtn);


        verticalLayout->addLayout(buttonLayout);


        retranslateUi(NoticeManager);
        QObject::connect(closeBtn, &QPushButton::clicked, NoticeManager, qOverload<>(&QDialog::close));
        QObject::connect(addBtn, SIGNAL(clicked()), NoticeManager, SLOT(on_addBtn_clicked()));
        QObject::connect(deleteBtn, SIGNAL(clicked()), NoticeManager, SLOT(on_deleteBtn_clicked()));
        QObject::connect(scrollBtn, SIGNAL(clicked()), NoticeManager, SLOT(on_scrollBtn_clicked()));
        QObject::connect(exportNoticeBtn, SIGNAL(clicked()), NoticeManager, SLOT(on_exportNoticeBtn_clicked()));

        QMetaObject::connectSlotsByName(NoticeManager);
    } // setupUi

    void retranslateUi(QDialog *NoticeManager)
    {
        NoticeManager->setWindowTitle(QCoreApplication::translate("NoticeManager", "\351\200\232\347\237\245\347\256\241\347\220\206", nullptr));
        addBtn->setText(QCoreApplication::translate("NoticeManager", "\346\267\273\345\212\240\351\200\232\347\237\245", nullptr));
        deleteBtn->setText(QCoreApplication::translate("NoticeManager", "\345\210\240\351\231\244\351\200\232\347\237\245", nullptr));
        scrollBtn->setText(QCoreApplication::translate("NoticeManager", "\345\210\207\346\215\242\346\273\232\345\212\250\347\212\266\346\200\201", nullptr));
        exportNoticeBtn->setText(QCoreApplication::translate("NoticeManager", "\345\257\274\345\207\272\351\200\232\347\237\245", nullptr));
        closeBtn->setText(QCoreApplication::translate("NoticeManager", "\345\205\263\351\227\255", nullptr));
    } // retranslateUi

};

namespace Ui {
    class NoticeManager: public Ui_NoticeManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NOTICEMANAGER_H
