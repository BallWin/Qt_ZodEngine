#ifndef Q_SETTINGS_H
#define Q_SETTINGS_H
// =========================================================
// *********************************************************
// =========================================================
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QIODevice>
#include <QDebug>
#include <QUrl>

#include <iostream>
// =========================================================
// *********************************************************
// =========================================================
#ifdef Q_OS_UNIX
#define SH_SEP                  "/"
#define DEF_FOLDER_DATA         "/../Data"
#define DEF_FOLDER_SAVE         "/../Data/save"
#define DEF_FOLDER_CAMPAING     "/../Data/Campaing"
#define DEF_FOLDER_SETS         "/../Data/SetMaps"
#define DEF_FOLDER_MAPS         "/../Data/Maps"
#elif Q_OS_WIN
#define SH_SEP                  R"("\")"
#define DEF_FOLDER_DATA         R"("\..\Data")"
#define DEF_FOLDER_SAVE         R"("\..\Data\save")"
#define DEF_FOLDER_CAMPAING     R"("\..\Data\Campaing")"
#define DEF_FOLDER_SETS         R"("\..\Data\SetMaps")"
#define DEF_FOLDER_MAPS         R"("\..\Data\Maps")"
#endif

static QString name_settings_file = "settings_zod.ini";
static QString name_folder_singleplayer = "SinglePlayer";
static QString name_folder_multiplayer = "MultiPlayer";
static QString name_folder_mp = "mp";
static QString name_folder_mpAI = "mp_and_AI";
static QString name_reference_maps= "reference.txt";

// ----------------------------------------
inline bool check_file_exist( QString str_file_path );
inline bool check_settings_file( QString ini_path );
inline bool make_default_folders_data();
inline QStringList make_campaing_list();
inline QStringList make_sets_map_list(bool f_singleplayer );
inline QStringList get_files_list(QString folder_path, QString filter_str= QString() );

inline bool make_ini_default( QString ini_file_name, QString folder_data = QString() );




// =========================================================
// *********************************************************
// =========================================================
#ifndef SETTING_AppGroup
#define SETTING_AppGroup                    "ZodLauncher"
#endif
#ifndef SETTING_NetworkGroup
#define SETTING_NetworkGroup                "Connection"
#endif
#ifndef SETTING_MapsGroup
#define SETTING_MapsGroup                   "MapsGroup"
#endif
#ifndef SETTING_GameGroup
#define SETTING_GameGroup                   "GameGroup"
#endif

// -- keys
// ----- AppGroup
#ifndef SETTING_AppComment
#define SETTING_AppComment                  "AppComment"
#endif
#ifndef SETTING_AppPlayerName
#define SETTING_AppPlayerName               "PlayerName"
#endif
#ifndef SETTING_AppData
#define SETTING_AppData                     "AppData"
#endif
// ----- NetworkGroup
#ifndef SETTING_NetworkAddres
#define SETTING_NetworkAddres               "Addres"
#endif
#ifndef SETTING_NetworkLogin
#define SETTING_NetworkLogin                "Login"
#endif
#ifndef SETTING_NetworkPWD
#define SETTING_NetworkPWD                  "Password"
#endif
#ifndef SETTING_NetworkTeam
#define SETTING_NetworkTeam                 "Team"
#endif

// ----- MapsGroup
#ifndef SETTING_MapsCampaign
#define SETTING_MapsCampaing                "Campaign"
#endif
#ifndef SETTING_MapsSet
#define SETTING_MapsSet                     "SetMaps"
#endif
#ifndef SETTING_MapsFile
#define SETTING_MapsFile                    "FileMap"
#endif
#ifndef SETTING_MapsIndexLevel
#define SETTING_MapsIndexLevel              "IndexLevel"
#endif

// ----- GameGroup
#ifndef SETTING_GameModePlayer
#define SETTING_GameModePlayer              "ModePlayer"
#endif

