#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "yuvplayer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _player = new YuvPlayer(this);
    YuvFile in = {
        "/Users/caitou/Desktop/out.yuv",
        1280,
        720,
        AV_PIX_FMT_YUV420P,
    };
    _player->replace(in);
    _player->setGeometry(QRect(100, 0, 640, 640));

    _player->setAttribute(Qt::WA_StyledBackground);
    _player->setStyleSheet("background: red");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startPlayButton_clicked()
{

    _player->play();
}


void MainWindow::on_pauseButton_clicked()
{
    _player->pause();
}

