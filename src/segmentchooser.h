#ifndef SEGMENTCHOOSER_H
#define SEGMENTCHOOSER_H

#include <QDialog>

namespace Ui {
class SegmentChooser;
}

class SegmentChooser : public QDialog
{
    Q_OBJECT

public:
    explicit SegmentChooser(QWidget *parent = 0);
    ~SegmentChooser();
    void setChoices(QStringList choices) ;
    int choice() ;

private:
    Ui::SegmentChooser *ui;
};

#endif // SEGMENTCHOOSER_H
