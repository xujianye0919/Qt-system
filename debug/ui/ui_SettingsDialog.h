/********************************************************************************
** Form generated from reading UI file 'SettingsDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QSpinBox *syncIntervalSpin;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout;
    QLineEdit *dbPathEdit;
    QPushButton *selectDbBtn;
    QLabel *label_3;
    QLineEdit *serverUrlEdit;
    QLabel *label_4;
    QCheckBox *autoSyncCheck;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *resetBtn;
    QSpacerItem *horizontalSpacer;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName("SettingsDialog");
        SettingsDialog->resize(500, 300);
        verticalLayout = new QVBoxLayout(SettingsDialog);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName("formLayout");
        label = new QLabel(SettingsDialog);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label);

        syncIntervalSpin = new QSpinBox(SettingsDialog);
        syncIntervalSpin->setObjectName("syncIntervalSpin");
        syncIntervalSpin->setMinimum(60);
        syncIntervalSpin->setMaximum(3600);
        syncIntervalSpin->setValue(600);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, syncIntervalSpin);

        label_2 = new QLabel(SettingsDialog);
        label_2->setObjectName("label_2");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        dbPathEdit = new QLineEdit(SettingsDialog);
        dbPathEdit->setObjectName("dbPathEdit");
        dbPathEdit->setReadOnly(true);

        horizontalLayout->addWidget(dbPathEdit);

        selectDbBtn = new QPushButton(SettingsDialog);
        selectDbBtn->setObjectName("selectDbBtn");

        horizontalLayout->addWidget(selectDbBtn);


        formLayout->setLayout(1, QFormLayout::ItemRole::FieldRole, horizontalLayout);

        label_3 = new QLabel(SettingsDialog);
        label_3->setObjectName("label_3");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, label_3);

        serverUrlEdit = new QLineEdit(SettingsDialog);
        serverUrlEdit->setObjectName("serverUrlEdit");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, serverUrlEdit);

        label_4 = new QLabel(SettingsDialog);
        label_4->setObjectName("label_4");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, label_4);

        autoSyncCheck = new QCheckBox(SettingsDialog);
        autoSyncCheck->setObjectName("autoSyncCheck");
        autoSyncCheck->setChecked(true);

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, autoSyncCheck);


        verticalLayout->addLayout(formLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        resetBtn = new QPushButton(SettingsDialog);
        resetBtn->setObjectName("resetBtn");

        horizontalLayout_2->addWidget(resetBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        saveBtn = new QPushButton(SettingsDialog);
        saveBtn->setObjectName("saveBtn");

        horizontalLayout_2->addWidget(saveBtn);

        cancelBtn = new QPushButton(SettingsDialog);
        cancelBtn->setObjectName("cancelBtn");

        horizontalLayout_2->addWidget(cancelBtn);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(SettingsDialog);
        QObject::connect(cancelBtn, &QPushButton::clicked, SettingsDialog, qOverload<>(&QDialog::close));
        QObject::connect(saveBtn, SIGNAL(clicked()), SettingsDialog, SLOT(on_saveBtn_clicked()));
        QObject::connect(selectDbBtn, SIGNAL(clicked()), SettingsDialog, SLOT(on_selectDbBtn_clicked()));
        QObject::connect(resetBtn, SIGNAL(clicked()), SettingsDialog, SLOT(on_resetBtn_clicked()));

        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QCoreApplication::translate("SettingsDialog", "\347\263\273\347\273\237\350\256\276\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("SettingsDialog", "\345\220\214\346\255\245\351\227\264\351\232\224\357\274\210\347\247\222\357\274\211\357\274\232", nullptr));
        label_2->setText(QCoreApplication::translate("SettingsDialog", "\346\225\260\346\215\256\345\272\223\350\267\257\345\276\204\357\274\232", nullptr));
        selectDbBtn->setText(QCoreApplication::translate("SettingsDialog", "\351\200\211\346\213\251", nullptr));
        label_3->setText(QCoreApplication::translate("SettingsDialog", "\346\234\215\345\212\241\345\231\250\345\234\260\345\235\200\357\274\232", nullptr));
        serverUrlEdit->setText(QCoreApplication::translate("SettingsDialog", "http://127.0.0.1:8080/api/sync", nullptr));
        serverUrlEdit->setPlaceholderText(QCoreApplication::translate("SettingsDialog", "\344\276\213\345\246\202\357\274\232http://your-server:8080/api/sync", nullptr));
        label_4->setText(QCoreApplication::translate("SettingsDialog", "\350\207\252\345\212\250\345\220\214\346\255\245\357\274\232", nullptr));
        autoSyncCheck->setText(QCoreApplication::translate("SettingsDialog", "\345\220\257\347\224\250\345\256\232\346\227\266\345\220\214\346\255\245", nullptr));
        resetBtn->setText(QCoreApplication::translate("SettingsDialog", "\346\201\242\345\244\215\351\273\230\350\256\244", nullptr));
        saveBtn->setText(QCoreApplication::translate("SettingsDialog", "\344\277\235\345\255\230\350\256\276\347\275\256", nullptr));
        cancelBtn->setText(QCoreApplication::translate("SettingsDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
