#include "segmentchooser.h"
#include "ui_segmentchooser.h"

SegmentChooser::SegmentChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SegmentChooser)
{
    ui->setupUi(this);
}

SegmentChooser::~SegmentChooser()
{
    delete ui;
}

void SegmentChooser::setChoices(QStringList choices)
{
    ui->comboBox_segment->clear() ;
    ui->comboBox_segment->addItem("All Track Segments");
    ui->comboBox_segment->addItems(choices) ;
}

int SegmentChooser::choice()
{
    return ui->comboBox_segment->currentIndex() ;
}
