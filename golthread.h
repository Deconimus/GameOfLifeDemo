#ifndef GOLTHREAD_H
#define GOLTHREAD_H


#include <QObject>
#include <QWidget>
#include <QThread>
#include <QElapsedTimer>


#include <atomic>
#include <mutex>


class GOLScene;


class GOLThread : public QThread
{
    Q_OBJECT
    
public:
    explicit GOLThread(GOLScene* scene, QObject *parent = nullptr);
    virtual ~GOLThread();
    
    
    inline void setRun(bool run) { m_run.store(run); }
    
    
protected:
    
    virtual void run() override;
    
    
signals:
    
    
public slots:
    
    
private:
    
    std::atomic<bool> m_run;
    
    GOLScene* m_scene;
    
    int m_lastFps;
    QElapsedTimer m_timer;
    
};

#endif // GOLTHREAD_H
