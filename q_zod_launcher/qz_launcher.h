#ifndef QZ_LAUNCHER_H
#define QZ_LAUNCHER_H

#include <QDialog>
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

#include "q_settings.h"


namespace Ui {
class QZ_Launcher;
}

class QZ_Launcher : public QDialog
{
    Q_OBJECT

public:
    explicit QZ_Launcher(QString ini_file_path, QWidget *parent = nullptr);
    ~QZ_Launcher();

private slots:
    void on_rb_Network_Nighsoft_clicked();
    void on_pb_Launch_Zod_clicked();
    void on_rb_Localame_clicked();
    void on_pb_Save_Exit_clicked();

protected:
    void init_control_interface();
    void load_params();
    void make_comand_line();
private:
    Ui::QZ_Launcher *ui;
    QString         _ini_path;
    QSettings       *_p_settings;
    QString         _str_command_line;
};

#endif // QZ_LAUNCHER_H
