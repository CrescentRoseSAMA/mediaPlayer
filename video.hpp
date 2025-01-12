#pragma once
#include "tqueue.hpp"
#include <thread>
#include <iostream>
#include <string>
#include <QImage>
#include <vector>
#include "Format_Print.hpp"
#include <QObject>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}
#define VIDEO_PKT_SIZE 20
#define Assert(expr)                                       \
    do                                                     \
    {                                                      \
        if (!(expr))                                       \
        {                                                  \
            printf(__CLEAR__                               \
                       __HIGHLIGHT__ __FRED__ #expr "\n"); \
            exit(-1);                                      \
        }                                                  \
    } while (0)
struct imgInfo
{
    QImage img;
    double pts = 0;
};
class videoState
{
public:
    /*成员变量*/
    int videoStreadIdx = -1;

    AVStream *pStream = nullptr;
    AVCodecContext *pCodecCtx = nullptr;

    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;
    uint8_t *rgbBuf = nullptr;

    SwsContext *pSwsCtx = nullptr;

    PacketQueue videoPktQueue;
    myThreadQueue<std::optional<imgInfo>> videoImgQueue;

    double videoClk = 0;
    double lastPts = 0;
    double lastDelay = 0;
    bool inited = false;

    /*成员函数*/
    videoState();
    ~videoState();
    void release();
    void decode();
    double synchronizePts(AVFrame *frame, double pts);
    void startDecode();
};