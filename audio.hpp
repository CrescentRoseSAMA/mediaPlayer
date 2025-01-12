#pragma once
#include <iostream>
#include <thread>
#include <SDL2/SDL.h>
#include <vector>
#include "tqueue.hpp"
#include "Format_Print.hpp"
#include <SDL2/SDL_thread.h>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

#define AUDIO_PKT_SIZE 20
#define AUDIO_BUF_SIZE 192000
#define SDL_AUDIO_BUFFER_SIZE 2048
struct audioState
{
    /*成员函数*/
    const int outSampleRate = 44100;
    const AVSampleFormat outFormatFFMPEG = AV_SAMPLE_FMT_S16;
    const int outFormatSDL = AUDIO_S16SYS;

    int audioStreadIdx = -1;
    double audioClk = 0;

    SwrContext *pSwrCtx = nullptr;
    AVStream *pStream = nullptr;
    AVCodecContext *pCodecCtx = nullptr;

    uint8_t *audioCvtBuf = nullptr;
    int audioCvtBufSz = 0;
    AVFrame *pFrame = nullptr;

    uint8_t *audioBuf = nullptr;
    int audioBufSz = 0;
    int audioBufIdx = 0;

    PacketQueue audioPktQueue;

   
    bool inited = false;
    /*成员变量*/

    audioState();
    audioState(AVSampleFormat aOutFormatFFMPEG, int aOutFormatSDL);
    ~audioState();
    void release();
    void playAudio();
    double getAudioClk();
    int decode();
};

void sdlCallBack(void *userdata, Uint8 *stream, int len);