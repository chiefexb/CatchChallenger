#include "AddServer.h"
#include "ui_AddServer.h"

#include <QMessageBox>

AddServer::AddServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddServer)
{
    ui->setupUi(this);
    ok=false;
}

AddServer::~AddServer()
{
    delete ui;
}

void AddServer::on_ok_clicked()
{
    if(ui->name->text()=="Internal" || ui->name->text()=="internal")
    {
        QMessageBox::warning(this,tr("Error"),tr("The name can't be \"internal\""));
        return;
    }
    ok=true;
    close();
}

QString AddServer::server() const
{
    return ui->server->text();
}

quint16 AddServer::port() const
{
    return ui->port->value();
}

QString AddServer::proxyServer() const
{
    if(!ui->proxy->isChecked())
        return QString();
    return ui->proxyServer->text();
}

quint16 AddServer::proxyPort() const
{
    return ui->proxyPort->value();
}

QString AddServer::name() const
{
    return ui->name->text();
}

bool AddServer::isOk() const
{
    return ok;
}
