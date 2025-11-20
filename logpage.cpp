#include "logpage.h"
#include "ui_logpage.h"

LogPage::LogPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogPage)
{
    ui->setupUi(this);
}

LogPage::~LogPage()
{
    delete ui;
}
