#ifndef PHOTOIMPORTDIALOG_H
#define PHOTOIMPORTDIALOG_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
class PhotoImportDialog;
}

class PhotoImportDialog : public QDialog
{
    Q_OBJECT

private:
    void refresh() ;

public:
    explicit PhotoImportDialog(QWidget *parent = 0);
    ~PhotoImportDialog();

    void setTrackTimes(QDateTime from, QDateTime to) ;
    void setPhoto(QDateTime photo, QImage img) ;
    int offset() ;
    bool repeat() ;


private slots:
    void on_spinBox_adjustHours_valueChanged(int arg1);

private:
    Ui::PhotoImportDialog *ui;
    QDateTime phototime, trackfrom, trackto ;
};

#endif // PHOTOIMPORTDIALOG_H
