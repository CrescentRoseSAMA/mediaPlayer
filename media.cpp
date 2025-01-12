#include "media.hpp"
using namespace std;
mediaState::mediaState() : audio(outFormatFFMPEG, outFormatSDL)
{
}

mediaState::mediaState(string aPath) : audio(outFormatFFMPEG, outFormatSDL)
{
    this->filePath = aPath;
}

mediaState::~mediaState()
{
    if (pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
    }
}

void mediaState::openFile(string aPath)
{
    this->filePath = aPath;
}

void mediaState::init()
{
    Assert(avformat_open_input(&pFormatCtx, filePath.c_str(), nullptr, nullptr) == 0);
    Assert(avformat_find_stream_info(pFormatCtx, nullptr) >= 0);
    av_dump_format(pFormatCtx, 0, filePath.c_str(), 0);
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video.videoStreadIdx < 0)
        {
            video.videoStreadIdx = i;
            video.pStream = pFormatCtx->streams[i];
        }
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio.audioStreadIdx < 0)
        {
            audio.audioStreadIdx = i;
            audio.pStream = pFormatCtx->streams[i];
        }
    }
    Assert(video.videoStreadIdx >= 0);
    Assert(audio.audioStreadIdx >= 0);
    AVCodec *pVCodec = nullptr;
    /*视频流相关*/
    Assert(pVCodec = avcodec_find_decoder(video.pStream->codecpar->codec_id));
    Assert(video.pCodecCtx = avcodec_alloc_context3(pVCodec));
    Assert(avcodec_parameters_to_context(video.pCodecCtx, video.pStream->codecpar) >= 0);
    Assert(avcodec_open2(video.pCodecCtx, pVCodec, nullptr) == 0);
    video.pSwsCtx = sws_getContext(
        video.pCodecCtx->width,
        video.pCodecCtx->height,
        video.pCodecCtx->pix_fmt,
        video.pCodecCtx->width,
        video.pCodecCtx->height,
        AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    video.pFrame = av_frame_alloc();
    video.pFrameRGB = av_frame_alloc();
    int bufSz = av_image_get_buffer_size(AV_PIX_FMT_RGB24, video.pCodecCtx->width, video.pCodecCtx->height, 1);
    video.rgbBuf = (uint8_t *)av_malloc(bufSz);
    av_image_fill_arrays(video.pFrameRGB->data, video.pFrameRGB->linesize, video.rgbBuf, AV_PIX_FMT_RGB24, video.pCodecCtx->width, video.pCodecCtx->height, 1);
    video.inited = true;

    /*音频流相关*/
    AVCodec *pACodec = nullptr;
    Assert(pACodec = avcodec_find_decoder(audio.pStream->codecpar->codec_id));
    Assert(audio.pCodecCtx = avcodec_alloc_context3(pACodec));
    Assert(avcodec_parameters_to_context(audio.pCodecCtx, audio.pStream->codecpar) >= 0);
    Assert(avcodec_open2(audio.pCodecCtx, pACodec, nullptr) == 0);
    audio.pSwrCtx = swr_alloc();
    av_opt_set_int(audio.pSwrCtx, "in_channel_layout", audio.pCodecCtx->channel_layout, 0);
    av_opt_set_int(audio.pSwrCtx, "out_channel_layout", audio.pCodecCtx->channel_layout, 0);

    av_opt_set_int(audio.pSwrCtx, "in_sample_rate", audio.pCodecCtx->sample_rate, 0);
    av_opt_set_int(audio.pSwrCtx, "out_sample_rate", audio.pCodecCtx->sample_rate, 0);

    av_opt_set_sample_fmt(audio.pSwrCtx, "in_sample_fmt", audio.pCodecCtx->sample_fmt, 0);
    av_opt_set_sample_fmt(audio.pSwrCtx, "out_sample_fmt", outFormatFFMPEG, 0);
    Assert(swr_init(audio.pSwrCtx) >= 0);

    audio.pFrame = av_frame_alloc();
    audio.audioBuf = (uint8_t *)av_malloc(AUDIO_BUF_SIZE);
    audio.inited = true;
}

void mediaState::decodePack()
{
    AVPacket *pPkt = av_packet_alloc();
    int ret = 0;
    while (true)
    {
        ret = av_read_frame(pFormatCtx, pPkt);
        if (ret == AVERROR_EOF)
        {
            PrintInfo("数据包获取结束");
            break;
        }
        if (pPkt->stream_index == video.videoStreadIdx)
        {
            video.videoPktQueue.push(pPkt);
            av_packet_unref(pPkt);
        }
        else if (pPkt->stream_index == audio.audioStreadIdx)
        {
            audio.audioPktQueue.push(pPkt);
            av_packet_unref(pPkt);
        }
        else
        {
            av_packet_unref(pPkt);
        }
    }
    av_packet_unref(pPkt);
    audio.audioPktQueue.push(std::nullopt);
    video.videoPktQueue.push(std::nullopt);
    PrintInfo("数据包获取结束");
}

void mediaState::syncDisplay()
{
    while (true)
    {
        auto imgFrame = video.videoImgQueue.pop();
        if (!imgFrame)
        {
            break;
        }
        double pts = imgFrame.value().pts;
        double delay = pts - video.lastPts;
        if (delay <= 0 || delay >= 1.0)
        {
            delay = video.lastDelay;
        }
        video.lastDelay = delay;
        video.lastPts = pts;
        double audioClock = audio.getAudioClk();
        double diff = pts - audioClock;
        double syncThreshold = (delay > SYNC_THRESH) ? delay : SYNC_THRESH;
        if (fabs(diff) < NO_SYNC_THRESHOLD)
        {
            if (diff <= -syncThreshold)
            {
                delay = 0;
            }
            else if (diff >= syncThreshold)
            {
                delay = 2 * delay;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(int(delay * 1000)));
        updateImg(imgFrame.value());
    }
}

void mediaState::startPlay()
{
    audio.playAudio();
    video.startDecode();
    std::thread decodeThread([this]()
                             { this->decodePack(); });
    std::thread syncDisplayThread([this]()
                                  { this->syncDisplay(); });
    decodeThread.detach();
    syncDisplayThread.detach();
}
