#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sharedobject.h>
#include <QDebug>
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    client_ = new SharedObjectCli();
    server_ = new SharedObjectSrv();
}

MainWindow::~MainWindow()
{
    delete ui;
    if(server_){delete server_;}
    if(client_){delete client_;}
}

void MainWindow::on_client_btn_connect_clicked()
{
    QString ip = ui->client_ip->text().trimmed();
    QString port = ui->client_port->text().trimmed();

    if(ip== ""){ip = "127.0.0.1";};
    if(port== ""){port = "10086";};
    try{
        client_->connect(("tcp://" +ip).toLatin1().data(), port.toInt());
        client_->sync();
        qDebug() << "connected to " + ip + ":" + port;
    }catch(void*){
        qDebug() << "error on client connect ";
    }

}

void MainWindow::on_server_btn_start_clicked()
{
    QString port = ui->server_port->text().trimmed();
    if(port== ""){port = "10086";}
    std::thread *t = new std::thread([this,port](){
        try{
            this->server_->bind("tcp://0.0.0.0", port.toInt() );
            qDebug() << "bind to " + port;
        }catch(void*){
            qDebug() << "error on server bind ";
        }
        while(1){
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    });

}

void MainWindow::on_client_btn_set_clicked()
{
    QString key = ui->client_set_key->text().trimmed();
    QString val = ui->client_set_value->text().trimmed();
    int rc = client_->set(key.toLatin1().data(),val.toLatin1().data());
    qDebug()<<"rc = " << rc << key <<":" << val;
}

void MainWindow::on_client_btn_get_clicked()
{
    QString key = ui->client_get_key->text().trimmed();
    std::string val;
    int rc = client_->get(key.toLatin1().data(), val);
    qDebug() <<"rc = " << rc << key <<":" << val.c_str();
    ui->client_get_value->setText(QString::fromStdString(val));
}
