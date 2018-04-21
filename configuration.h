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
    QString& configFolder() ;
    int exec() ;

private slots:
    void on_pushButton_clicked();

private:
    QString sKey, sFileName ;       // Google Key & file save folder
    QSettings *settings ;
    Ui::Configuration *ui;

};

#endif // CONFIGURATION_H
