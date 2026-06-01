/**
 * @file rate_limiter.h
 * @brife RateLimiter for Utility of engine_management_system
 * @date 01.06.2026
 */

 #ifndef RATE_LIMITER_H
 #define RATE_LIMITER_H

 #include "signals/signals.h"
 #include <algorithm>

 namespace rate_limiter {
 class RateLimiter {
    public:
        struct Config {
            float max_step;
        };

        RateLimiter(
            Config config
        ):config_(config), output_(0.0f) {}

        void Update(float target) {
            if (target > output_) {
                output_ += std::min(target - output_, config_.max_step);
            } else if (target < output_) {
                output_ -= std::min(output_ - target, config_.max_step);
            }
        }

        float GetValue() const {
            return output_;
        }

    private:
        Config config_;
        float output_;
        

 };
 }



 #endif
