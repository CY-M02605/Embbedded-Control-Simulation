/**
 * @file hysteresis.h
 * @brief Hysteresis class for the utility
 * @date 28.05.2026
 */

#ifndef HYSTERESIS_H
#define HYSTERESIS_H

// Do not hardcode threshold values in utility classes -- pass them via Config for reusability.

#include "signals/signals.h"

namespace utility {
class Hysteresis {
    public:
        struct Config {
            float low_threshold;
            float high_threshold;
        };

        Hysteresis(
            Config config
        ):config_(config),
        state_(Signals::OnOffState::OFF) {}

        void Evaluate(float temp) {
            if (temp >= config_.high_threshold ) {
                state_ = Signals::OnOffState::ON;
            } else if (temp <= config_.low_threshold) {
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