#ifndef SETTING_GameModeView
#define SETTING_GameModeView                "ModeView"
#endif
#ifndef SETTING_GameMusic
#define SETTING_GameMusic                   "Music"
#endif
#ifndef SETTING_GameSound
#define SETTING_GameSound                   "Sound"
#endif
#ifndef SETTING_GameOpenGL
#define SETTING_GameOpenGL                  "OpenGL"
#endif
#ifndef SETTING_GameBlandCPU
#define SETTING_GameBlandCPU                "BlandCPU"
#endif
#ifndef SETTING_GameResolutionW
#define SETTING_GameResolutionW             "ResolutionW"
#endif
#ifndef SETTING_GameResolutionH
#define SETTING_GameResolutionH             "ResolutionH"
#endif
#ifndef SETTING_GameAIplayerColor
#define SETTING_GameAIplayerColor           "AI_player_color"
#endif

// ----------------------------------------
inline bool check_file_exist( QString str_file_path )
{
    QFileInfo info( str_file_path );
    return info.exists();
}
// ----------------------------------------
inline  bool check_settings_file( QString ini_path )
{
    return check_file_exist(ini_path);
}
// ----------------------------------------
inline bool make_default_folders_data()
{
    QString str_sep= SH_SEP;
    QString app_path= QApplication::applicationDirPath();
    QString str_path= app_path + DEF_FOLDER_DATA;
    QDir dir_data;
    dir_data.setPath( str_path );
    if( !dir_data.exists() && !dir_data.mkdir(str_path) ){
        std::cerr<<"\n--<error>\t not make folder data\n\t"<<str_path.toStdString()<<std::endl;
        return false;
    }
    QStringList ls_dir;
    ls_dir.append(app_path + DEF_FOLDER_SAVE);
    ls_dir.append(app_path + DEF_FOLDER_CAMPAING);
    ls_dir.append(app_path + DEF_FOLDER_SETS);
    ls_dir.append(app_path + DEF_FOLDER_MAPS);
    ls_dir.append(app_path + DEF_FOLDER_SETS+str_sep+ name_folder_singleplayer);
    ls_dir.append(app_path + DEF_FOLDER_SETS+str_sep+ name_folder_multiplayer);
    ls_dir.append(app_path + DEF_FOLDER_MAPS+str_sep+ name_folder_singleplayer);
    ls_dir.append(app_path + DEF_FOLDER_MAPS+str_sep+ name_folder_multiplayer);
    ls_dir.append(app_path + DEF_FOLDER_MAPS+str_sep+ name_folder_multiplayer
                  +str_sep+ name_folder_mp);
    ls_dir.append(app_path + DEF_FOLDER_MAPS+str_sep+ name_folder_multiplayer
                  +str_sep+ name_folder_mpAI);
    ls_dir.append(app_path + DEF_FOLDER_SETS+str_sep+ name_folder_multiplayer);
    ls_dir.append(app_path + DEF_FOLDER_SETS+str_sep+ name_folder_multiplayer
                  +str_sep + name_folder_mp);
    ls_dir.append(app_path + DEF_FOLDER_SETS+str_sep+ name_folder_multiplayer
                  +str_sep + name_folder_mpAI);
    for( QString str : ls_dir ){
        dir_data.setPath( str );
        if( !dir_data.exists() && !dir_data.mkdir(str) )
         std::cerr<<"\n--<error>\t not make folder data\n\t"<<str.toStdString()<<std::endl;
    }
    str_path=(app_path + DEF_FOLDER_SETS+str_sep+ name_folder_singleplayer);
    dir_data.setPath( str_path );
    if( !dir_data.exists() && !dir_data.mkdir(str_path) ){
        std::cerr<<"\n--<error>\t not make folder data\n\t"<<str_path.toStdString()<<std::endl;
    }
    return true;
}
// ----------------------------------------
inline QStringList make_campaing_list()
{
    QStringList ls;
    QString app_path = QApplication::applicationDirPath();
    QString str_path= app_path + DEF_FOLDER_CAMPAING;
    QDir dir(str_path);
    if( !dir.exists() )
        std::cerr<<"\n--<error>\t not exists folder \n\t"<<str_path.toStdString()<<std::endl;
    else{
        QFileInfoList fi_ls = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        if( !fi_ls.empty() ){
            for(QFileInfo &info : fi_ls )
                ls.append( info.absoluteFilePath());
        }
    }
    return ls;
}
inline QStringList make_sets_map_list( bool f_singleplayer )
{
    QStringList ls;
    QStringList ls_folders;
    QString str_sep= SH_SEP;
    QString app_path = QApplication::applicationDirPath();
    QString str_path= app_path + DEF_FOLDER_SETS+str_sep;
    if( f_singleplayer ) {
        str_path.append( name_folder_singleplayer);
    }
    else {
        str_path.append( name_folder_multiplayer);
        ls_folders.append(str_path);
        ls_folders.append(str_path + str_sep + name_folder_mp);
        ls_folders.append(str_path + str_sep + name_folder_mpAI);
    }
    for(QString path : ls_folders)
    {
        ls = get_files_list(str_path, "txt");
    }
    return ls;
}
inline QStringList get_files_list(QString folder_path, QString filter_extension)
{
    QStringList ls;
    QStringList ls_ext;
    QDir dir(folder_path);
    if( dir.exists()){
        if(!filter_extension.isEmpty())
            ls_ext= filter_extension.split("|");
        QFileInfoList fi_ls = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for(QFileInfo &info : fi_ls ){
            if( !ls_ext.empty() ){
                for( QString extension : ls_ext ){
                    if( info.suffix() == extension ){
                        ls.append( info.absoluteFilePath());
                        break;
                    }
                }
            }
            else {
                ls.append( info.absoluteFilePath());
            }
        }
    }
    return ls;
}

