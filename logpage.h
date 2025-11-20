#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QWidget>

namespace Ui {
class LogPage;
}

class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

private:
    Ui::LogPage *ui;
};

#endif // LOGPAGE_H
