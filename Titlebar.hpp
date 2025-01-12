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
#include <QDebug>
#include <QLabel>
#include <QMetaObject>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QSettings>
#include <QFontDatabase>
#include <QDebug>
#include <QLayout>
#include <QEvent>
#include <QMouseEvent>
#include <opencv4/opencv2/opencv.hpp>
enum class ButtonStatus
{
    maxBtn,
    minBtn,
    closeBtn,
};

class TitleBar : public QFrame
{
    Q_OBJECT
private:
    QPoint tmp_pos;
    QWidget *parent_widget;
    QWidget *band_widget;
    bool onBtn[3];

public:
    explicit TitleBar(QWidget *parent = nullptr);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void setBtnStatus(ButtonStatus btn, bool on);
    void updateBandWidget(QWidget *widget);
    void adjustBandWidget();
    bool eventFilter(QObject *watched, QEvent *event) override;
private Q_SLOTS:
    void on_close_clicked();
    void on_max_clicked();
    void on_min_clicked();
};
