/**
 * @file signals.h
 * @brief Signals for signals of engine_management_system
 * @date 01.06.2026
 */

 #ifndef SIGNALS_H
 #define SIGNALS_H

 namespace signals {

    enum class OnOffStatus {
        ON, OFF
    };

    enum class ValidationStatus {
        VALID, INVALID
    };
    template <typename T>
    class Signals {
        public:

            Signals(
                T value,
                ValidationStatus validation
            ):value_(value),
            validation_(validation) {}

            T GetValue() const {
                return value_;
            }

            ValidationStatus GetState() const {
                return validation_;
            }

            void SetValue(T value) {
                value_ = value;
            }

            void SetState(ValidationStatus validation) {
                validation_ = validation;
            }

            void Set(T value, ValidationStatus validation) {
                value_ = value;
                validation_ = validation;
            }

            bool IsValid() const {
                return validation_ == ValidationStatus::VALID;
            }

        private:
            T value_;
            ValidationStatus validation_;
            
    };

    typedef Signals<float> TemperaturalSignal;
    typedef Signals<float> SpeedSignal;
    typedef Signals<bool> BoolSignal;
    typedef Signals<int> EngineRotationSignal;

    typedef Signals<OnOffStatus> OnOffSignal;
 }


 #endif
