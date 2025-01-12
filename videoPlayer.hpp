#pragma once
#include <iostream>
#include <QMainWindow>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QTextEdit>
#include <QApplication>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QMetaObject>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QMouseEvent>
#include <QProgressBar>
#include <QCloseEvent>
#include <QSettings>
#include <QFontDatabase>
#include <QDebug>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <opencv4/opencv2/opencv.hpp>
#include "media.hpp"
#include "Titlebar.hpp"

class videoPlayer : public QWidget
{
    Q_OBJECT
private:
    QSize minSize;
    QSize maxSize;
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    QPushButton *selectButton;
    QFrame *videoFrame;
    TitleBar *titleBar;
    QFrame *buttonBar;
    QMenuBar *menuBar;
    QProgressBar *progressBar;
    QRect showAera;
    QImage frame;
    QTimer timer;
    mediaState mediaPlayer;
    bool buttonBarVisible = true;

public:
    videoPlayer(QWidget *parent = nullptr);
    ~videoPlayer();
    void setVideo(std::string path);
    void updateProgressBar(int value);
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private Q_SLOTS:
    void on_select_clicked();
    void on_s05_triggered();
    void on_s1_triggered();
    void on_s2_triggered();
    void on_timer_timeout();
    void m_show(imgInfo img);
};

class mainWindow : public QWidget
{
    Q_OBJECT
private:
    videoPlayer *player;
    TitleBar *titleBar;
    QTimer *timer;
    int count = 0;
    bool isTitleBarVisible = true;

public:
    mainWindow(QWidget *parent = nullptr)
    {
        timer = new QTimer(this);
        player = new videoPlayer(this);
        titleBar = new TitleBar(this);
        this->setGeometry(0, 0, player->geometry().width(), player->geometry().height() + 60);
        player->move(0, 60);
        setWindowFlags(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        titleBar->setFixedHeight(60);
        // titleBar->setBtnStatus(ButtonStatus::maxBtn, false);
        titleBar->updateBandWidget(player);
        player->setMouseTracking(true);
        this->setMouseTracking(true);
        timer->start(100);
        connect(timer, &QTimer::timeout, this, &mainWindow::Timeout);
    }
    void resizeEvent(QResizeEvent *event)
    {
        double scaleW = (double)event->size().width() / event->oldSize().width();
        double scaleH = (double)event->size().height() / event->oldSize().height();
        player->resize(player->width() * scaleW, player->height() * scaleH);
    }
    void Timeout()
    {
        count += 100;
        if (count > 3000)
        {
            count = 0;
            this->titleBar->hide();
            isTitleBarVisible = false;
        }
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        if (!isTitleBarVisible)
        {
            this->titleBar->show();
            isTitleBarVisible = true;
        }
        QWidget::mouseMoveEvent(event);
    }
};