// ----------------------------------------
inline  bool make_ini_default( QString ini_file_name, QString folder_data )
{
    if( folder_data.isEmpty() )
        folder_data = QApplication::applicationDirPath() + QString(DEF_FOLDER_DATA);
    QFileInfo f_Info(folder_data);
    folder_data= f_Info.canonicalFilePath();
    if( folder_data.isEmpty() )
        qDebug()<<"\n--<warning>\t empty path io folder data!\n";


    QSettings sts( ini_file_name, QSettings::IniFormat );



    // ----- AppGroup -----------------------------------------
    sts.beginGroup( SETTING_AppGroup );
    sts.setValue( SETTING_AppComment, "==== "+ QApplication::applicationName()+" ===="  );
    sts.setValue( SETTING_AppData, folder_data );
    sts.setValue( SETTING_AppPlayerName, "Player_One" );
    sts.endGroup();

    // ----- NetworkGroup -----------------------------------------
    sts.beginGroup( SETTING_NetworkGroup );
    sts.setValue( SETTING_NetworkAddres, "" );
    sts.setValue( SETTING_NetworkLogin, "" );
    sts.setValue( SETTING_NetworkPWD, "" );
    sts.setValue( SETTING_NetworkTeam, "" );
    sts.endGroup();

    // ----- MapsGroup -----------------------------------------
    sts.beginGroup( SETTING_MapsGroup );
    sts.setValue( SETTING_MapsCampaing, "" );
    sts.setValue( SETTING_MapsSet, "" );
    sts.setValue( SETTING_MapsFile, "" );
    sts.setValue( SETTING_MapsIndexLevel, 1 );
    sts.endGroup();


    // ----- GameGroup -----------------------------------------
    sts.beginGroup( SETTING_GameGroup );
    // :: multiplayer singleplayer
    sts.setValue( SETTING_GameModePlayer, "singleplayer" );
    // :: isWindow fullScreen
    sts.setValue( SETTING_GameModeView, "isWindow" );
    // :: Music-ON Music-OFF
    sts.setValue( SETTING_GameMusic, "Music-ON" );
    // :: Sound-ON Sound-OFF
    sts.setValue( SETTING_GameSound, "Sound-ON" );
    // :: OpenGL-ON OpenGL-OFF
    sts.setValue( SETTING_GameOpenGL, "OpenGL-OFF" );
    // :: BlandCPU-ON BlandCPU-OFF
    sts.setValue( SETTING_GameBlandCPU, "BlandCPU-OFF" );

    // ::  Resolution
    sts.setValue( SETTING_GameResolutionW, 720 );
    sts.setValue( SETTING_GameResolutionH, 860 );

    // ::  red blue creen yellow
    sts.setValue( SETTING_GameAIplayerColor, "blue" );
    sts.endGroup();


    return true;
}




// =========================================================
// *********************************************************
// =========================================================
#endif // Q_SETTINGS_H
