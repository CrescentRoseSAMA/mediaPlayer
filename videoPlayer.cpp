#include "videoPlayer.hpp"
using namespace std;
int mouseTime = 0;
string path = "../mad.mp4";
int hourPerSec = 60 * 60;
int minutePerSec = 60;
videoPlayer::videoPlayer(QWidget *parent) : QWidget(parent), minSize(450, 450), maxSize(1870, 1030), showAera(0, 0, 500, 500)
{
    setMaximumSize(maxSize);
    setMinimumSize(minSize);
    // this->setStyleSheet("background-color:rgba(125, 125, 125, 100);");
    playButton = new QPushButton();
    playButton->setFixedSize(50, 50);
    playButton->setObjectName("play");
    pauseButton = new QPushButton();
    pauseButton->setFixedSize(50, 50);
    pauseButton->setObjectName("pause");
    stopButton = new QPushButton();
    stopButton->setObjectName("stop");
    stopButton->setFixedSize(50, 50);
    selectButton = new QPushButton();
    selectButton->setFixedSize(50, 50);
    selectButton->setObjectName("select");

    QHBoxLayout *bHlayout = new QHBoxLayout();
    QVBoxLayout *bVlayout = new QVBoxLayout();
    QVBoxLayout *vlayout = new QVBoxLayout();
    progressBar = new QProgressBar();
    progressBar->setFixedHeight(20);
    progressBar->setObjectName("progressBar");
    buttonBar = new QFrame();
    buttonBar->setObjectName("bottomBar");
    buttonBar->setFixedHeight(70);
    bHlayout->setContentsMargins(0, 0, 0, 0);
    bHlayout->addWidget(playButton, 0, Qt::AlignCenter);
    bHlayout->addWidget(pauseButton, 0, Qt::AlignCenter);
    bHlayout->addWidget(selectButton, 0, Qt::AlignCenter);
    bVlayout->setContentsMargins(0, 0, 0, 0);
    bVlayout->setSpacing(0);
    bVlayout->addWidget(progressBar);
    bVlayout->addLayout(bHlayout);
    buttonBar->setLayout(bVlayout);
    vlayout->addWidget(buttonBar, 0, Qt::AlignBottom);
    this->setLayout(vlayout);
    menuBar = new QMenuBar(this);
    menuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto menu = menuBar->addMenu("speed");
    auto action0 = menu->addAction("0.5x");
    action0->setObjectName("s05");
    auto action1 = menu->addAction("1x");
    action1->setObjectName("s1");
    auto action2 = menu->addAction("2x");
    action2->setObjectName("s2");
    timer.setObjectName("timer");
    timer.start(100);
    connect(&timer, &QTimer::timeout, this, &videoPlayer::on_timer_timeout);
    connect(&mediaPlayer, &mediaState::updateImg, this, &videoPlayer::m_show);
    // this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setWindowFlags(Qt::FramelessWindowHint);
    QMetaObject::connectSlotsByName(this);
}

videoPlayer::~videoPlayer()
{
}

void videoPlayer::setVideo(string path)
{
    mediaPlayer.openFile(path);
    mediaPlayer.init();
    mediaPlayer.startPlay();
}

void videoPlayer::on_select_clicked()
{
    string path = QFileDialog::getOpenFileName(this, "Open Video", "../", "Video Files (*.mp4 *.avi *.flv *.mkv *.wmv)").toStdString();
    setVideo(path);
}
void videoPlayer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!frame.isNull())
    {
        QImage scaledFrame = frame.scaled(showAera.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int x = showAera.x() + (showAera.width() - scaledFrame.width()) / 2;
        int y = showAera.y() + (showAera.height() - scaledFrame.height()) / 2;

        painter.drawImage(x, y, scaledFrame);
    }
}

void videoPlayer::m_show(imgInfo img)
{
    frame = img.img;
    double pts = img.pts;
    updateProgressBar(pts);
    update();
}

void videoPlayer::resizeEvent(QResizeEvent *event)
{
    showAera = QRect(0, 0, event->size().width(), event->size().height());
    update();
    QWidget::resizeEvent(event);
}

void videoPlayer::on_s05_triggered()
{
}

void videoPlayer::on_s1_triggered()
{
}

void videoPlayer::on_s2_triggered()
{
}

void videoPlayer::on_timer_timeout()
{
    mouseTime += 100;
    if (mouseTime > 3000 && buttonBarVisible)
    {
        this->setCursor(Qt::BlankCursor);
        this->buttonBar->hide();
        this->menuBar->hide();
        buttonBarVisible = false;
    }
}

void videoPlayer::mouseMoveEvent(QMouseEvent *event)
{
    mouseTime = 0;
    if (!buttonBarVisible)
    {
        this->setCursor(Qt::ArrowCursor);
        this->buttonBar->show();
        this->menuBar->show();
        buttonBarVisible = true;
    }
    QWidget::mouseMoveEvent(event);
}

void videoPlayer::updateProgressBar(int value)
{
    int second = value % minutePerSec;
    int minute = (value - second) / 60.0f;
    int hour = static_cast<int>(value / static_cast<double>(hourPerSec));
    QString timeStr = QString("%1:%2:%3").arg(hour, 2).arg(minute, 2).arg(second, 2);
    progressBar->setFormat(timeStr);
    progressBar->setValue(value);
}