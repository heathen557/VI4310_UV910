/********************************************************************************
** Form generated from reading UI file 'filesave_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILESAVE_DIALOG_H
#define UI_FILESAVE_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_fileSave_Dialog
{
public:
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer_5;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_4;
    QCheckBox *RawTof_checkBox;
    QCheckBox *pileUpTof_checkBox;
    QCheckBox *filterTof_checkBox;
    QCheckBox *RawPeak_checkBox;
    QCheckBox *filterPeak_checkBox;
    QCheckBox *X_checkBox;
    QCheckBox *Y_checkBox;
    QCheckBox *Z_checkBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *SelFilePath_pushButton;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *start_saveFile_pushButton;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *cancel_save_pushButton;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_3;
    QLabel *SavedFrame_label;
    QSpacerItem *horizontalSpacer_6;

    void setupUi(QDialog *fileSave_Dialog)
    {
        if (fileSave_Dialog->objectName().isEmpty())
            fileSave_Dialog->setObjectName(QStringLiteral("fileSave_Dialog"));
        fileSave_Dialog->resize(678, 376);
        QIcon icon;
        icon.addFile(QStringLiteral("../testOpenGL3D_PCL_20200305_pileup/images/ccSave.png"), QSize(), QIcon::Normal, QIcon::Off);
        fileSave_Dialog->setWindowIcon(icon);
        gridLayout_2 = new QGridLayout(fileSave_Dialog);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        label_2 = new QLabel(fileSave_Dialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_3->addWidget(label_2);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_5);


        gridLayout_2->addLayout(horizontalLayout_3, 0, 0, 1, 1);

        groupBox = new QGroupBox(fileSave_Dialog);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        RawTof_checkBox = new QCheckBox(groupBox);
        RawTof_checkBox->setObjectName(QStringLiteral("RawTof_checkBox"));
        RawTof_checkBox->setChecked(true);

        horizontalLayout_4->addWidget(RawTof_checkBox);

        pileUpTof_checkBox = new QCheckBox(groupBox);
        pileUpTof_checkBox->setObjectName(QStringLiteral("pileUpTof_checkBox"));

        horizontalLayout_4->addWidget(pileUpTof_checkBox);

        filterTof_checkBox = new QCheckBox(groupBox);
        filterTof_checkBox->setObjectName(QStringLiteral("filterTof_checkBox"));

        horizontalLayout_4->addWidget(filterTof_checkBox);

        RawPeak_checkBox = new QCheckBox(groupBox);
        RawPeak_checkBox->setObjectName(QStringLiteral("RawPeak_checkBox"));
        RawPeak_checkBox->setChecked(true);

        horizontalLayout_4->addWidget(RawPeak_checkBox);

        filterPeak_checkBox = new QCheckBox(groupBox);
        filterPeak_checkBox->setObjectName(QStringLiteral("filterPeak_checkBox"));

        horizontalLayout_4->addWidget(filterPeak_checkBox);

        X_checkBox = new QCheckBox(groupBox);
        X_checkBox->setObjectName(QStringLiteral("X_checkBox"));

        horizontalLayout_4->addWidget(X_checkBox);

        Y_checkBox = new QCheckBox(groupBox);
        Y_checkBox->setObjectName(QStringLiteral("Y_checkBox"));

        horizontalLayout_4->addWidget(Y_checkBox);

        Z_checkBox = new QCheckBox(groupBox);
        Z_checkBox->setObjectName(QStringLiteral("Z_checkBox"));

        horizontalLayout_4->addWidget(Z_checkBox);

        horizontalLayout_4->setStretch(0, 2);
        horizontalLayout_4->setStretch(1, 2);
        horizontalLayout_4->setStretch(2, 2);
        horizontalLayout_4->setStretch(3, 2);
        horizontalLayout_4->setStretch(4, 2);
        horizontalLayout_4->setStretch(5, 1);
        horizontalLayout_4->setStretch(6, 1);
        horizontalLayout_4->setStretch(7, 1);

        gridLayout->addLayout(horizontalLayout_4, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setMaximumSize(QSize(50, 16777215));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        SelFilePath_pushButton = new QPushButton(groupBox);
        SelFilePath_pushButton->setObjectName(QStringLiteral("SelFilePath_pushButton"));
        SelFilePath_pushButton->setMaximumSize(QSize(25, 16777215));

        horizontalLayout->addWidget(SelFilePath_pushButton);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        start_saveFile_pushButton = new QPushButton(groupBox);
        start_saveFile_pushButton->setObjectName(QStringLiteral("start_saveFile_pushButton"));

        horizontalLayout_2->addWidget(start_saveFile_pushButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        cancel_save_pushButton = new QPushButton(groupBox);
        cancel_save_pushButton->setObjectName(QStringLiteral("cancel_save_pushButton"));

        horizontalLayout_2->addWidget(cancel_save_pushButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        horizontalLayout_2->setStretch(0, 3);
        horizontalLayout_2->setStretch(1, 1);
        horizontalLayout_2->setStretch(2, 1);
        horizontalLayout_2->setStretch(3, 1);
        horizontalLayout_2->setStretch(4, 3);

        gridLayout->addLayout(horizontalLayout_2, 2, 0, 1, 1);


        gridLayout_2->addWidget(groupBox, 1, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_3 = new QLabel(fileSave_Dialog);
        label_3->setObjectName(QStringLiteral("label_3"));

        horizontalLayout_5->addWidget(label_3);

        SavedFrame_label = new QLabel(fileSave_Dialog);
        SavedFrame_label->setObjectName(QStringLiteral("SavedFrame_label"));

        horizontalLayout_5->addWidget(SavedFrame_label);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_6);


        gridLayout_2->addLayout(horizontalLayout_5, 2, 0, 1, 1);


        retranslateUi(fileSave_Dialog);

        QMetaObject::connectSlotsByName(fileSave_Dialog);
    } // setupUi

    void retranslateUi(QDialog *fileSave_Dialog)
    {
        fileSave_Dialog->setWindowTitle(QApplication::translate("fileSave_Dialog", "\346\226\207\344\273\266\344\277\235\345\255\230\347\252\227\345\217\243", Q_NULLPTR));
        label_2->setText(QApplication::translate("fileSave_Dialog", "\346\226\207\344\273\266\344\277\235\345\255\230", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("fileSave_Dialog", "\346\216\245\346\224\266\346\225\260\346\215\256\344\277\235\345\255\230\357\274\232", Q_NULLPTR));
        RawTof_checkBox->setText(QApplication::translate("fileSave_Dialog", "\345\216\237\345\247\213TOF", Q_NULLPTR));
        pileUpTof_checkBox->setText(QApplication::translate("fileSave_Dialog", "pileUpTof", Q_NULLPTR));
        filterTof_checkBox->setText(QApplication::translate("fileSave_Dialog", "filter_tof", Q_NULLPTR));
        RawPeak_checkBox->setText(QApplication::translate("fileSave_Dialog", "\345\216\237\345\247\213peak", Q_NULLPTR));
        filterPeak_checkBox->setText(QApplication::translate("fileSave_Dialog", "filter_peak", Q_NULLPTR));
        X_checkBox->setText(QApplication::translate("fileSave_Dialog", "X", Q_NULLPTR));
        Y_checkBox->setText(QApplication::translate("fileSave_Dialog", "Y", Q_NULLPTR));
        Z_checkBox->setText(QApplication::translate("fileSave_Dialog", "Z", Q_NULLPTR));
        label->setText(QApplication::translate("fileSave_Dialog", "\345\255\230\345\202\250\350\267\257\345\276\204\357\274\232", Q_NULLPTR));
        SelFilePath_pushButton->setText(QApplication::translate("fileSave_Dialog", "...", Q_NULLPTR));
        start_saveFile_pushButton->setText(QApplication::translate("fileSave_Dialog", "\345\274\200\345\247\213\344\277\235\345\255\230", Q_NULLPTR));
        cancel_save_pushButton->setText(QApplication::translate("fileSave_Dialog", "\345\217\226\346\266\210\344\277\235\345\255\230", Q_NULLPTR));
        label_3->setText(QApplication::translate("fileSave_Dialog", "SavedFrame:", Q_NULLPTR));
        SavedFrame_label->setText(QApplication::translate("fileSave_Dialog", "0", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class fileSave_Dialog: public Ui_fileSave_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILESAVE_DIALOG_H
