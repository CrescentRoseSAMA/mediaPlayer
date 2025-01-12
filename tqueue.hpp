#pragma once
#include <condition_variable>
#include <deque>
#include <chrono>
#include <mutex>
#include <optional>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <deque>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
/*  写在前面
 *
 *  std::chrono::steady_clock::duration 代表一个时间段，类比与1个小时，20秒这种
 *  std::chrono::steady_clock::time_point 代表自开机以来的时间
 *
 */
using packetOpt = std::optional<AVPacket *>;
using frameOpt = std::optional<AVFrame *>;
template <class T, class Deque = std::deque<T>>
class myThreadQueue
{
private:
    Deque m_queue;
    QMutex m_mutex;
    QWaitCondition m_cv_empty;
    QWaitCondition m_cv_full;
    std::size_t m_limit;

public:
    myThreadQueue() : m_limit(std::numeric_limits<std::size_t>::max()) {};

    explicit myThreadQueue(std::size_t limit) : m_limit(limit) {};

    virtual void beforePush(T value) {};

    virtual void afterPop() {};

    void push(T value)
    {
        QMutexLocker locker(&m_mutex);
        while (m_queue.size() >= m_limit) // 队列满了，等待
        {
            m_cv_full.wait(&m_mutex);
        }
        beforePush(value);
        m_queue.push_front(value);
        m_cv_empty.wakeOne();
    }

    bool tryPush(T value)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= m_limit)
            return false;
        beforePush(value);
        m_queue.push_front(value);
        m_cv_empty.wakeOne();
        return true;
    }

    bool tryPushFor(T value, std::chrono::steady_clock::duration timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_full.wait(&m_mutex, timeout.count());
        if (!success)
            return false;
        beforePush(value);
        m_queue.push_front(value);
        m_cv_empty.wakeOne();
        return true;
    }

    T pop()
    {
        QMutexLocker locker(&m_mutex);
        while (m_queue.empty())
        {
            m_cv_empty.wait(&m_mutex);
        }
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        afterPop();
        m_cv_full.wakeOne();
        return value;
    }

    std::optional<T> tryPop()
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.empty())
            return std::nullopt;
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        afterPop();
        m_cv_full.wakeOne();
        return value;
    }

    std::optional<T> tryPopFor(std::chrono::steady_clock::duration timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_empty.wait(&m_mutex, timeout.count());
        if (!success)
            return std::nullopt;
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        afterPop();
        m_cv_full.wakeOne();
        return value;
    }
    void clear()
    {
        QMutexLocker locker(&m_mutex);
        m_queue.clear();
        m_cv_full.wakeAll();
        m_cv_empty.wakeAll();
    }

    int size()
    {
        return m_queue.size();
    }
};

class PacketQueue
{
private:
    std::deque<packetOpt> m_queue;
    QMutex m_mutex;
    QWaitCondition m_cv_empty;
    QWaitCondition m_cv_full;
    std::size_t m_limit;

public:
    PacketQueue() : m_limit(std::numeric_limits<std::size_t>::max()) {};

    explicit PacketQueue(std::size_t limit) : m_limit(limit) {};

    void push(packetOpt value)
    {
        AVPacket *pkt;
        QMutexLocker locker(&m_mutex);
        while (m_queue.size() >= m_limit) // 队列满了，等待
        {
            m_cv_full.wait(&m_mutex);
        }
        if (value)
        {
            pkt = av_packet_clone(value.value());
            m_queue.push_front(pkt);
        }
        else
        {
            m_queue.push_front(value);
        }
        m_cv_empty.wakeOne();
    }

    bool tryPush(packetOpt value)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= m_limit)
            return false;
        AVPacket *pkt;
        if (value)
        {
            pkt = av_packet_clone(value.value());
            m_queue.push_front(pkt);
        }
        else
        {
            m_queue.push_front(value);
        }
        m_cv_empty.wakeOne();
        return true;
    }
    packetOpt pop()
    {
        QMutexLocker locker(&m_mutex);
        while (m_queue.empty())
        {
            m_cv_empty.wait(&m_mutex);
        }
        packetOpt value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }

    void clear()
    {
        QMutexLocker locker(&m_mutex);
        for (auto pkt : m_queue)
        {
            if (pkt)
            {
                av_packet_free(&pkt.value());
            }
        }
        m_queue.clear();
        m_cv_full.wakeAll();
        m_cv_empty.wakeAll();
    }
    int size()
    {
        return m_queue.size();
    }
};

class FrameQueue
{
private:
    std::deque<frameOpt> m_queue;
    QMutex m_mutex;
    QWaitCondition m_cv_empty;
    QWaitCondition m_cv_full;
    std::size_t m_limit;

public:
    FrameQueue() : m_limit(std::numeric_limits<std::size_t>::max()) {};

    explicit FrameQueue(std::size_t limit) : m_limit(limit) {};

    void push(frameOpt value)
    {
        AVFrame *frm;
        QMutexLocker locker(&m_mutex);
        while (m_queue.size() >= m_limit) // 队列满了，等待
        {
            m_cv_full.wait(&m_mutex);
        }
        if (value)
        {
            frm = av_frame_clone(value.value());
            m_queue.push_front(frm);
        }
        else
        {
            m_queue.push_front(value);
        }
        m_cv_empty.wakeOne();
    }

    frameOpt pop()
    {
        QMutexLocker locker(&m_mutex);
        while (m_queue.empty())
        {
            m_cv_empty.wait(&m_mutex);
        }
        frameOpt value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }
};