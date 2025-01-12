#include <thread>
#include "Format_Print.hpp"
#include "tqueue.hpp"
enum class endFlag
{
    end
};
class finishTread
{
public:
    int m_threadNum;
    myThreadQueue<endFlag> endQueue;

    finishTread(int nbThreads = 1) : m_threadNum(nbThreads), endQueue(nbThreads) {}

    void finish()
    {
        for (int i = 0; i < m_threadNum; i++)
        {
            endQueue.pop();
        }
        PrintInfo("ALL THREAD FINISHED");
    }

    void start()
    {
        std::thread finT([this]()
                         { this->finish(); });
        finT.detach();
    }
};
