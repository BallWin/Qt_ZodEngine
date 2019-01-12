/********************************************************************************
** Form generated from reading UI file 'qz_launcher.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QZ_LAUNCHER_H
#define UI_QZ_LAUNCHER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_QZ_Launcher
{
public:
    QGridLayout *gridLayout_6;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QLabel *label_5;
    QSpacerItem *horizontalSpacer_4;
    QLineEdit *le_PlayerName;
    QGroupBox *gbox_ConnectSetting;
    QGridLayout *gridLayout;
    QSplitter *splitter;
    QLabel *label;
    QLineEdit *le_ConnectAdress;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer;
    QLineEdit *le_LoginName;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QSpacerItem *horizontalSpacer_2;
    QLineEdit *le_Password;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QSpacerItem *horizontalSpacer_3;
    QComboBox *cb_TeamColor;
    QGroupBox *groupBox_GameMode;
    QRadioButton *rb_Localame;
    QRadioButton *rb_Network_Nighsoft;
    QGroupBox *groupBox_Maps;
    QGridLayout *gridLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_6;
    QSpacerItem *horizontalSpacer_5;
    QComboBox *cb_Maps_Campaing;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_7;
    QSpacerItem *horizontalSpacer_6;
    QComboBox *cb_Maps_SetMaps;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_8;
    QSpacerItem *horizontalSpacer_7;
    QLineEdit *le_StartLayer_Number;
    QFrame *line;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_9;
    QSpacerItem *horizontalSpacer_8;
    QLineEdit *le_FilePath_Map;
    QToolButton *tb_SelectFileMap;
    QGroupBox *groupBox_OPtions;
    QGridLayout *gridLayout_4;
    QVBoxLayout *verticalLayout;
    QCheckBox *chb_isWindow;
    QCheckBox *chb_NoMusic;
    QCheckBox *chb_NoSounds;
    QCheckBox *chb_NoOpenGL;
    QCheckBox *chb_blandCU;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *gb_Colors;
    QGridLayout *gridLayout_5;
    QLabel *label_12;
    QRadioButton *rb_Color_Red;
    QRadioButton *rb_Color_Blue;
    QRadioButton *rb_Color_Green;
    QRadioButton *rb_Color_Yellow;
    QSpacerItem *verticalSpacer;
    QPushButton *pb_PreviousSetting;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_10;
    QSpacerItem *horizontalSpacer_9;
    QLineEdit *le_Resolution_width;
    QLabel *label_11;
    QLineEdit *le_Resolution_height;
    QLineEdit *le_command_line_setting;
    QHBoxLayout *horizontalLayout_9;
    QPushButton *pb_Save_Exit;
    QPushButton *pb_Exit;
    QPushButton *pb_Launch_Zod;

    void setupUi(QDialog *QZ_Launcher)
    {
        if (QZ_Launcher->objectName().isEmpty())
            QZ_Launcher->setObjectName(QStringLiteral("QZ_Launcher"));
        QZ_Launcher->resize(603, 445);
        gridLayout_6 = new QGridLayout(QZ_Launcher);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        groupBox_2 = new QGroupBox(QZ_Launcher);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout_2->addWidget(label_5, 0, 0, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(5, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_4, 0, 1, 1, 1);

        le_PlayerName = new QLineEdit(groupBox_2);
        le_PlayerName->setObjectName(QStringLiteral("le_PlayerName"));
        le_PlayerName->setMinimumSize(QSize(100, 0));

        gridLayout_2->addWidget(le_PlayerName, 0, 2, 1, 1);


        gridLayout_6->addWidget(groupBox_2, 0, 0, 1, 1);

        gbox_ConnectSetting = new QGroupBox(QZ_Launcher);
        gbox_ConnectSetting->setObjectName(QStringLiteral("gbox_ConnectSetting"));
        gridLayout = new QGridLayout(gbox_ConnectSetting);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        splitter = new QSplitter(gbox_ConnectSetting);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        label = new QLabel(splitter);
        label->setObjectName(QStringLiteral("label"));
        splitter->addWidget(label);
        le_ConnectAdress = new QLineEdit(splitter);
        le_ConnectAdress->setObjectName(QStringLiteral("le_ConnectAdress"));
        splitter->addWidget(le_ConnectAdress);

        gridLayout->addWidget(splitter, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_2 = new QLabel(gbox_ConnectSetting);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        le_LoginName = new QLineEdit(gbox_ConnectSetting);
        le_LoginName->setObjectName(QStringLiteral("le_LoginName"));

        horizontalLayout->addWidget(le_LoginName);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_3 = new QLabel(gbox_ConnectSetting);
        label_3->setObjectName(QStringLiteral("label_3"));

        horizontalLayout_2->addWidget(label_3);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        le_Password = new QLineEdit(gbox_ConnectSetting);
        le_Password->setObjectName(QStringLiteral("le_Password"));

        horizontalLayout_2->addWidget(le_Password);


        gridLayout->addLayout(horizontalLayout_2, 2, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_4 = new QLabel(gbox_ConnectSetting);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout_3->addWidget(label_4);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        cb_TeamColor = new QComboBox(gbox_ConnectSetting);
        cb_TeamColor->setObjectName(QStringLiteral("cb_TeamColor"));
        cb_TeamColor->setMinimumSize(QSize(140, 0));

        horizontalLayout_3->addWidget(cb_TeamColor);


        gridLayout->addLayout(horizontalLayout_3, 3, 0, 1, 1);


        gridLayout_6->addWidget(gbox_ConnectSetting, 0, 1, 2, 2);

        groupBox_GameMode = new QGroupBox(QZ_Launcher);
        groupBox_GameMode->setObjectName(QStringLiteral("groupBox_GameMode"));
        rb_Localame = new QRadioButton(groupBox_GameMode);
        rb_Localame->setObjectName(QStringLiteral("rb_Localame"));
        rb_Localame->setGeometry(QRect(10, 30, 112, 23));
        rb_Localame->setChecked(true);
        rb_Network_Nighsoft = new QRadioButton(groupBox_GameMode);
        rb_Network_Nighsoft->setObjectName(QStringLiteral("rb_Network_Nighsoft"));
        rb_Network_Nighsoft->setGeometry(QRect(10, 60, 209, 23));

        gridLayout_6->addWidget(groupBox_GameMode, 1, 0, 1, 1);

        groupBox_Maps = new QGroupBox(QZ_Launcher);
        groupBox_Maps->setObjectName(QStringLiteral("groupBox_Maps"));
        gridLayout_3 = new QGridLayout(groupBox_Maps);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        label_6 = new QLabel(groupBox_Maps);
        label_6->setObjectName(QStringLiteral("label_6"));

        horizontalLayout_4->addWidget(label_6);

        horizontalSpacer_5 = new QSpacerItem(13, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_5);

        cb_Maps_Campaing = new QComboBox(groupBox_Maps);
        cb_Maps_Campaing->setObjectName(QStringLiteral("cb_Maps_Campaing"));
        cb_Maps_Campaing->setMinimumSize(QSize(140, 0));

        horizontalLayout_4->addWidget(cb_Maps_Campaing);


        gridLayout_3->addLayout(horizontalLayout_4, 0, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_7 = new QLabel(groupBox_Maps);
        label_7->setObjectName(QStringLiteral("label_7"));

        horizontalLayout_5->addWidget(label_7);

        horizontalSpacer_6 = new QSpacerItem(13, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_6);

        cb_Maps_SetMaps = new QComboBox(groupBox_Maps);
        cb_Maps_SetMaps->setObjectName(QStringLiteral("cb_Maps_SetMaps"));
        cb_Maps_SetMaps->setMinimumSize(QSize(140, 0));
        cb_Maps_SetMaps->setEditable(true);

        horizontalLayout_5->addWidget(cb_Maps_SetMaps);


        gridLayout_3->addLayout(horizontalLayout_5, 1, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_8 = new QLabel(groupBox_Maps);
        label_8->setObjectName(QStringLiteral("label_8"));

        horizontalLayout_6->addWidget(label_8);

        horizontalSpacer_7 = new QSpacerItem(13, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_7);

        le_StartLayer_Number = new QLineEdit(groupBox_Maps);
        le_StartLayer_Number->setObjectName(QStringLiteral("le_StartLayer_Number"));

        horizontalLayout_6->addWidget(le_StartLayer_Number);


        gridLayout_3->addLayout(horizontalLayout_6, 2, 0, 1, 1);

        line = new QFrame(groupBox_Maps);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout_3->addWidget(line, 3, 0, 1, 1);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        label_9 = new QLabel(groupBox_Maps);
        label_9->setObjectName(QStringLiteral("label_9"));

        horizontalLayout_7->addWidget(label_9);

        horizontalSpacer_8 = new QSpacerItem(13, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_8);

        le_FilePath_Map = new QLineEdit(groupBox_Maps);
        le_FilePath_Map->setObjectName(QStringLiteral("le_FilePath_Map"));

        horizontalLayout_7->addWidget(le_FilePath_Map);

        tb_SelectFileMap = new QToolButton(groupBox_Maps);
        tb_SelectFileMap->setObjectName(QStringLiteral("tb_SelectFileMap"));

        horizontalLayout_7->addWidget(tb_SelectFileMap);


        gridLayout_3->addLayout(horizontalLayout_7, 4, 0, 1, 1);


        gridLayout_6->addWidget(groupBox_Maps, 2, 0, 1, 1);

        groupBox_OPtions = new QGroupBox(QZ_Launcher);
        groupBox_OPtions->setObjectName(QStringLiteral("groupBox_OPtions"));
        gridLayout_4 = new QGridLayout(groupBox_OPtions);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        chb_isWindow = new QCheckBox(groupBox_OPtions);
        chb_isWindow->setObjectName(QStringLiteral("chb_isWindow"));

        verticalLayout->addWidget(chb_isWindow);

        chb_NoMusic = new QCheckBox(groupBox_OPtions);
        chb_NoMusic->setObjectName(QStringLiteral("chb_NoMusic"));

        verticalLayout->addWidget(chb_NoMusic);

        chb_NoSounds = new QCheckBox(groupBox_OPtions);
        chb_NoSounds->setObjectName(QStringLiteral("chb_NoSounds"));

        verticalLayout->addWidget(chb_NoSounds);

        chb_NoOpenGL = new QCheckBox(groupBox_OPtions);
        chb_NoOpenGL->setObjectName(QStringLiteral("chb_NoOpenGL"));

        verticalLayout->addWidget(chb_NoOpenGL);

        chb_blandCU = new QCheckBox(groupBox_OPtions);
        chb_blandCU->setObjectName(QStringLiteral("chb_blandCU"));

        verticalLayout->addWidget(chb_blandCU);


        gridLayout_4->addLayout(verticalLayout, 0, 0, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gb_Colors = new QGroupBox(groupBox_OPtions);
        gb_Colors->setObjectName(QStringLiteral("gb_Colors"));
        gridLayout_5 = new QGridLayout(gb_Colors);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        label_12 = new QLabel(gb_Colors);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout_5->addWidget(label_12, 0, 0, 1, 1);

        rb_Color_Red = new QRadioButton(gb_Colors);
        rb_Color_Red->setObjectName(QStringLiteral("rb_Color_Red"));
        rb_Color_Red->setMinimumSize(QSize(23, 23));
        rb_Color_Red->setMaximumSize(QSize(32, 16777215));
        rb_Color_Red->setStyleSheet(QLatin1String("background-color: rgb(253, 118, 118);\n"
"border-color: rgb(0, 0, 0);"));

        gridLayout_5->addWidget(rb_Color_Red, 0, 1, 1, 1);

        rb_Color_Blue = new QRadioButton(gb_Colors);
        rb_Color_Blue->setObjectName(QStringLiteral("rb_Color_Blue"));
        rb_Color_Blue->setMinimumSize(QSize(23, 23));
        rb_Color_Blue->setMaximumSize(QSize(32, 16777215));
        rb_Color_Blue->setStyleSheet(QLatin1String("background-color: rgb(129, 136, 255);\n"
"border-color: rgb(0, 0, 0);"));
        rb_Color_Blue->setChecked(true);

        gridLayout_5->addWidget(rb_Color_Blue, 0, 2, 1, 1);

        rb_Color_Green = new QRadioButton(gb_Colors);
        rb_Color_Green->setObjectName(QStringLiteral("rb_Color_Green"));
        rb_Color_Green->setMinimumSize(QSize(23, 23));
        rb_Color_Green->setMaximumSize(QSize(32, 16777215));
        rb_Color_Green->setStyleSheet(QLatin1String("background-color: rgb(124, 253, 103);\n"
"border-color: rgb(0, 0, 0);"));

        gridLayout_5->addWidget(rb_Color_Green, 0, 3, 1, 1);

        rb_Color_Yellow = new QRadioButton(gb_Colors);
        rb_Color_Yellow->setObjectName(QStringLiteral("rb_Color_Yellow"));
        rb_Color_Yellow->setMinimumSize(QSize(23, 23));
        rb_Color_Yellow->setMaximumSize(QSize(32, 16777215));
        rb_Color_Yellow->setStyleSheet(QLatin1String("background-color: rgb(253, 255, 134);\n"
"border-color: rgb(0, 0, 0);"));

        gridLayout_5->addWidget(rb_Color_Yellow, 0, 4, 1, 1);


        verticalLayout_2->addWidget(gb_Colors);

        verticalSpacer = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        pb_PreviousSetting = new QPushButton(groupBox_OPtions);
        pb_PreviousSetting->setObjectName(QStringLiteral("pb_PreviousSetting"));

        verticalLayout_2->addWidget(pb_PreviousSetting);


        gridLayout_4->addLayout(verticalLayout_2, 0, 1, 1, 1);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        label_10 = new QLabel(groupBox_OPtions);
        label_10->setObjectName(QStringLiteral("label_10"));

        horizontalLayout_8->addWidget(label_10);

        horizontalSpacer_9 = new QSpacerItem(13, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_9);

        le_Resolution_width = new QLineEdit(groupBox_OPtions);
        le_Resolution_width->setObjectName(QStringLiteral("le_Resolution_width"));
        le_Resolution_width->setMaximumSize(QSize(75, 16777215));

        horizontalLayout_8->addWidget(le_Resolution_width);

        label_11 = new QLabel(groupBox_OPtions);
        label_11->setObjectName(QStringLiteral("label_11"));

        horizontalLayout_8->addWidget(label_11);

        le_Resolution_height = new QLineEdit(groupBox_OPtions);
        le_Resolution_height->setObjectName(QStringLiteral("le_Resolution_height"));
        le_Resolution_height->setMaximumSize(QSize(75, 16777215));

        horizontalLayout_8->addWidget(le_Resolution_height);


        gridLayout_4->addLayout(horizontalLayout_8, 1, 0, 1, 2);


        gridLayout_6->addWidget(groupBox_OPtions, 2, 1, 1, 2);

        le_command_line_setting = new QLineEdit(QZ_Launcher);
        le_command_line_setting->setObjectName(QStringLiteral("le_command_line_setting"));
        le_command_line_setting->setMinimumSize(QSize(300, 0));

        gridLayout_6->addWidget(le_command_line_setting, 3, 0, 1, 1);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        pb_Save_Exit = new QPushButton(QZ_Launcher);
        pb_Save_Exit->setObjectName(QStringLiteral("pb_Save_Exit"));

        horizontalLayout_9->addWidget(pb_Save_Exit);

        pb_Exit = new QPushButton(QZ_Launcher);
        pb_Exit->setObjectName(QStringLiteral("pb_Exit"));

        horizontalLayout_9->addWidget(pb_Exit);

        pb_Launch_Zod = new QPushButton(QZ_Launcher);
        pb_Launch_Zod->setObjectName(QStringLiteral("pb_Launch_Zod"));

        horizontalLayout_9->addWidget(pb_Launch_Zod);


        gridLayout_6->addLayout(horizontalLayout_9, 3, 2, 1, 1);


        retranslateUi(QZ_Launcher);
        QObject::connect(pb_Exit, SIGNAL(clicked()), QZ_Launcher, SLOT(close()));

        QMetaObject::connectSlotsByName(QZ_Launcher);
    } // setupUi

    void retranslateUi(QDialog *QZ_Launcher)
    {
        QZ_Launcher->setWindowTitle(QApplication::translate("QZ_Launcher", "QZod Launcher", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("QZ_Launcher", "Player", Q_NULLPTR));
        label_5->setText(QApplication::translate("QZ_Launcher", "Player name", Q_NULLPTR));
        gbox_ConnectSetting->setTitle(QApplication::translate("QZ_Launcher", "Connection Settings (Team Deathmatch)", Q_NULLPTR));
        label->setText(QApplication::translate("QZ_Launcher", "Adress", Q_NULLPTR));
        label_2->setText(QApplication::translate("QZ_Launcher", "Login Name", Q_NULLPTR));
        label_3->setText(QApplication::translate("QZ_Launcher", "Password", Q_NULLPTR));
        le_Password->setText(QString());
        label_4->setText(QApplication::translate("QZ_Launcher", "Team", Q_NULLPTR));
        groupBox_GameMode->setTitle(QApplication::translate("QZ_Launcher", "Preset Settings", Q_NULLPTR));
        rb_Localame->setText(QApplication::translate("QZ_Launcher", "Single player", Q_NULLPTR));
        rb_Network_Nighsoft->setText(QApplication::translate("QZ_Launcher", "Multi player", Q_NULLPTR));
        groupBox_Maps->setTitle(QApplication::translate("QZ_Launcher", "Maps", Q_NULLPTR));
        label_6->setText(QApplication::translate("QZ_Launcher", "Campaign", Q_NULLPTR));
        label_7->setText(QApplication::translate("QZ_Launcher", " Set of maps", Q_NULLPTR));
        label_8->setText(QApplication::translate("QZ_Launcher", "Start level", Q_NULLPTR));
        label_9->setText(QApplication::translate("QZ_Launcher", "File map", Q_NULLPTR));
        tb_SelectFileMap->setText(QApplication::translate("QZ_Launcher", "...", Q_NULLPTR));
        groupBox_OPtions->setTitle(QApplication::translate("QZ_Launcher", "Options", Q_NULLPTR));
        chb_isWindow->setText(QApplication::translate("QZ_Launcher", "is Window", Q_NULLPTR));
        chb_NoMusic->setText(QApplication::translate("QZ_Launcher", "no music", Q_NULLPTR));
        chb_NoSounds->setText(QApplication::translate("QZ_Launcher", "no sounds", Q_NULLPTR));
        chb_NoOpenGL->setText(QApplication::translate("QZ_Launcher", "no OpenGL", Q_NULLPTR));
        chb_blandCU->setText(QApplication::translate("QZ_Launcher", "bland CPU", Q_NULLPTR));
        gb_Colors->setTitle(QApplication::translate("QZ_Launcher", "AI player", Q_NULLPTR));
        label_12->setText(QApplication::translate("QZ_Launcher", "Color:", Q_NULLPTR));
        rb_Color_Red->setText(QString());
        rb_Color_Blue->setText(QString());
        rb_Color_Green->setText(QString());
        rb_Color_Yellow->setText(QString());
        pb_PreviousSetting->setText(QApplication::translate("QZ_Launcher", "Previous Settings ", Q_NULLPTR));
        label_10->setText(QApplication::translate("QZ_Launcher", "Resolution", Q_NULLPTR));
        label_11->setText(QApplication::translate("QZ_Launcher", "X", Q_NULLPTR));
        pb_Save_Exit->setText(QApplication::translate("QZ_Launcher", "Save&Exit", Q_NULLPTR));
        pb_Exit->setText(QApplication::translate("QZ_Launcher", "Cancel", Q_NULLPTR));
        pb_Launch_Zod->setText(QApplication::translate("QZ_Launcher", "Launch Zod", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class QZ_Launcher: public Ui_QZ_Launcher {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QZ_LAUNCHER_H
