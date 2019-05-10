#include "golthread.h"
#include "golscene.h"

#include <mutex>



GOLThread::GOLThread(GOLScene* scene, QObject *parent)
  : QThread(parent)
  , m_scene(scene)
{
    m_run.store(true);
    m_lastFps = m_scene->fps();
}

GOLThread::~GOLThread()
{
    
}


void GOLThread::run()
{
    while (m_run.load())
    {
        long delta;
        if (m_timer.isValid()) 
        { 
            delta = static_cast<long>(m_timer.nsecsElapsed() / 1000);
        }
        else { delta = 0; }
        m_timer.start();
        
        if (!m_scene->paused())
        {
            m_scene->tick();
        }
        
        long add = (delta != 0) ? (long)((1000.0 / std::max(m_lastFps, 1)) * 1000.0) - delta - (m_timer.nsecsElapsed() / 1000) : 0;
        
        m_lastFps = m_scene->fps();
        long wait = (long)((1000.0 / std::max(m_lastFps, 1)) * 1000.0);
        
        QThread::usleep(std::max((long)0, std::min(wait + add, wait * 2)));
    }
}
