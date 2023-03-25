#ifndef __TIMER_H__
#define __TIMER_H__

namespace infra {

class TimerHandler {
 public:
    TimerHandler() {}
    virtual ~TimerHandler() {}
    virtual void expired() = 0;
};

class AbstractTimer {
 public:
    AbstractTimer();
    ~AbstractTimer();
    virtual void stop() = 0;

};

class AbstractNodeHandle {
 public:
    AbstractNodeHandle();
    ~AbstractNodeHandle();

    virtual AbstractTimer* createTimer(double duration, TimerHandler &handler, bool oneshot = true, bool autostart = true) = 0;
};

class Timer {
 public:
    Timer() : m_timer_handler(nullptr) {};
    ~Timer() {};

 public:

    void setTimer(double duration, TimerHandler &handler) {
        m_timer_handler = &handler;
        m_abstract_timer = m_abstract_node_handle->createTimer(duration, handler, true, true);
    }

    void stopTimer() {
        m_abstract_timer->stop();
    }


 private:
    // possibly can remove this expired() func, depending on non-ros timer implementation
    void expired(void) { 
        if (m_timer_handler) {
            m_timer_handler->expired();
        }
    }   
    TimerHandler *m_timer_handler;  // possibly can remove this TimerHandler ptr, depending on non-ros timer implementation
    AbstractNodeHandle *m_abstract_node_handle;
    AbstractTimer *m_abstract_timer;
};
};
#endif // __TIMER_H__

