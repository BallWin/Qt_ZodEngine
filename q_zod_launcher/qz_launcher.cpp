#include "qz_launcher.h"
#include "ui_qz_launcher.h"
#include <QComboBox>

QZ_Launcher::QZ_Launcher(QString ini_file_path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QZ_Launcher), _p_settings(nullptr)
{
    ui->setupUi(this);

    _ini_path = ini_file_path;
    _p_settings = new QSettings( _ini_path, QSettings::IniFormat );

    init_control_interface();
    load_params();
}

QZ_Launcher::~QZ_Launcher()
{
    delete ui;
    if(_p_settings != nullptr)
        delete _p_settings;
}
void QZ_Launcher::load_params()
{
//./zod -l map_list.txt -n zlover -t red -r 800x484 -b blue
// map_list.txt

    QString str_prm;
    // ----- AppGroup
    _p_settings->beginGroup( SETTING_AppGroup );
    str_prm = _p_settings->value(SETTING_AppPlayerName).toString();
    qDebug()<<"+++++++++++   str_prm= ["<<str_prm<<" ]";
    ui->le_PlayerName->setText(str_prm);
    _p_settings->endGroup();

    // ----- GameGroup -----------------------------------------
    _p_settings->beginGroup( SETTING_GameGroup );
    bool Ok;
    // :: isWindow or fullScreen
    Ok = (_p_settings->value(SETTING_GameModeView).toBool());
    ui->chb_isWindow->setChecked(Ok);
    // :: Music-ON Music-OFF
    Ok = (_p_settings->value(SETTING_GameMusic).toBool());
    ui->chb_NoMusic->setChecked(Ok);
    // :: Sound-ON Sound-OFF
    Ok = (_p_settings->value(SETTING_GameSound).toBool());
    ui->chb_NoSounds->setChecked(Ok);
    // :: OpenGL-ON OpenGL-OFF
    Ok = (_p_settings->value(SETTING_GameOpenGL).toBool());
    ui->chb_NoOpenGL->setChecked(Ok);
    // :: BlandCPU-ON BlandCPU-OFF
    Ok = (_p_settings->value(SETTING_GameBlandCPU).toBool());
    ui->chb_blandCU->setChecked(Ok);

    // ::  Resolution
    str_prm = QString::number(_p_settings->value(SETTING_GameResolutionW).toInt());
    ui->le_Resolution_width->setText(str_prm);
    str_prm = QString::number(_p_settings->value(SETTING_GameResolutionH).toInt());
    ui->le_Resolution_height->setText(str_prm);

    // ::  red blue green yellow
    str_prm = (_p_settings->value(SETTING_GameAIplayerColor).toString()).toLower();
    if( str_prm  == "red"  ) ui->rb_Color_Red->setChecked(true);
    else if(str_prm  == "green") ui->rb_Color_Green->setChecked(true);
    else if(str_prm  == "yellow") ui->rb_Color_Yellow->setChecked(true);
    else ui->rb_Color_Blue->setChecked(true);

    // :: multiplayer singleplayer
    str_prm = _p_settings->value(SETTING_GameModePlayer).toString();
    ui->rb_Localame->setChecked(str_prm == "singleplayer");
    ui->rb_Network_Nighsoft->setChecked(str_prm == "singleplayer");
    ui->gbox_ConnectSetting->setEnabled(str_prm == "singleplayer");
    ui->rb_Color_Red->setVisible(str_prm == "singleplayer");
    _p_settings->endGroup();

    // ----- NetworkGroup
    _p_settings->beginGroup( SETTING_NetworkGroup );
    str_prm = _p_settings->value(SETTING_NetworkAddres).toString();
    ui->le_ConnectAdress->setText(str_prm);
    str_prm = _p_settings->value(SETTING_NetworkLogin).toString();
    ui->le_LoginName->setText(str_prm);
    str_prm = _p_settings->value(SETTING_NetworkPWD).toString();
    ui->le_Password->setText(str_prm);
    str_prm = _p_settings->value(SETTING_NetworkTeam).toString();
    ui->cb_TeamColor->setItemText(0, str_prm );
    _p_settings->endGroup();

    // ----- MapsGroup -----------------------------------------
    _p_settings->beginGroup( SETTING_MapsGroup );
    QStringList ls;
    // ** all game campaing
    str_prm = _p_settings->value(SETTING_MapsCampaing).toString();
    ls = make_campaing_list();
    ui->cb_Maps_Campaing->addItem("none");
    for( QString folder_path : ls ){
        QFileInfo info(folder_path);
        if( info.exists() ){
            ui->cb_Maps_Campaing->addItem(info.baseName(),info.absoluteFilePath());
        }
    }
    if( !str_prm.isEmpty() )
    {
        int idx = ui->cb_Maps_Campaing->findText(str_prm);
        if(!(idx < 0 ))
            ui->cb_Maps_Campaing->setCurrentIndex(idx);
    }

    // ** all map sets
    ls.clear();
    str_prm = _p_settings->value(SETTING_MapsSet).toString();
    if( ui->rb_Localame->isChecked() )  ls = make_sets_map_list(true);
    else                                ls = make_sets_map_list(false);
    ls.push_front("none");
    if( !str_prm.isEmpty() ){
        for( int i=0; i< ls.size(); ++i )
        {
            if( ls.at(i) == str_prm ){
                ui->cb_Maps_SetMaps->setCurrentIndex(i);
                break;
            }
        }
    }

    // ** map for loaded
    str_prm = _p_settings->value(SETTING_MapsFile).toString();
    ui->le_FilePath_Map->setText(str_prm);

    // ** start index level (for campaing or set maps)
    str_prm = QString::number(_p_settings->value(SETTING_MapsIndexLevel).toInt());
    ui->le_StartLayer_Number->setText(str_prm);
    _p_settings->endGroup();

}



