#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>

namespace Ui {
class config;
}

class config : public QDialog
{
    Q_OBJECT

public:
    explicit config(QWidget *parent = nullptr);
    ~config();

signals:
    void config_signal(QString client_id, QString secret_id, QString refresh_token);

private:
    Ui::config *ui;
};

#endif // CONFIG_H
