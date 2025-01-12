#pragma once
#include "video.hpp"
#include "audio.hpp"
#include "Format_Print.hpp"
#define SYNC_THRESH 0.01
#define NO_SYNC_THRESHOLD 10

const AVSampleFormat outFormatFFMPEG = AV_SAMPLE_FMT_S16;
const int outFormatSDL = AUDIO_S16SYS;
class mediaState : public QObject
{
    Q_OBJECT
public:
    /*成员变量*/
    videoState video;
    audioState audio;
    AVFormatContext *pFormatCtx = nullptr;
    std::string filePath;

public:
    /*成员函数*/
    mediaState();
    mediaState(std::string aPath);
    ~mediaState();
    void openFile(std::string aPath);
    void init();
    /*播放相关*/
    void decodePack();
    void syncDisplay();
    void startPlay();
    /*功能相关*/
    void setPlaySpeed(double speed);
    void pauseVideo();
    void resumeVideo();
Q_SIGNALS:
    void updateImg(imgInfo img);
};