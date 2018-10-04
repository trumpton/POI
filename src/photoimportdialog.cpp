#include "photoimportdialog.h"
#include "ui_photoimportdialog.h"

#include <QPixmap>

PhotoImportDialog::PhotoImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PhotoImportDialog)
{
    ui->setupUi(this);
}

PhotoImportDialog::~PhotoImportDialog()
{
    delete ui;
}

void PhotoImportDialog::refresh()
{
    QDateTime displayphototime = phototime.addSecs(3600*ui->spinBox_adjustHours->value()) ;

    // Set Palette
    QPalette palette = ui->label_photo->palette();
    ui->label_photo->setAutoFillBackground(true);
    if (displayphototime>=trackfrom && displayphototime<=trackto) {
        // Set Colour Green
        palette.setColor(QPalette::WindowText, Qt::green);
        ui->label_photo->setPalette(palette);
    } else {
        // Set Colour Red
        palette.setColor(QPalette::WindowText, Qt::red);
        ui->label_photo->setPalette(palette);
        ui->label_photoTime->setPalette(palette);
    }
    // Set Text
    ui->label_photoTime->setText(displayphototime.toString()) ;
}

void PhotoImportDialog::setTrackTimes(QDateTime from, QDateTime to)
{
    trackfrom = from ;
    trackto = to ;
    ui->label_trackStartTime->setText(from.toString()) ;
    ui->label_trackEndTime->setText(to.toString()) ;
}

void PhotoImportDialog::setPhoto(QDateTime photo, QImage img)
{
    qint64 duration = ((trackfrom.secsTo(trackto)+3599)/3600) - 1 ;
    qint64 offset = (photo.secsTo(trackfrom))/3600 ;

    phototime = photo ;
    phototime.setTimeSpec(Qt::UTC);

    if (offset>-12 && offset<12) {
        ui->spinBox_adjustHours->setMinimum(offset) ;
        ui->spinBox_adjustHours->setMaximum(offset+duration);
        ui->spinBox_adjustHours->setValue(offset);
    } else {
        ui->spinBox_adjustHours->setMinimum(-2);
        ui->spinBox_adjustHours->setMaximum(2);
        ui->spinBox_adjustHours->setValue(0) ;
    }

    QPixmap pm ;
    pm.convertFromImage(img.scaled(ui->label_photo->width(),ui->label_photo->height())) ;
    ui->label_photo->setPixmap(pm);

    refresh() ;
}

int PhotoImportDialog::offset()
{
    return ui->spinBox_adjustHours->value() ;
}

bool PhotoImportDialog::repeat()
{
    return ui->checkBox_repeat->isChecked() ;
}

void PhotoImportDialog::on_spinBox_adjustHours_valueChanged(int arg1)
{
    Q_UNUSED(arg1) ;
    refresh() ;
}
