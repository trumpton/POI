#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QDialog>
#include <QSettings>
#include <QDataStream>

namespace Ui {
class Configuration;
}

class Configuration : public QDialog
{
    Q_OBJECT

public:
    explicit Configuration(QWidget *parent = 0);
    ~Configuration();
    QString& key() ;

    QString& poiFolder() ;
    QString& tracksFolder() ;
    QString& importFolder() ;
    QString& garminFolder() ;
    QString& importFilter() ;
    bool importMove() ;

    QString& openFolder() ;
    void setOpenFolder(QString folder) ;

    int exec() ;

private slots:
    void on_pushButton_SearchPoi_clicked();
    void on_pushButton_SearchGarmin_clicked();
    void on_pushButton_SearchTracks_clicked();

    void on_pushButton_SearchImport_clicked();

private:
    QString sKey, sFileName, sGarmin, sTracks, sImportFolder, sOpenFolder , sImport, sFilter;       // Google Key & file save folder
    bool bMove ;
    QSettings *settings ;
    Ui::Configuration *ui;

};

#endif // CONFIGURATION_H
