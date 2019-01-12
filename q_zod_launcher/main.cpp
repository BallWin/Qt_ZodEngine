// =========================================================
// *********************************************************
// =========================================================
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <iostream>


#include "qz_launcher.h"
#include "q_settings.h"
// =========================================================
// *********************************************************
// =========================================================

#ifdef Q_OS_UNIX
static QString ch_sep= "/";
#elif Q_OS_WIN
static QString ch_sep= R"("\")";
#endif
// =========================================================
// *********************************************************
// =========================================================
int main(int argc, char *argv[])
{
    std::cout<<"\n--\t"<<"$$$$$$$$$$$$  Start $$$$$$$$$$$$$$$$$$$$$$$$\n";
    QApplication a(argc, argv);

    QString str_data_folder = a.applicationDirPath();
    str_data_folder.append( DEF_FOLDER_DATA );

    QFileInfo f_Info(str_data_folder);
    str_data_folder= f_Info.canonicalFilePath();
    if( !make_default_folders_data() )
    {
        std::cerr<<"\n--<error> is not to make default folders a data!"<<std::endl;
    }

    if( !check_settings_file(str_data_folder + ch_sep + name_settings_file) )
    {
        std::cout<<std::endl<<(str_data_folder + ch_sep + name_settings_file).toStdString()
                <<std::endl;

        if( !make_ini_default(str_data_folder + ch_sep + name_settings_file)){
            std::cerr<<"\n--<error> Not find && not make settings file!\n\t"
                    << (str_data_folder + ch_sep + name_settings_file).toStdString()
                    <<std::endl;
            return -1;
        }
    }


    QZ_Launcher w(str_data_folder + ch_sep + name_settings_file);
    w.show();

    return a.exec();
}
// =========================================================
// *********************************************************
// =========================================================