// ===============================================================
// ===============================================================
void QZ_Launcher::make_comand_line()
{

#ifdef Q_OS_WIN
    _str_command_line = ("zod_engine.exe");
#else
    _str_command_line = ("./zod");
#endif

}
// ===============================================================
void QZ_Launcher::init_control_interface()
{
    ui->cb_TeamColor->addItem("none");
    ui->cb_TeamColor->addItem("red");
    ui->cb_TeamColor->addItem("blue");
    ui->cb_TeamColor->addItem("green");
    ui->cb_TeamColor->addItem("yellow");
}


// ===============================================================
// ===============================================================
void QZ_Launcher::on_rb_Localame_clicked()
{
    ui->rb_Color_Blue->setChecked(true);
    ui->rb_Color_Red->setVisible(false);
    ui->gbox_ConnectSetting->setEnabled(false);
}
void QZ_Launcher::on_rb_Network_Nighsoft_clicked()
{
    ui->rb_Color_Red->setVisible(true);
    ui->gbox_ConnectSetting->setEnabled(true);

}

void QZ_Launcher::on_pb_Save_Exit_clicked()
{
    QString str_prm;
    // ----- AppGroup
    _p_settings->beginGroup( SETTING_AppGroup );
    _p_settings->setValue(SETTING_AppPlayerName,ui->le_PlayerName->displayText());
    _p_settings->endGroup();

    // ----- NetworkGroup
    _p_settings->beginGroup( SETTING_NetworkGroup );
    _p_settings->setValue(SETTING_NetworkAddres,ui->le_ConnectAdress->displayText());
    _p_settings->setValue(SETTING_NetworkLogin,ui->le_LoginName->displayText());
    _p_settings->setValue(SETTING_NetworkPWD,ui->le_Password->displayText());
    _p_settings->setValue(SETTING_NetworkTeam, ui->cb_TeamColor->currentText());
    _p_settings->endGroup();


    // ----- MapsGroup -----------------------------------------
    _p_settings->beginGroup( SETTING_MapsGroup );
    // ** all game campaing
    QStringList ls;
    int sz= ui->cb_Maps_Campaing->count();
    for(int i=0; i<sz; ++i ){
        ui->cb_Maps_Campaing->setCurrentIndex(i);
        ls.append( ui->cb_Maps_Campaing->currentText());
    }
    str_prm = ls.join("|");
    _p_settings->setValue(SETTING_MapsCampaing, str_prm );
    ls.clear();

    // ** all map sets
    sz= ui->cb_Maps_SetMaps->count();
    for(int i=0; i<sz; ++i ){
        ui->cb_Maps_SetMaps->setCurrentIndex(i);
        ls.append( ui->cb_Maps_SetMaps->currentText());
    }
    str_prm = ls.join("|");
    _p_settings->setValue(SETTING_MapsSet, str_prm );
    ls.clear();

    // ** map for loaded
    _p_settings->setValue(SETTING_MapsFile,ui->le_FilePath_Map->displayText());

    // ** start index level (for campaing or set maps)
    _p_settings->setValue(SETTING_MapsIndexLevel,ui->le_StartLayer_Number->displayText().toInt());
    _p_settings->endGroup();


    // ----- GameGroup -----------------------------------------
    _p_settings->beginGroup( SETTING_GameGroup );
    bool Ok;
    // :: isWindow or fullScreen
    Ok = (ui->chb_isWindow->checkState()==Qt::Checked);
    _p_settings->setValue(SETTING_MapsIndexLevel, Ok);
    // :: Music-ON Music-OFF
    Ok = (ui->chb_NoMusic->checkState()==Qt::Checked);
    _p_settings->setValue(SETTING_GameMusic, Ok);
    // :: Sound-ON Sound-OFF
    Ok = (ui->chb_NoSounds->checkState()==Qt::Checked);
    _p_settings->setValue(SETTING_GameSound, Ok);
    // :: OpenGL-ON OpenGL-OFF
    Ok = (ui->chb_NoOpenGL->checkState()==Qt::Checked);
    _p_settings->setValue(SETTING_GameOpenGL, Ok);
    // :: BlandCPU-ON BlandCPU-OFF
    Ok = (ui->chb_blandCU->checkState()==Qt::Checked);
    _p_settings->setValue(SETTING_GameBlandCPU, Ok);

    // ::  Resolution
    _p_settings->setValue(SETTING_GameResolutionW,ui->le_Resolution_width->displayText().toInt());
    _p_settings->setValue(SETTING_GameResolutionH,ui->le_Resolution_height->displayText().toInt());

    // ::  red blue creen yellow
    if(ui->rb_Color_Red->isChecked())           str_prm= "red";
    else if(ui->rb_Color_Blue->isChecked())     str_prm= "blue";
    else if(ui->rb_Color_Green->isChecked())    str_prm= "green";
    else if(ui->rb_Color_Yellow->isChecked())   str_prm= "yellow";
    _p_settings->setValue(SETTING_GameAIplayerColor, str_prm);

    // :: multiplayer singleplayer
    str_prm= "multiplayer";
    if(ui->rb_Localame->isChecked())
        str_prm= "singleplayer";
    _p_settings->setValue(SETTING_GameModePlayer, str_prm);

    close();
}

void QZ_Launcher::on_pb_Launch_Zod_clicked()
{
    QString command_str = ui->le_command_line_setting->displayText();
}




