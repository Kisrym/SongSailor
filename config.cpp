#include "config.h"
#include "ui_config.h"

config::config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::config)
{
    ui->setupUi(this);

    connect(ui->cancel, &QPushButton::clicked, this, [=]{
        ui->client_id->clear();
        ui->secret_id->clear();
        ui->refresh_token->clear();

        this->close();
    });
    connect(ui->send, &QPushButton::clicked, this, [=]{
        emit config_signal(ui->client_id->text(), ui->secret_id->text(), ui->refresh_token->text());
    });
}

config::~config()
{
    delete ui;
}
