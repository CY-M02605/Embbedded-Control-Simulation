/**
 * @file hysteresis.h
 * @brife Hysteresis for utility of engine_management_system
 * @date 01.06.2026
 */

 #ifndef HYSTERESIS_H
 #define HYSTERESIS_H

 #include "signals/signals.h"

 namespace utility{
 class hysteresis {
    public:
        struct Config {
            float high_threshold;
            float low_threshold;
        };

        hysteresis(
            Config config
        ):config_(config),
        state_(Signals::OnOffState::OFF) {}

        void Evaluate(float value) {
            if (value >= config_.high_threshold) {
                state_ = Signals::OnOffState::ON;
            } else if (value <= config_.low_threshold) {
                state_ = Signals::OnOffState::OFF;
            }
        }

        Signals::OnOffState GetState() const {
            return state_;
        }

    private:
        Config config_;
        Signals::OnOffState state_;

 };    
 }


 #endif
