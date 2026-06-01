/**
 * @file hysteresis.h
 * @brife Hysteresis for utility of engine_management_system
 * @date 01.06.2026
 */

 #ifndef HYSTERESIS_H
 #define HYSTERESIS_H

 #include "signals/signals.h"

 namespace utility{
 class Hysteresis {
    public:
        struct Config {
            float high_threshold;
            float low_threshold;
        };

        Hysteresis(
            Config config
        ):config_(config),
        state_(signals::OnOffStatus::OFF) {}

        void Evaluate(float value) {
            if (value >= config_.high_threshold) {
                state_ = signals::OnOffStatus::ON;
            } else if (value <= config_.low_threshold) {
                state_ = signals::OnOffStatus::OFF;
            }
        }

        signals::OnOffStatus GetState() const {
            return state_;
        }

    private:
        Config config_;
        signals::OnOffStatus state_;

 };    
 }


 #endif
