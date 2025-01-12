#include "video.hpp"
#include "finishTread.hpp"
using namespace std;
videoState::videoState() : videoPktQueue(VIDEO_PKT_SIZE), videoImgQueue(VIDEO_PKT_SIZE)
{
}

videoState::~videoState()
{
    release();
}

void videoState::release()
{
    inited = false;
    if (this->rgbBuf)
    {
        av_free(this->rgbBuf);
        rgbBuf = nullptr;
    }
    if (this->pFrame)
    {
        av_frame_free(&this->pFrame);
        pFrame = nullptr;
    }

    if (this->pFrameRGB)
    {
        av_frame_free(&this->pFrameRGB);
        pFrameRGB = nullptr;
    }

    if (this->pCodecCtx)
    {
        avcodec_free_context(&this->pCodecCtx);
        pCodecCtx = nullptr;
    }

    if (this->pSwsCtx)
    {
        sws_freeContext(this->pSwsCtx);
        pSwsCtx = nullptr;
    }
}

void videoState::decode()
{
    Assert(inited);
    double pts;
    while (true)
    {
        auto pack = videoPktQueue.pop();
        if (!pack)
        {
            break;
        }
        if (avcodec_send_packet(pCodecCtx, pack.value()) == 0)
        {
            if (avcodec_receive_frame(pCodecCtx, pFrame) == 0)
            {
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0,
                          pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                if ((pts = pFrame->best_effort_timestamp) == AV_NOPTS_VALUE)
                    pts = 0;
                pts *= av_q2d(pStream->time_base);
                pts = synchronizePts(pFrame, pts);
                imgInfo img{QImage(pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB888), pts};
                videoImgQueue.push(img);
            }
        }
    }
    videoImgQueue.push(std::nullopt);
}

double videoState::synchronizePts(AVFrame *frame, double pts)
{
    double frameDelay = 0;
    if (pts)
        videoClk = pts;
    else
        pts = videoClk;

    frameDelay = av_q2d(pStream->time_base);
    frameDelay += frame->repeat_pict * (frameDelay * 0.5);
    videoClk += frameDelay;
    return pts;
}

void videoState::startDecode()
{
    Assert(inited);
    std::thread decode([this]()
                       { this->decode(); });
    decode.detach();
}