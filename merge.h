#ifndef MERGE_H
#define MERGE_H

#include <QDialog>

namespace Ui {
class Merge;
}

class Merge : public QDialog
{
    Q_OBJECT

public:
    explicit Merge(QWidget *parent = 0);
    ~Merge();

    void clear() ;
    void addItem(QString filename, QString filepath) ;
    QString &currentData() ;

private slots:
    void on_comboBox_Files_currentIndexChanged(int index);

private:
    Ui::Merge *ui;
    QString selected ;
};

#endif // MERGE_H
