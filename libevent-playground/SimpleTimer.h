#ifndef HTTP_CPP_PLAYGROUND_SIMPLETIMER_H
#define HTTP_CPP_PLAYGROUND_SIMPLETIMER_H
#include <event2/event.h>
#include <iostream>
#include <chrono>
#include <memory>

class SimpleTimer {
public:
    SimpleTimer() : m_base(event_base_new(), &event_base_free),
                    m_timer_event(nullptr, &event_free),
                    m_counter(0) {
        if (!m_base) {
            throw std::runtime_error("Failed to create event_base");
        }

        m_timer_event.reset(evtimer_new(m_base.get(), timer_callback, this));
        if (!m_timer_event) {
            throw std::runtime_error("Failed to create timer_event");
        }
    }

    void start() {
        std::cout << "Timer start\n";

        timeval tv{1, 0};
        evtimer_add(m_timer_event.get(), &tv);

        event_base_dispatch(m_base.get());
    }

    void stop() {
        std::cout << "Timer stop\n";
        event_base_loopexit(m_base.get(), nullptr);
    }

private:
    /**
     * decltype으로 unique_ptr로 선언한 event_base와 event의 컴파일 타임 타입 추론
     *  -> 소멸될 때 delete가 아닌, event_base_free, event_free 가 호출되도록 지정
     */
    std::unique_ptr<event_base, decltype(&event_base_free)> m_base;
    std::unique_ptr<event, decltype(&event_free)> m_timer_event;
    int m_counter;

    static void timer_callback(evutil_socket_t fd, short event, void* arg) {
        auto* self = static_cast<SimpleTimer*>(arg);
        self->handle_timer();
    }

    void handle_timer() {
        std::cout << "[EVENT] #" << (m_counter + 1) << "\n";

        if (m_counter >= 9) {
            std::cout << "Timer count is over (10), stop\n";
            stop();
            return;
        }

        timeval tv{1, 0};
        evtimer_add(m_timer_event.get(), &tv);
        m_counter++;
    }
};


#endif //HTTP_CPP_PLAYGROUND_SIMPLETIMER_H
