#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui {
class MainWindow;
}
class SharedObjectCli;
class SharedObjectSrv;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_client_btn_connect_clicked();

    void on_server_btn_start_clicked();

    void on_client_btn_set_clicked();

    void on_client_btn_get_clicked();

private:
    Ui::MainWindow *ui;
    SharedObjectCli* client_;
    SharedObjectSrv* server_;

};

#endif // MAINWINDOW_H
