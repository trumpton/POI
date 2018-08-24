#include "prompt.h"
#include "ui_prompt.h"

Prompt::Prompt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Prompt)
{
    ui->setupUi(this);
}

Prompt::~Prompt()
{
    delete ui;
}

void Prompt::setup(QString title, QString prompt, QString hint)
{
    this->setWindowTitle(title) ;
    ui->prompt->setText(prompt) ;
    ui->hint->setText(hint) ;
}

QString Prompt::text()
{
    return ui->lineEdit->text() ;
}

