/********************************************************************************
** Form generated from reading UI file 'statisticsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STATISTICSDIALOG_H
#define UI_STATISTICSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_statisticsDialog
{
public:
    QGridLayout *gridLayout_4;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer_2;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_2;
    QWidget *tofMean_widget;
    QWidget *tofStd_widget;
    QWidget *tab_2;
    QGridLayout *gridLayout_3;
    QWidget *peakMean_widget;
    QWidget *peakStd_widget;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_3;
    QFrame *frame;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *frame_lineEdit;
    QPushButton *start_pushButton;
    QPushButton *start_pushButton_2;
    QSpacerItem *horizontalSpacer_4;

    void setupUi(QDialog *statisticsDialog)
    {
        if (statisticsDialog->objectName().isEmpty())
            statisticsDialog->setObjectName(QStringLiteral("statisticsDialog"));
        statisticsDialog->resize(902, 663);
        QIcon icon;
        icon.addFile(QStringLiteral("icon_1.png"), QSize(), QIcon::Normal, QIcon::Off);
        statisticsDialog->setWindowIcon(icon);
        gridLayout_4 = new QGridLayout(statisticsDialog);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(258, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        label_2 = new QLabel(statisticsDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setStyleSheet(QString::fromUtf8("font: 16pt \"\351\273\221\344\275\223\";"));

        horizontalLayout_2->addWidget(label_2);

        horizontalSpacer_2 = new QSpacerItem(258, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        gridLayout_4->addLayout(horizontalLayout_2, 0, 0, 1, 1);

        tabWidget = new QTabWidget(statisticsDialog);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        gridLayout_2 = new QGridLayout(tab);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        tofMean_widget = new QWidget(tab);
        tofMean_widget->setObjectName(QStringLiteral("tofMean_widget"));
        tofMean_widget->setMaximumSize(QSize(16777215, 16777215));
        tofMean_widget->setStyleSheet(QStringLiteral(""));

        gridLayout_2->addWidget(tofMean_widget, 0, 0, 1, 1);

        tofStd_widget = new QWidget(tab);
        tofStd_widget->setObjectName(QStringLiteral("tofStd_widget"));
        tofStd_widget->setMinimumSize(QSize(0, 340));
        tofStd_widget->setMaximumSize(QSize(16777215, 16777215));
        tofStd_widget->setStyleSheet(QStringLiteral(""));

        gridLayout_2->addWidget(tofStd_widget, 1, 0, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        gridLayout_3 = new QGridLayout(tab_2);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        peakMean_widget = new QWidget(tab_2);
        peakMean_widget->setObjectName(QStringLiteral("peakMean_widget"));
        peakMean_widget->setStyleSheet(QStringLiteral(""));

        gridLayout_3->addWidget(peakMean_widget, 0, 0, 1, 1);

        peakStd_widget = new QWidget(tab_2);
        peakStd_widget->setObjectName(QStringLiteral("peakStd_widget"));
        peakStd_widget->setStyleSheet(QStringLiteral(""));

        gridLayout_3->addWidget(peakStd_widget, 1, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());

        gridLayout_4->addWidget(tabWidget, 1, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalSpacer_3 = new QSpacerItem(148, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        frame = new QFrame(statisticsDialog);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(frame);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(frame);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        frame_lineEdit = new QLineEdit(frame);
        frame_lineEdit->setObjectName(QStringLiteral("frame_lineEdit"));

        horizontalLayout->addWidget(frame_lineEdit);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        start_pushButton = new QPushButton(frame);
        start_pushButton->setObjectName(QStringLiteral("start_pushButton"));

        gridLayout->addWidget(start_pushButton, 0, 1, 1, 1);

        start_pushButton_2 = new QPushButton(frame);
        start_pushButton_2->setObjectName(QStringLiteral("start_pushButton_2"));

        gridLayout->addWidget(start_pushButton_2, 0, 2, 1, 1);


        horizontalLayout_3->addWidget(frame);

        horizontalSpacer_4 = new QSpacerItem(198, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);


        gridLayout_4->addLayout(horizontalLayout_3, 2, 0, 1, 1);


        retranslateUi(statisticsDialog);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(statisticsDialog);
    } // setupUi

    void retranslateUi(QDialog *statisticsDialog)
    {
        statisticsDialog->setWindowTitle(QApplication::translate("statisticsDialog", "\347\273\237\350\256\241\344\277\241\346\201\257\347\225\214\351\235\242", Q_NULLPTR));
        label_2->setText(QApplication::translate("statisticsDialog", "\347\273\237\350\256\241\344\277\241\346\201\257", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("statisticsDialog", "TOF\345\235\207\345\200\274\345\222\214\346\240\207\345\207\206\345\267\256", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("statisticsDialog", "PEAK\345\235\207\345\200\274\345\222\214\346\240\207\345\207\206\345\267\256", Q_NULLPTR));
        label->setText(QApplication::translate("statisticsDialog", "\347\273\237\350\256\241\345\270\247\346\225\260\357\274\232", Q_NULLPTR));
        frame_lineEdit->setText(QApplication::translate("statisticsDialog", "10", Q_NULLPTR));
        start_pushButton->setText(QApplication::translate("statisticsDialog", "\345\274\200\345\247\213\347\273\237\350\256\241", Q_NULLPTR));
        start_pushButton_2->setText(QApplication::translate("statisticsDialog", " \346\232\202\345\201\234\347\273\237\350\256\241", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class statisticsDialog: public Ui_statisticsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STATISTICSDIALOG_H
