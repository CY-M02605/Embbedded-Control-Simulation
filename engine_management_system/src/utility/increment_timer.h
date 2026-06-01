/**
 * @file increment_timer.h
 * @brife IncrementTimer for utility of engine_management_system
 * @date 01.06.2026
 */

 #ifndef INCREMENT_TIMER_H
 #define INCREMENT_TIMER_H

 #include "signals/signals.h"

 namespace utility {
 class IncrementTimer {
    public:
        struct Config {
        float step;
        float threshold;
        };

        IncrementTimer(
            Config config):
            config_(config),
            elapsed_(0.0f) {}

        void Clear() {
            elapsed_ = 0;
        }

        void Update() {
            elapsed_ += config_.step;
        }

        bool IsTimeUp() const {
            return (elapsed_ >= config_.threshold);
        }

    private:
        Config config_;
        float elapsed_;   // 成员变量，记录积累时间

 };
 }


 #endif
