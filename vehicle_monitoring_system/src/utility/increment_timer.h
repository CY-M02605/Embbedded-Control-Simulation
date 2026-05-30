/**
 * @file increment_timer.h
 * @brief IncrementTimer class for the Utility
 * @date 28.05.2026
 */

 #ifndef INCREMENT_TIMER_H
 #define INCREMENT_TIMER_H

 #include "signals/signals.h"

 namespace utility {
 class IncrementTimer {
    public:
        struct Config {
            float threshold;

            // step size (how much to add each time)
            float step;
        };

        IncrementTimer(
            Config config
        ):config_(config),
        elapsed_(0.0f) {}

        void Update(bool value) {
            if (value) {
                elapsed_ = elapsed_ + config_.step;
            } else {
                elapsed_ = 0.0f;
            }
        }

        bool IsTimeUp() const {
            return elapsed_ >= config_.threshold;
        }

        void Clear() {
            elapsed_ = 0.0f;
        }

    private:
        Config config_;

        // elapsed time (accumulated over time)
        float elapsed_;

 };
 }

#endif
 