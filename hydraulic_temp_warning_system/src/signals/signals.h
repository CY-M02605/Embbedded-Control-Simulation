/**
 * @file signals.h
 * @brief Signals template class for the signals
 * @date 28.05.2026
 */

#ifndef SIGNALS_H
#define SIGNALS_H

namespace Signals {

    // enum class is a strongely-typed enumeration introduced in C++11
    enum class OnOffState {
        ON,
        OFF
    };

    enum class SignalValidity {
        VALID,
        INVALID
    };

    // The T here is a placeholder type, allows Signal to store values of different type.
    template <typename T>
    class Signal {
        public:

            // Default constructor
            Signal(
            ):value_(T()), validity_(SignalValidity::VALID) {}

            // Constructor with parametres
            Signal(
                T value, SignalValidity validity
            ):value_(value), validity_(validity) {}

            // Adding const after a method guarantees that the method does not modify member variable
            // Benefit: If another file holds a const reference, e.g. const Signal<float>& oil_temp
            // only methods marked with const can be called.
            // calling a non-const method through a const reference will cause a compiler error.
            T GetValue() const {
                return value_;
            }

            SignalValidity GetValidity() const {
                return validity_;
            }

            bool IsValid() const {
                return validity_ == SignalValidity::VALID;
            };

            // Methods that modify member variables must NOT be marked const
            void SetValue(T value) {
                value_ = value;
            };

            void SetValidity(SignalValidity validity) {
                validity_ = validity;
            }

            void SetDV(T value, SignalValidity validity) {
                value_ = value;
                validity_ = validity;
            }

        private:
            T value_;
            SignalValidity validity_;
    };

    // typedef creates an alias: Signal<T> wraps a value of type T with a validity field.
    // The type inside angle brackets <> specifies what kind of value the Signal holds.
    typedef Signal<float> TemperatureSignal;
    typedef Signal<float> SpeedSignal;
    typedef Signal<bool> BoolSignal;

    typedef Signal<OnOffState> OnOffSignal;

    // Signal<SignalValidity> would mean: value = SignalValidity, plus another validity field.
    // The value itself IS validity, wrapped with yet another validity -- redundant and meaningless.
    // typedef Signal<SignalValidity> ValiditySignal;

}

#endif
