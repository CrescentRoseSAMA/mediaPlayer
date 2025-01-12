#include "audio.hpp"

audioState::audioState() : audioPktQueue(AUDIO_PKT_SIZE)
{
}

audioState::audioState(AVSampleFormat aOutFormatFFMPEG, int aOutFormatSDL) : audioPktQueue(AUDIO_PKT_SIZE), outFormatFFMPEG(aOutFormatFFMPEG), outFormatSDL(aOutFormatSDL)
{
}

audioState::~audioState()
{
    release();
}

void audioState::release()
{
    inited = false;
    if (this->audioCvtBuf)
    {
        av_free(this->audioCvtBuf);
        audioCvtBuf = nullptr;
    }
    if (this->audioBuf)
    {
        av_free(this->audioBuf);
        audioBuf = nullptr;
    }
    if (this->pFrame)
    {
        av_frame_free(&this->pFrame);
        pFrame = nullptr;
    }

    if (this->pCodecCtx)
    {
        avcodec_free_context(&this->pCodecCtx);
        pCodecCtx = nullptr;
    }

    if (this->pSwrCtx)
    {
        swr_free(&this->pSwrCtx);
        pSwrCtx = nullptr;
    }
    SDL_CloseAudio();
    SDL_Quit();
}

int audioState::decode()
{
    Assert(inited);
    auto pack = audioPktQueue.pop();
    if (!pack)
    {
        return 0;
    }
    int flag = avcodec_send_packet(this->pCodecCtx, pack.value());
    av_packet_unref(pack.value());
    if (flag == 0)
    {
        if (avcodec_receive_frame(this->pCodecCtx, pFrame) == 0)
        {
            /*检查和确定重采样后的一帧音频采样数以及大小*/
            int nbOutSamples = av_rescale_rnd(
                swr_get_delay(this->pSwrCtx, this->pCodecCtx->sample_rate) + this->pFrame->nb_samples,
                pFrame->sample_rate,
                pFrame->sample_rate,
                AV_ROUND_UP);

            int outBufSz = av_samples_get_buffer_size(
                nullptr,
                this->pCodecCtx->channels,
                nbOutSamples,
                outFormatFFMPEG,
                1);

            if (outBufSz > this->audioCvtBufSz)
            {
                av_free(audioCvtBuf);
                audioCvtBuf = (uint8_t *)av_malloc(outBufSz);
                this->audioCvtBufSz = outBufSz;
            }

            /*重采样*/
            int retSamples;
            retSamples = swr_convert(
                pSwrCtx,
                &audioCvtBuf,
                nbOutSamples,
                (const uint8_t **)pFrame->data,
                pFrame->nb_samples);
            if (retSamples >= 0)
            {
                int dataSz = retSamples * pFrame->channels * av_get_bytes_per_sample(outFormatFFMPEG);
                memcpy(audioBuf, audioCvtBuf, dataSz);
                if (pFrame->best_effort_timestamp != AV_NOPTS_VALUE)
                {
                    audioClk = av_q2d(pStream->time_base) * pFrame->best_effort_timestamp;
                }
                return dataSz;
            }
        }
    }
    return -1;
}

void audioState::playAudio()
{
    Assert(inited);
    SDL_AudioSpec wantedSpec, spec;
    wantedSpec.freq = outSampleRate;
    wantedSpec.format = outFormatSDL;
    wantedSpec.channels = pCodecCtx->channels;
    wantedSpec.silence = 0;
    wantedSpec.samples = SDL_AUDIO_BUFFER_SIZE;
    wantedSpec.callback = sdlCallBack;
    wantedSpec.userdata = this;

    Assert(SDL_OpenAudio(&wantedSpec, &spec) >= 0);
    SDL_PauseAudio(0);
}

double audioState::getAudioClk()
{
    int leftMem = audioBufSz - audioBufIdx;
    double bytesPerSec = outSampleRate * av_get_bytes_per_sample(outFormatFFMPEG) * pCodecCtx->channels;
    return audioClk - leftMem / bytesPerSec;
}

void sdlCallBack(void *userdata, Uint8 *stream, int len)
{

    audioState *audio = (audioState *)userdata;
    int len1, audioSz;
    while (len > 0)
    {
        if (audio->audioBufIdx >= audio->audioBufSz)
        {
            audioSz = audio->decode();
            if (audioSz < 0)
            {
                audio->audioBufSz = 0;
                memset(stream, 0, len);
                std::cerr << "Decoding error, filling with silence" << std::endl;
            }
            else
            {
                audio->audioBufSz = audioSz;
            }
            audio->audioBufIdx = 0;
        }
        len1 = audio->audioBufSz - audio->audioBufIdx;
        if (len1 > len)
            len1 = len;
        memcpy(stream, audio->audioBuf + audio->audioBufIdx, len1);
        len -= len1;
        stream += len1;
        audio->audioBufIdx += len1;
    }
}