# Universal Embedded Systems Knowledge
## Learned from a Real Safety-Critical Transmission Control System

> This document extracts **project-independent, universally applicable** knowledge from hands-on experience analyzing a 400+ module embedded C++ codebase for heavy construction equipment (Renesas RH850, Green Hills compiler, 10ms cyclic control loop).
>

## 36. Diagnosis Module Inheritance vs From-Scratch Design Pattern

### 36.1 Template Method Pattern in Fault Detection
```
Base class: fault_state_detection_model::Function
  ├── Has: FailureDetector (contains Timer + Mask + State Machine + FailureSignal)
  ├── Has: Update() → calls virtual IsFaultCondition()/IsRecoverCondition()
  └── Subclass only overrides: condition check functions (return bool)

Subclass writes:             Base class handles:
┌──────────────────┐         ┌──────────────────────────┐
│ IsFaultCondition  │ ──bool──▶ Timer confirmation        │
│ IsRecoverCondition│ ──bool──▶ Mask check                │
└──────────────────┘         │ State machine (NO→CHECK   │
  "Is voltage out            │   →OCCUR→RECOVER→NO)      │
   of range?"                │ FailureSignal output      │
                             └──────────────────────────┘
```
Key insight: Don't re-implement what the base class already provides. Subclass should be as simple as clutch_slip: two if-else functions returning bool.

### 36.2 AbnormalityDetector: current_flag vs fault_value
```
current_flag = IsFaultCondition() result (this instant)
fault_value  = timer_.Update(current_flag) result (confirmed over time)

Cycle:  1    2    3    ...  50   51   52
curr:   true true true      true false false
fault:  false false false   true true  false (after recover timer)

JudgeAbnormalityState(current, fault):
  (false, false) → NO       Normal
  (true,  false) → CHECK    Condition met, timer counting
  (true,  true)  → OCCUR    Timer confirmed, fault active
  (false, true)  → RECOVER  Condition gone, awaiting recovery
```

### 36.3 From-Scratch Fault Detector Design
When NOT inheriting, implement everything yourself:
```
Update() {
    if (mask == ON) → freeze/skip
    
    switch (status_) {
    case NORMAL:
        output = NO;  // default
        if (fault condition) {
            confirm_timer++;
            if (timer up) { status = FAULT; output = OCCUR; }
        } else { confirm_timer.Clear(); }
        break;
    case FAULT:
        output = OCCUR;  // default
        if (recover condition) {
            recover_timer++;
            if (timer up) { status = NORMAL; output = NO; }
        } else { recover_timer.Clear(); }
        break;
    }
}
```
Tip: Set default output first, then override on transition — avoids one-cycle output delay.

### 36.4 switch-case Output Delay Pitfall
```
switch enters case based on value at entry time.
Even if variable changes inside case, execution stays in same case block.
→ Output set inside case reflects OLD state, not new state.

Fix: Set output AFTER switch using if-else (checks live value),
     or set default + override inside case on transition.
```

### 36.5 Common C/C++ Diagnosis Code Bugs
- `=` vs `==` in if conditions: `if (x = val)` assigns, always true
- `&this` vs `*this`: this is already pointer, *this = object reference
- Initializer list uses `,` not `;` between members
- Signal objects need `.GetValue()` before comparing with enum
- FailureSignal constructed with (EnumFailureState, EnumSignalState), not assigned raw enum


> All project-specific names have been generalized. These concepts apply to **any** embedded control system.

---

## 1. C++ for Embedded Systems

### 1.1 Syntax Quick Reference

| Symbol | Meaning | Example |
|--------|---------|---------|
| `::` | Scope resolution (namespace / class member) | `namespace::ClassName` |
| `&` | Reference (alias, no copy) / Address-of | `const Config& config` |
| `*` | Pointer / Dereference | `*this` |
| `const` | Immutable / read-only | `const float& value` |
| `extern` | External linkage — object defined in another translation unit | `extern Config config;` |
| `virtual` | Virtual function — overridable by subclass | `virtual void Update();` |
| `= 0` | Pure virtual — subclass **must** implement | `virtual void Update() = 0;` |
| `<>` | Template type parameter | `Container<Module, 32>` |
| `*this` | The object itself (dereference of `this` pointer) | `manager.Register(*this)` |
| `#define` | Preprocessor macro — text substitution | `#define MAX 100` |
| `#include` | Preprocessor — textual copy-paste of a file | `#include "timer.h"` |
| `#ifndef / #define / #endif` | Include guard — prevents duplicate inclusion | Standard pattern |
| `namespace` | Logical grouping to avoid name collisions | `namespace logic { }` |
| `class / struct` | struct defaults to `public`, class defaults to `private` | — |
| Initializer list | `member_(value)` after colon in constructor | `timer_(config.timer_config)` |
| `.h / .cc (.cpp)` | Header (declaration) / Source (implementation) | `.h` = interface, `.cc` = code |
| `40.F` | Float literal (F suffix = float type) | — |
| `10U` | Unsigned literal (U suffix) | — |
| `std::numeric_limits<T>::max()` | Maximum value of a type | Used for overflow protection |

### 1.2 Comment Syntax

```cpp
/* This is a comment */  42    // ← 42 is code, NOT a comment!
// Single-line comment
/* label = */ 40.F             // ← 40.F is code; the part before is a comment label
```

### 1.3 Aggregate Initialization

```cpp
struct TimerConfig {
    uint32_t sampling_time;
    uint32_t initial_time;
    uint32_t threshold_time;
};

struct ModuleConfig {
    float threshold;               // Field 1 (simple)
    TimerConfig timer_config;      // Field 2 (nested struct)
};

ModuleConfig config = {
    40.0F,                         // Field 1
    {10U, 0U, 500U}                // Field 2 (nested → wrap in { })
};
```
**Rule**: Each level of struct nesting = one level of `{ }` braces.

### 1.4 Data Types in Embedded C/C++

| Type | Description |
|------|-------------|
| `uint8_t / uint16_t / uint32_t` | Fixed-width unsigned integers (8/16/32-bit) |
| `int8_t / int16_t / int32_t` | Fixed-width signed integers |
| `float` / `double` | Single / double precision floating-point |
| `bool` | Boolean — `true` / `false` |
| `enum` | Enumeration — named constants (e.g. ON, OFF, FORWARD, REVERSE) |

### 1.5 Three Constructor Initializer-List Patterns

```cpp
MyModule::MyModule(const Config& config, const Signal& input, Manager& mgr)
    : config_(config),                     // ① Reference binding (const&) — "borrowed" data
      timer_(config.timer_config),         // ② Object construction — "owned" tool, created from config
      output_(false) {                     // ③ Value initialization — simple initial state
  mgr.Register(*this);
}
```

| Pattern | Ownership | Can modify? |
|---------|-----------|-------------|
| `const &` member | Borrowed (belongs to someone else) | No (read-only) |
| Non-reference member (object) | Owned (this instance's property) | Yes |
| Non-reference member (primitive) | Owned | Yes |

### 1.6 Reference Binding vs Value Copy in Constructor

A subtle but critical difference in initializer lists:

```cpp
class Monitor {
    const State& live_state_;       // Reference — "live window"
    State snapshot_state_;          // Value copy — "snapshot"

    Monitor(const State& state_signal)
        : live_state_(state_signal),              // Reference binding: NO .GetValue()
          snapshot_state_(state_signal.GetValue()) // Value copy: calls .GetValue()
    {}
};
```

| | Reference binding | Value copy |
|---|---|---|
| **Syntax** | `member_(obj)` | `member_(obj.GetValue())` |
| **Result** | Alias to the original — always sees latest value | Independent copy — frozen at construction time |
| **Analogy** | Security camera (live feed) | Photograph (captured once) |
| **Use case** | "Current state" — need real-time data | "Previous state" — need last-cycle snapshot |

**Why it matters**: In cyclic systems, you often need both the **current** value (reference) and the **previous** value (snapshot updated at end of each cycle) for edge detection.

```cpp
void Update() {
    // live_state_ always shows current value (reference)
    // snapshot_state_ still has last cycle's value (copy)
    if (snapshot_state_ != live_state_) {
        // Edge detected! State just changed.
    }
    snapshot_state_ = live_state_;  // Update snapshot for next cycle
}
```

### 1.7 Header (.h) vs Source (.cc) Relationship

```
.h declares ("I have these")              .cc implements ("Here's how they work")
──────────────────────                    ──────────────────────
class MyModule {                          (no need to repeat class)
  struct Config { ... };                  (no need to repeat, defined in .h)
  MyModule(params);              ──→      MyModule::MyModule(params) : init_list { body }
  void Update();                 ──→      void MyModule::Update() { logic }
  const Signal& OutputRef() const; →      const Signal& MyModule::OutputRef() const { return output_; }
  private: members;                       (no need to repeat, declared in .h)
};
```

**Why `MyModule::` prefix in .cc?** — Outside the class body, the compiler needs to know which class the method belongs to.

**Naming convention**: Constructor parameter `config` → member variable `config_` (trailing underscore).

---

## 2. Compilation & Linking

### 2.1 Two-Step Process: Source → Executable

| | Step 1: Compilation | Step 2: Linking |
|---|---|---|
| **What** | Each `.cc` compiled **independently** into `.o` | All `.o` files **combined** into one executable |
| **Needs** | `.h` files (type/shape information) | `.o` files (actual machine code) |
| **Output** | `.o` with unresolved symbol references | Executable (`.elf` / `.abs` / `.hex` / `.mot`) |

### 2.2 Why #include ≠ Direct Relationship

```
              timer.h (declaration)
               ↑              ↑
               |              |
  my_module.h               timer.cc
  "I use it as a member,    "I implement its methods,
   need to know its shape"   need to know its shape"
```

- Two files compiled **independently** — they don't know about each other.
- They both `#include` the same `.h` because they both need the type's shape.
- The **linker** connects them by resolving symbol addresses.

### 2.3 #include vs Linking

| | `#include` (compile-time) | Linking (post-compile) |
|---|---|---|
| **When** | Preprocessing stage (before compilation) | After all `.cc` files are compiled |
| **Does** | Copy-pastes `.h` file contents | Combines `.o` files into executable |
| **Answers** | "What does this type look like?" | "Where is the actual code for this function?" |

### 2.4 Typical Embedded Build Pipeline

```
Source files (.cc)  →  Object files (.o)  →  Static libraries (.a/.lib)
                                                      ↓
                                            Linker combines all
                                                      ↓
                                            Executable (.elf/.abs)
                                                      ↓
                                            Flash image (.hex/.mot/.bin)
                                                      ↓
                                            Flashed to MCU via programmer
```

### 2.5 Build System Basics (Make + Shell)

```makefile
# Typical Makefile structure:
# Main makefile — global settings (compiler, chip, output name)
# Module makefile — per-module compilation rules (.cc → .o → .lib)
# User config — source directories, include paths, libraries to link
```

```bash
# Common Make targets:
make all        # Full build (compile + link → executable)
make clean      # Remove intermediate files (.o)
make lib -j4    # Compile library with 4 parallel jobs
```

**Cross-compilation**: Compiling on PC (x86) to produce code that runs on MCU (e.g. ARM, RH850). The compiler runs on your machine but generates machine code for a different architecture.

---

## 3. Software Architecture Patterns

### 3.1 Layered Architecture

```
┌─────────────────────────────────┐
│  Application Layer              │  ← Business logic modules (your code)
├─────────────────────────────────┤
│  Middleware / Framework Layer   │  ← Module manager, signal types, interfaces
├─────────────────────────────────┤
│  Utility Layer                  │  ← Generic tools (timers, arrays, math)
├─────────────────────────────────┤
│  OS / HAL Layer                 │  ← Hardware abstraction, drivers, RTOS
└─────────────────────────────────┘
```

**Rule**: Lower layers provide services to upper layers. Upper layers must not modify lower layers.

### 3.2 Module Pattern (The Standard Embedded Module)

```cpp
class MyModule : public ModuleInterface {
 public:
  struct Config {                           // Tunable parameters
      float threshold;
      TimerConfig timer_config;
  };

  MyModule(const Config& config,            // Configuration (injected)
           const Signal& input,             // Input signals (const ref = read-only)
           Manager& manager);               // Lifecycle manager

  void Update();                            // Called cyclically (e.g. every 10ms)
  const Signal& OutputRef() const;          // Other modules read this

 private:
  const Signal& input_;                     // Borrowed input (read-only)
  const Config& config_;                    // Borrowed config (read-only)
  Timer timer_;                             // Owned internal state
  Signal output_;                           // Owned output
};
```

### 3.3 Five Common Module Patterns

| Pattern | Has Config? | Has Internal State? | Core Logic | Example Use Case |
|---------|:-----------:|:-------------------:|------------|------------------|
| **Combinational** | No | No | `if/else` | Reverse lamp: reverse gear → light ON |
| **Timer/Debounce** | Yes | Yes (timer) | Timer + Clear | Vehicle stop: speed < threshold for 500ms |
| **Multi-output** | Yes | No | `if/else` | Direction judge: outputs direction + safety flag |
| **State Machine** | Yes | Yes (state enum) | `switch-case` | Edge detection: OFF → OFF_ON → ON → ON_OFF |
| **Gate + Histogram** | Yes | Yes (flag + snapshot) | Gate flag → N-condition gate → bin → count + alert | Clutch quality: measure, classify, count, alert if severe |

### 3.4 Design Patterns in Embedded Systems

| Pattern | How It Appears | Why It Matters |
|---------|---------------|----------------|
| **Composite** | Manager contains modules; Manager itself is also a module | Tree-structured task execution |
| **Dependency Injection** | All dependencies passed via constructor; modules never create/locate their own | Loose coupling, testability |
| **Interface Abstraction** | Base class defines `Update()`, all modules implement it | Uniform lifecycle management |
| **Strategy** | Sensor selection switches between algorithms based on diagnostic state | Runtime behavior switching |
| **Observer** (lightweight) | Modules read other modules' outputs through `const &` | No callback overhead |
| **Template Method** | Base class defines lifecycle; subclasses override specific steps | Extensibility via `protected virtual` |
| **Star Topology FSM** | All states route through a central idle state | Safety, simplicity, no stale state |

### 3.5 Loose Coupling: The Wiring Mechanism

```
Module A                    Wiring File                    Module B
─────────                   ────────────                   ─────────
Defines OutputRef()    →    A.OutputRef() passed as     →  Constructor receives
(output method)             parameter to B's constructor    const Signal& input
                            (the ONLY place that knows      (doesn't know where
                             who connects to whom)           the signal comes from)
```

**Key Rules**:
1. Module A doesn't know who reads its output (loose coupling)
2. Module B doesn't know where its input comes from (only knows the type)
3. Only the wiring file knows "who connects to whom"
4. Parameters are positional — order matters
5. Dependencies must be created before dependents
6. Downstream modules can only **read** upstream outputs (`const &`)

**Modules do NOT `#include` each other.** They only `#include` signal type definitions and framework classes. The wiring file `#include`s ALL modules and connects them.

### 3.6 Configuration Separation

```
Module .h  → Defines struct Config (the "shape" — what fields exist, what types)
Module .cc → Uses config_.field_name (reads values, doesn't know where they come from)
Config file → Fills in actual values (40.0F, 500U, etc.)
Wiring file → Passes config values to module constructors
```

**Benefit**: Change a threshold value without touching module code. Different products can share the same code with different config files.

---

## 4. Embedded Systems Fundamentals

### 4.1 Core Principles

- **No dynamic memory allocation** (`new`/`delete`) — use fixed-size arrays to prevent fragmentation
- **Signal = value + validity status** (VALID/INVALID) — prevents decisions based on corrupt data
- **`const` protection everywhere** — prevents accidental modification in safety-critical code
- **All objects determined at compile time** — zero runtime overhead, deterministic behavior
- **Module lifecycle**: Init → Update (called cyclically, e.g. every 10ms) → Exit

### 4.2 Cooperative Scheduling (Polling Model)

```
OS timer interrupt (every 10ms)
  → calls main task
    → Manager.Update()
      → Module1.Update()  ← runs, returns quickly
      → Module2.Update()  ← runs, returns quickly
      → Module3.Update()  ← runs, returns quickly
      → ...
```

- **No blocking waits.** Modules check conditions and return immediately.
- "Waiting 500ms" = timer runs for 50 cycles (50 × 10ms), each cycle checks `IsTimeUp()` and returns `false`.
- Every module **must** complete quickly. If one hangs, the entire system freezes.
- Analogy: Checking your mailbox once a day (polling), not standing at the door waiting (blocking).

### 4.3 Increment Timer (Debounce Tool)

The most common utility in embedded control. Counts up each cycle, clears on condition break.

```cpp
// Internal state:
//   time_       = current accumulated time (ms)
//   time_up_    = has threshold been reached? (stays true — level signal)
//   time_up_now_= was threshold JUST reached? (true for 1 cycle — edge signal)

void Timer::Update() {
    // Overflow protection: clamp at max instead of wrapping to 0
    if (time_ > (UINT32_MAX - sampling_time_))
        time_ = UINT32_MAX;
    else
        time_ += sampling_time_;      // e.g. +10ms each cycle

    if (time_ >= threshold_time_) {
        time_up_now_ = !time_up_;     // true only on the cycle it first reaches threshold
        time_up_ = true;              // stays true from now on
    } else {
        time_up_ = false;
        time_up_now_ = false;
    }
}

void Timer::Clear() {
    time_ = 0;
    time_up_ = false;
    time_up_now_ = false;
}
```

**Timeline example** (sampling=10ms, threshold=30ms):

```
Cycle:    1     2     3     4     5     6
time_:    10    20    30    40    50    60
TimeUp:   F     F     T     T     T     T     ← level (stays true)
TimeUpNow:F     F     T⭐   F     F     F     ← edge (true once)
```

**IsTimeUp() vs IsTimeUpNow()**:
```
IsTimeUp()    = ______████████████████  (stays ON — "level signal")
IsTimeUpNow() = ______█_______________  (one pulse — "edge signal")
                      ↑ threshold reached
```

- `IsTimeUp()` → "Has the condition been met?" (e.g. "Is the vehicle stopped?")
- `IsTimeUpNow()` → "Was the condition **just** met?" (e.g. "Send a one-time notification")

### 4.4 Overflow Protection

- `uint32_t` max = 4,294,967,295 (~49.7 days if incrementing every ms)
- Without protection: counter wraps to 0 → timer suddenly resets → dangerous
- Embedded systems may run for months without reboot → must handle this

### 4.5 Signal Object Pattern (Value + Validity)

```cpp
Signal output(true, SignalState::VALID);    // value=true, trustworthy
Signal output(false, SignalState::INVALID); // value=false, sensor broken

// Usage: unpack → logic → repack
value = signal.GetValue();                  // unpack
result = (value >= threshold);              // logic
output_ = Signal(result, SignalState::VALID); // repack
```

**Rule**: Business modules always output VALID. INVALID originates from hardware layer (CAN timeout, sensor disconnection).

### 4.6 Signal Type Inheritance Chain

In well-designed embedded frameworks, signal types form an inheritance hierarchy:

```
SignalObject<T>           ← Base: holds value_ of type T + GetValue()/SetValue()
  ↑ inherits
NumericSignal<T>          ← Adds: numeric properties (unit, range validation)
  ↑ inherits
Concrete types:           ← Domain-specific: Pressure, Temperature, Rpm, Voltage...
  Pressure                   (= NumericSignal<float32>)
  Temperature                (= NumericSignal<float32>)
  Rpm                        (= NumericSignal<float32>)
```

**Key points**:
- `GetValue()` is defined at the **top** (`SignalObject<T>`) — returns `T value_`
- Business modules typically reference the **concrete type** (`const Pressure& brake_`)
- The module never calls `new` or constructs these — they're provided by upstream device drivers
- `.h` files only `#include` what they **directly use**: `pressure.h` includes `numeric_signal.h`, which includes `signal_object.h`. A module using `Pressure` includes `pressure.h` only — **don't skip levels**

**"Only include what you directly use" rule**:
```
✅ Module includes pressure.h → pressure.h includes numeric_signal.h → ... → signal_object.h
❌ Module includes signal_object.h directly (skips intermediate types)
```
This rule ensures that if an intermediate type changes its base class, only its own `.h` needs updating — downstream code remains untouched.

### 4.7 Complete Signal Chain (Physical Sensor → Software Value)

How a physical measurement becomes a usable software value:

```
Physical world       Hardware         Device Driver          Business Module
─────────────       ─────────        ──────────────         ───────────────
Brake pipe      →   Pressure     →   PressureSensor         BrakeLampModule
(hydraulic)         sensor            ByAnalogInput           reads const Pressure&
                    (analog)           │                       │
                                       ├─ AD conversion        └─ .GetValue() → float
                                       ├─ Low-pass filter         (doesn't know/care
                                       ├─ Range validation         about AD or filter)
                                       ├─ Interpolation table
                                       │  (6 calibration points:
                                       │   voltage → pressure MPa)
                                       └─ Stores in Pressure object
```

**The beauty of this design**: The business module (`BrakeLampModule`) sees only `const Pressure&` and calls `.GetValue()`. It has **zero knowledge** of:
- Which physical sensor (could be swapped to a different brand)
- AD conversion details (12-bit? 16-bit?)
- Filter algorithm (could change from LPF to moving average)
- Calibration table (could be updated without touching business code)

This is **loose coupling at the signal level** — the module is decoupled from hardware implementation.

### 4.8 Bool→Enum Type Conversion Between Layers

Different layers use different types for the same concept:

```
Tool layer (Hysteresis):     bool     (true / false)
Business layer (Output):     enum     (ON / OFF)
```

```cpp
// Hysteresis utility returns bool (generic tool, doesn't know about brake lamps)
bool is_active = hysteresis_.GetState();

// Business logic converts to domain-specific enum
output_ = is_active ? ElectricalOnOffState::ON : ElectricalOnOffState::OFF;
```

**Why?** Generic tools (Hysteresis, Timer) use `bool` because they serve any domain. Business modules use domain enums (`ON/OFF`, `FORWARD/REVERSE`) because they're semantically richer. The conversion happens at the boundary between tool and business logic.

---

## 5. Safety-Critical Design Principles

### 5.1 Debounce Filtering

**Problem**: Sensor signals have noise; readings oscillate near threshold values.

```
Actual value (smooth):    ────────────╲_____________________
Sensor reading (noisy):   ─────┬─┬──╲┬┬──┬________________
                              threshold line
                          ↑↑↑ rapid crossings → false triggers
```

**Solution**: Condition must be sustained for a defined duration.

```cpp
void Update() {
    timer.Update();                     // +10ms each cycle
    if (condition_NOT_met) {
        timer.Clear();                  // reset to 0
    }
    output = timer.IsTimeUp();          // true only after sustained period
}
```

**Why it matters**: Without debounce, the system would rapidly toggle outputs (e.g. clutch engage/disengage), causing mechanical wear and dangerous behavior.

### 5.2 Redundant Sensor Design

**Problem**: Sensors fail. A 50-ton machine cannot lose control because of one broken sensor.

**Solution**: Measure the same physical quantity with two independent sensors (A and B).

```
                 Sensor A ──→ ┐
Physical shaft ──┤             ├──→ Selection logic ──→ output
                 Sensor B ──→ ┘
```

**Three-layer failover**:

| Layer | Logic | Purpose |
|-------|-------|---------|
| **Selection** | Both OK → max(A,B); one failed → use the good one; both failed → 0 | Basic redundancy |
| **Mode switch** | Sensors OK → use measured value; all failed → switch to estimated value | Graceful degradation |
| **Estimation** | Use other available sensors + gear ratios to calculate the missing value | Limp-home fallback |

**Limp-home mode**: When all sensors fail, switch to estimated values and drive back slowly for repair. **Never strand the machine.**

### 5.3 Operator Intent ≠ Physical Reality

| | Operator Intent | Physical Reality |
|---|---|---|
| **Source** | Human input (joystick, pedals) | Sensors (speed, RPM, pressure) |
| **Example** | Gear lever = REVERSE | Vehicle still moving FORWARD (inertia) |
| **Changes** | Instantly (on shift) | With delay (acceleration/deceleration) |

```
Scenario: Vehicle going downhill, operator shifts to Reverse
  T1: gear = REVERSE, but vehicle still sliding FORWARD
  If we engage reverse clutch based on gear alone → transmission damage!
  Cross-validate with speed sensor → wait until stopped → safe
```

**Design method**: Cross-validation — combine operator intent with physical measurements for conservative decisions.

### 5.4 Conservative Design Summary

| Strategy | Description | Example |
|----------|-------------|---------|
| **Output permission** | No control output until system is ready | Block signals during power-up |
| **Graceful degradation** | Safe default values on sensor failure | Use INVALID status to flag bad data |
| **Multi-source validation** | Cross-check independent signals | Speed + gear position cross-validated |
| **Debounce filtering** | Reject transient noise | Speed below threshold for 500ms = "stopped" |
| **Sensor redundancy** | Same quantity measured by two sensors | Dual speed sensors on same shaft |
| **Limp-home mode** | Total sensor failure → estimated values | Drive back slowly using calculated RPM |
| **Safety override** | Intercept output without disturbing internal logic | State machine runs normally; only final output blocked |

### 5.5 Three Abstraction Layers of a Signal

```
Layer 1: Raw hardware     → Sensor pulses / voltage
         ↓ Device driver
Layer 2: Physical quantity → RPM, speed (km/h), pressure (MPa)
         ↓ Diagnostic selection, redundancy, mode switching
Layer 2.5: Safety-processed → After A/B redundancy + fault degradation
         ↓ Business modules
Layer 3: State / decision  → "Stopped?" (bool), "Direction" (FORWARD/REVERSE/STOP)
```

Each layer is handled by a separate module — **single-responsibility pipeline**.

---

## 6. State Machine Design

### 6.1 Basic FSM Structure

```cpp
enum class State { IDLE, ACTIVE_A, ACTIVE_B };

void Update() {
    switch (current_state_) {
        case State::IDLE:
            if (start_condition) current_state_ = State::ACTIVE_A;
            break;
        case State::ACTIVE_A:
            if (done_condition) current_state_ = State::IDLE;
            break;
        case State::ACTIVE_B:
            if (done_condition) current_state_ = State::IDLE;
            break;
    }
    // Map state → output
    output_ = (current_state_ == State::ACTIVE_A);
}
```

### 6.2 Star Topology

```
        ACTIVE_A ──→ IDLE ←── ACTIVE_B
                      ↑↓
                   ACTIVE_C
```

**All states route through a central hub (e.g. IDLE).** No direct transitions between active states.

**Why?**
- **Not needed** — no use case requires jumping directly between active states
- **Not safe** — timer/state variables from the previous active state may still be stale
- **More complex** — more transition paths = more things to test and break

**Principle**: Not needed + not safe + more complex = **don't do it**.

### 6.3 Three-Phase Execution

Each cycle, the state machine executes in three phases:

| Phase | When | Purpose | Example |
|-------|------|---------|---------|
| **Transition** | Every cycle | Evaluate whether to change state | `if (condition) state = NEW_STATE` |
| **Entry** | Once, upon entering a new state | One-time initialization | `timer.Clear()` |
| **Do** | Every cycle while in this state | Continuous action | `timer.Update()` |

**Edge detection for state changes**: Compare `current_state != previous_state` — if different, a transition just occurred → run entry logic.

### 6.4 Edge Detection Module

Converts persistent ON/OFF signals into momentary edge signals:

```
OFF ──(input=ON)──→ OFF_ON ──(still ON)──→ ON
 ↑                    ↓(input=OFF)          ↓(input=OFF)
 └──(still OFF)── ON_OFF ←──(input=OFF)───┘
```

- **OFF_ON** (rising edge) = "just pressed" — exists for exactly 1 cycle
- **ON_OFF** (falling edge) = "just released" — exists for exactly 1 cycle
- **Purpose**: Pressing a button for 3 seconds should trigger once, not 300 times

---

## 7. Signal Flow Tracing Methodology

How to trace any signal from source to consumers in a wiring-file-based architecture:

### 7.1 Upstream Tracing (Where does the signal come from?)

```
Step 1: Search for the target module's instantiation (its construction/wiring)
Step 2: Read the parameter comments → each parameter = one input signal source
Step 3: The source format is "other_module.OutputMethod()"
Step 4: Search for THAT module's instantiation → repeat from Step 2
Step 5: Repeat until you reach a hardware driver or communication bus entry point
```

### 7.2 Downstream Tracing (Who consumes the signal?)

```
Step 1: Search for "module_name.OutputMethod()" across the wiring file
Step 2: Every match = one consumer → read context to identify which module and parameter
```

### 7.3 Search Tips

- Search `"instance_name,"` (with comma) → precisely locates the instantiation definition
- Search `"instance_name."` (with dot) → finds all consumers of this module's outputs
- Parameter comments (`/* param_name : */`) are key to understanding what each input is

### 7.4 Watch for Feedback Loops

Signal chains can form cycles:

```
Module_A.Output() → Module_B input
Module_B.Output() → Module_C input
Module_C.Output() → Module_A input  ← feedback loop!
```

This is common in control systems (e.g. vehicle stop detection → limp-home permission → sensor mode selection → back to stop detection). Execution order matters to avoid circular dependency issues.

---

## 8. Programming Mindset

### 8.1 Universal Programming Principles

- **Positional parameter matching** — constructor arguments are matched by order, not name
- **Dependency-ordered instantiation** — dependencies must be created before dependents
- **Loose coupling via `const &`** — modules read others' outputs through const references; cannot modify
- **Aggregate initialization by field order** — `{ val1, val2, { sub1, sub2 } }`
- **Shared `#include` ≠ direct relationship** — files are compiled independently
- **Generic tools carry no concrete values** — values injected from outside (dependency injection)
- **Read variable names in reverse** — `is_switch_turned_off_` = "has the switch ever been released?"
- **Inner class for namespace isolation** — `ClassName::EnumValue` prevents global name collisions

### 8.2 Sequential Logic vs Combinational Logic

| | Combinational | Sequential |
|---|---|---|
| **Requires memory?** | No | Yes (timer, state variable) |
| **Requires time?** | No | Yes (condition must persist) |
| **Example** | Reverse gear → lamp ON | Speed < threshold for 500ms → "stopped" |
| **Implementation** | `if/else` | Timer + Clear pattern or state machine |

### 8.3 Safety Override Pattern

```cpp
// The state machine runs normally (don't interfere with its logic)
// Only intercept the output at the very last step
if (safety_fault_detected) {
    output_ = SAFE_DEFAULT_VALUE;   // override
} else {
    output_ = state_machine_result; // normal operation
}
```

**Benefit**: State machine continues running in background → instant recovery when fault clears.

### 5.6 Hysteresis Pattern (Dead Zone Anti-Oscillation)

**Problem**: When a measured value hovers near a single threshold, output rapidly toggles between ON and OFF.

```
Sensor value:  ──────┐  ↑↓↑↓↑↓  ┌────── (oscillating near threshold)
Single threshold:  ───────────────────── (one line → rapid toggling)
Output:            OFF ON OFF ON OFF ON  ← BAD: relay chatters
```

**Solution**: Use TWO thresholds — a higher "on threshold" and a lower "off threshold" — creating a dead zone between them.

```cpp
template <typename T>
class Hysteresis {
 public:
    struct Config {
        T on_threshold;    // Value must rise ABOVE this to turn ON
        T off_threshold;   // Value must fall BELOW this to turn OFF
    };
    // on_threshold > off_threshold → "dead zone" in between

    void Update(T current_value) {
        if (state_ == false && current_value >= config_.on_threshold) {
            state_ = true;     // Turn ON (crossed upper threshold)
        }
        if (state_ == true && current_value < config_.off_threshold) {
            state_ = false;    // Turn OFF (crossed lower threshold)
        }
    }

    bool GetState() const { return state_; }

 private:
    Config config_;
    bool state_ = false;
};
```

**How the dead zone prevents oscillation**:
```
on_threshold:   ─────────────── 0.59 MPa ─────────────
                   DEAD ZONE (signal can oscillate here
                   without changing output state)
off_threshold:  ─────────────── 0.39 MPa ─────────────

Value rising:   0.3 → 0.4 → 0.5 → 0.6 → ON!  (only turns on at 0.59)
Value falling:  0.6 → 0.5 → 0.4 → 0.38 → OFF! (only turns off at 0.39)
Value hovering: 0.4 ↔ 0.5 ↔ 0.55 → NO CHANGE  (stuck in dead zone)
```

**Analogy**: Air conditioner thermostat
- ON threshold = 28°C ("start cooling when it gets this hot")
- OFF threshold = 24°C ("stop cooling when it gets this cool")
- Between 24-28°C: do nothing (prevents compressor from cycling on/off every second)

**Config vs Object**: Config is the **blueprint** (struct with threshold values); Object is the **runtime instance** (Hysteresis with internal state). Like a recipe (Config) vs the actual dish being cooked (Object). One Config can create multiple Objects with the same settings.

**Common use**: Brake lamp, fan control, pressure switches — anywhere a physical measurement toggles a relay or actuator.

### 5.7 Sensor Fallback with OR-Merge

**Problem**: Multiple sensors measure overlapping conditions. When one sensor's diagnostic fails, you can't just ignore that input — you must still make a decision using remaining sensors.

**Solution**: Check diagnostic validity per sensor; use only valid sensors in the final OR-merge.

```cpp
void Update() {
    // Step 1: Update each hysteresis independently
    brake_press_hysteresis_.Update(brake_pressure_.GetValue());
    pedal_rate_hysteresis_.Update(pedal_rate_.GetValue());
    collision_hysteresis_.Update(collision_pressure_.GetValue());

    // Step 2: Apply diagnostic mask (invalid sensor → treat as OFF)
    bool press_result   = brake_press_hysteresis_.GetState()
                          && brake_diag_normal_;         // diag OK?
    bool pedal_result   = pedal_rate_hysteresis_.GetState()
                          && pedal_diag_normal_;
    bool collision_result = collision_hysteresis_.GetState()
                            && collision_diag_normal_;

    // Step 3: OR-merge — any valid sensor says "brake" → output ON
    bool lamp_on = press_result || pedal_result || collision_result;

    output_ = lamp_on ? ON : OFF;
}
```

**Key insight**: The diagnostic flag acts as a **mask**. A sensor that's broken (diag = false) contributes `false` regardless of its hysteresis state. The OR-merge of remaining valid sensors still produces a correct result — the system degrades gracefully rather than failing completely.

**Contrast with redundant sensor (Section 5.2)**: Redundant sensors measure the **same** quantity and one replaces the other. Here, multiple sensors measure **different but related** quantities (pressure, pedal rate, collision) and any one is sufficient to trigger the output.

### 5.8 Gate/Door Flag Pattern (Signal Noise Filtering)

**Problem**: A simple `if (state_changed)` check triggers on **every** state transition, including transient intermediate states (e.g. gear shifting through a "DIRECT" transition state).

**Solution**: A boolean flag that acts as a **gate/door** with separate open and close conditions.

```cpp
// The flag has TWO independent control conditions:
bool gate_open_ = false;

void Update() {
    // === OPEN condition: met once → stays open ===
    if (!gate_open_) {
        if (off_duration >= threshold) {   // e.g. clutch disengaged long enough?
            gate_open_ = true;             // OPEN the gate
        }
    }

    // === CLOSE condition: met once → stays closed ===
    if (previous_state == target_end_state) {
        gate_open_ = false;               // CLOSE the gate (operation completed)
    }

    // === Gate is one of N conditions in a multi-condition check ===
    if (temperature >= min_temp           // condition 1
        && gate_open_                     // condition 2 (the gate)
        && previous == start_state        // condition 3
        && current == end_state) {        // condition 4
        // --- CRITICAL SECTION: only reached on valid events ---
        process_event();
    }

    previous_state = current_state;       // snapshot for next cycle
}
```

**Why not just `previous != current`?**

```
Timeline:  A (stable) → A_B_TRANS (transition) → B (stable)

Using previous != current:
  Tick N:   prev=A, cur=A_B_TRANS → different! → triggers (WRONG - still transitioning)
  Tick N+1: prev=A_B_TRANS, cur=B → different! → triggers AGAIN

Using gate + specific state comparison:
  Gate opens only after sufficient off-duration (filters short glitches)
  Check requires prev==A AND cur==B (specific pair, rejects transition states)
  Gate closes when prev==B (one tick after successful detection)
  → Triggers exactly ONCE, only on genuine complete transitions
```

**Key insight**: The close condition `previous == end_state` fires **one tick after** the detection (because previous lags by one cycle). This ensures the gate stays open during the detection tick but closes immediately after.

### 5.9 Histogram Binning (Event Classification)

**Problem**: Recording every individual event value is too much data for an embedded system. You need a summary.

**Solution**: Divide the value range into bins (buckets), count events per bin.

```
Value range:   0        T1        T2        T3        MAX
               |--Bin 0--|--Bin 1--|--Bin 2--|--Bin 3--|
               | Normal  | Mild    | Moderate| Severe  |
```

```cpp
// Typical implementation:
const float thresholds[] = {0.F, 150.F, 300.F, 450.F, FLT_MAX};
const size_t NUM_BINS = 4;

size_t bin_index = 0;
for (size_t i = 0; i < NUM_BINS; ++i) {
    if (value >= thresholds[i] && value < thresholds[i + 1]) {
        bin_index = i;
        break;
    }
}

// Increment persistent counter for that bin (with overflow protection)
uint32_t count = storage.GetValue(bin_index);
if (count < UINT_MAX) {
    storage.SetValue(count + 1, bin_index);
}

// Optionally: trigger alert for the most severe bin
if (bin_index == NUM_BINS - 1) {
    trigger_alert = true;   // Severe event → snapshot/alarm
}
```

**Design decisions**:
- **FLT_MAX as upper bound**: Ensures no value falls outside the bins (defensive programming)
- **UINT_MAX overflow protection**: Counter stops at max instead of wrapping to 0 (data preservation)
- **Alert only for worst bin**: Only the most severe events warrant real-time intervention; milder events are just recorded for later analysis

**Real-world analogy**: Hospital vital sign monitors classify heart rates into "normal / elevated / high / critical" ranges. Only "critical" triggers the alarm; all ranges are logged for the patient's chart.

### 5.10 Dual Output Pattern (Storage + Real-time Alert)

Some modules produce **two kinds of output** simultaneously:

| Output Type | Mechanism | Purpose | Data Lifetime |
|-------------|-----------|---------|---------------|
| **Statistical** | Write to non-volatile memory (MDO) | Long-term trend tracking | Survives power-off |
| **Real-time alert** | Boolean reference output (`OutputRef()`) | Immediate response trigger | RAM only (current cycle) |

```
Module runs → classifies event → writes to MDO counter (permanent)
                               → if severe: sets alert flag (transient)
                                             ↓
                                   Alert module reads flag → triggers snapshot
```

**When one output isn't enough**: Statistical storage answers "how often has this happened over the machine's lifetime?" while real-time alert answers "is it happening RIGHT NOW and do we need to act?"

---

## 9. Quick Reference Cards

### 🔧 Debounce

```
Problem: Sensor noise → rapid toggling near threshold
Solution: Timer + Clear pattern
  Condition met   → timer increments
  Condition broken → timer resets to 0
  Timer ≥ threshold → confirmed, output true
Rule: Sustained for long enough = real. Momentary = noise.
```

### 🔧 Redundant Sensor Switchover

```
Same shaft, two sensors (A + B):
  Both OK → use max(A, B)
  One failed → use the healthy one
  Both failed → estimated value (limp-home)
Principle: Always have a fallback. Never strand the machine.
```

### 🔧 State Machine

```
Structure: switch-case + enum states
Star topology: All states route through central IDLE
Three phases: transition (evaluate) → entry (one-time init) → do (continuous)
One rule: Not needed + not safe + more complex = don't do it
```

### 🔧 Edge Detection

```
OFF_ON = rising edge (just pressed, 1 cycle only)
ON_OFF = falling edge (just released, 1 cycle only)
Purpose: "Press 3 seconds → trigger once, not 300 times"
```

### 🔧 Signal Object

```
Signal = value + validity (VALID / INVALID)
Unpack: value = signal.GetValue()
Logic: result = (value >= threshold)
Repack: output = Signal(result, VALID)
Rule: Business logic always outputs VALID. INVALID comes from hardware.
```

### 🔧 Loose Coupling

```
Module constructor params = all inputs
Module output methods (OutputRef()) = appear in OTHER modules' constructors
Modules don't #include each other — only the wiring file knows who connects to whom
Find inputs: read own instantiation
Find outputs: search "instance_name." to see who references it
```

### 🔧 Gate/Door Flag

```
Problem: Simple "state changed" check triggers on transient intermediate states
Solution: Bool flag with SEPARATE open and close conditions
  Open:  off_duration >= threshold → open gate (noise filtered)
  Close: previous == target_end_state → close gate (event completed)
  Use:   gate_open_ is ONE of N conditions in a multi-condition AND check
Key: Close fires one tick AFTER detection (previous lags by one cycle)
Rule: Specific state comparisons beat generic "!= " checks
```

### 🔧 Histogram Binning

```
Problem: Too many individual events to store on embedded system
Solution: Divide value range into bins, count events per bin
  Thresholds: [0, T1, T2, T3, FLT_MAX] → 4 bins
  FLT_MAX = "no upper limit" (defensive — no value escapes all bins)
  UINT_MAX check before ++count (prevent wrap-to-zero)
  Worst bin triggers real-time alert; all bins stored in non-volatile memory
Analogy: Heart rate monitor — bins = normal/elevated/high/critical
```

### 🔧 Dual Output (Storage + Alert)

```
Same module produces two outputs:
  1. MDO write → permanent counter (survives power-off) → trend analysis
  2. Bool OutputRef() → transient flag (RAM only) → triggers snapshot/alarm
When? Statistical "how often over lifetime" + real-time "happening NOW?"
```

### 🔧 Hysteresis + Timer Combined (Two-Dimensional Debounce)

```
Problem: Hysteresis alone still reacts instantly; Timer alone has one threshold
Solution: Combine both — Hysteresis filters VALUE noise, Timer filters TIME noise
  Design B (preferred): Clear timer while hyst=true → timer counts only AFTER condition drops
  Result: condition must change AND persist for N cycles before acting
  Both STATE_OFF→ON and STATE_ON→OFF use symmetric timer Clear logic
Use case: Warning lamps, actuator control — anything needing robust anti-oscillation
```

### 🔧 How They Fit Together

```
Redundant sensors → reliable raw data
  → Debounce filter → confirmed stable signal
    → Hysteresis → oscillation-free ON/OFF decision
      → Hysteresis + Timer → two-dimensional confirmation
        → State machine → mode/state decisions
          → Edge detection → one-time trigger events
            → Signal objects → value + trustworthiness throughout

Goal: On a 50-ton machine, never make a wrong decision
      because of one sensor failure or one signal glitch.
```

### 🔧 Hysteresis (Dead Zone)

```
Problem: Single threshold + noisy signal → output chatters ON/OFF rapidly
Solution: Two thresholds (on_thresh > off_thresh) with dead zone between
  Rising:  crosses on_thresh  → ON
  Falling: crosses off_thresh → OFF
  Between: NO CHANGE (dead zone absorbs oscillation)
Analogy: AC thermostat — start cooling at 28°C, stop at 24°C
Config vs Object: Config = recipe (thresholds), Object = runtime instance (with state)
```

### 🔧 Sensor Fallback (OR-Merge + Diagnostic Mask)

```
N sensors measuring related quantities (not the same quantity):
  Each: hysteresis_result AND diagnostic_normal → masked result
  Final: masked_1 OR masked_2 OR ... OR masked_N → output
  Broken sensor (diag=false) → always contributes false → doesn't corrupt result
  Remaining valid sensors still produce correct output → graceful degradation
Different from Redundant Sensor: redundant = same quantity, one replaces other
                                  fallback  = different quantities, any one suffices
```

### 🔧 Signal Type Inheritance

```
SignalObject<T>    → base: value_ + GetValue()
  NumericSignal<T> → adds numeric semantics
    Pressure / Temperature / Rpm → domain-specific concrete types
Module sees: const Pressure& → calls .GetValue() → gets float
Module doesn't know: AD conversion, filter, calibration, sensor hardware
Rule: #include only what you DIRECTLY use — don't skip inheritance levels
```

---

## 10. Common Signals in Vehicle/Machine Control Systems

| Signal | Source | Purpose |
|--------|--------|---------|
| Travel direction (gear position) | FNR joystick / shift lever | Operator's intended direction |
| Vehicle velocity / speed | Output shaft RPM × tire radius | Actual movement speed |
| Engine / shaft RPM | Speed sensor (pulse) | Rotational speed control |
| Oil temperature | Temperature sensor | Thermal protection |
| Brake stroke / pedal position | Displacement sensor | Braking intent |
| Brake pedal rate | Calculated from brake stroke over time | Braking urgency (how fast pedal is pressed) |
| Brake pipe pressure | Pressure sensor (analog) | Actual braking force |
| Collision detection pressure | ODS pressure sensor | Operator presence / collision detection |
| Parking brake | Switch | Is braking locked? |
| Key switch / ignition | Rotary switch | START / ACC / IG states |
| System pressure | Pressure sensor | Hydraulic system health |
| Diagnostic normal flag | Diagnostic judge module (bool) | Is the associated sensor trustworthy? |

---

## 11. Vocabulary for Technical Discussions

| Concept (English) | Plain Explanation |
|-------------------|-------------------|
| Safety-critical system | Software where bugs can cause physical harm |
| Cooperative scheduling | Each task runs briefly, returns control voluntarily; no preemption at app level |
| Cross-compilation | Compile on PC, code runs on MCU (different architectures) |
| Debounce filtering | Condition must persist for X ms before it counts |
| Sensor redundancy | Two sensors measure the same thing; if one fails, use the other |
| Limp-home / graceful degradation | Total failure → safe fallback, not complete shutdown |
| Finite State Machine (FSM) | System with defined states and transition rules |
| Edge detection | Convert sustained signal to single-cycle pulse |
| Dependency injection | Pass dependencies into constructor, don't create them internally |
| Composite pattern | Container is the same type as its contents (tree structure) |
| Star topology FSM | All states transit through a central hub; no shortcuts between active states |
| Signal validity | Every value tagged with "trustworthy or not" |
| Polling | Check condition every cycle instead of waiting/blocking |
| Overflow protection | Clamp at max value instead of wrapping to 0 |
| Single-responsibility pipeline | Each module does one step; complex functions = chain of simple modules |
| Const reference (`const &`) | Read-only access; no copying; no modification allowed |
| Gate/door flag pattern | Bool that opens on one condition and closes on a different condition; filters transient noise |
| Histogram binning | Classify continuous values into discrete bins and count per bin; compact statistical summary |
| FLT_MAX sentinel | Use float max as "no upper limit" boundary; ensures all values map to some bin |
| Dual output module | Module writes persistent storage (statistics) AND provides real-time output (alert) simultaneously |
| Reference binding vs value copy | `member_(obj)` = live alias; `member_(obj.GetValue())` = frozen snapshot; determines real-time vs historical view |
| Transition state | Intermediate state during a mode change (e.g. shifting gears); must be filtered to avoid false triggers |
| ErrorSnap / snapshot trigger | On severe event, capture full system state for post-mortem analysis (like a flight recorder) |
| Hysteresis | Two-threshold switching: ON above upper, OFF below lower; dead zone between prevents oscillation |
| Dead zone | Gap between on_threshold and off_threshold where no state change occurs; absorbs signal noise |
| OR-merge with diagnostic mask | Combine multiple sensor results with OR; each ANDed with its diagnostic flag first; broken sensor = silent |
| Signal type inheritance | `SignalObject<T>` → `NumericSignal<T>` → concrete type; GetValue() at base, domain semantics at leaf |
| Only-include-what-you-use | `#include` the type you directly reference, not its grandparent; keeps dependency chains clean |
| Bool→enum conversion | Generic tools return `bool`; business layer converts to domain-specific enum (`ON/OFF`, `FORWARD/REVERSE`) at the boundary |
| Config vs Object | Config = struct with parameter values (blueprint); Object = runtime instance with internal state (machine) |
| .cs/.h mirror pattern | PC-side tool (.cs) mirrors embedded class (.h) using attributes like `[CppType]`; enables GUI parameter editing |
| Interpolation table | Lookup table mapping raw sensor input (voltage) to physical quantity (pressure); typically 6+ calibration points |

---

## 12. Build System & Compilation

### 12.1 Make: Timestamp-Based Rebuild

```
Make's core logic (one sentence):
"If the target file is older than the source file, regenerate it."

Rule format:
    target : dependencies
        command

Decision flow:
    .o newer than .cc  → skip ("Nothing to be done")
    .cc newer than .o  → recompile
    .o doesn't exist   → must compile
```

### 12.2 The .h Dependency Trap

Many embedded project makefiles only declare `.cc → .o` dependencies, **not `.h → .o`**:

```makefile
# What the makefile says:
%.o : %.cc              # only depends on .cc

# What's actually true:
%.o : %.cc config.h types.h signals.h    # also depends on .h files
```

**Symptom**: You edit a `.h` file, run `make` → "Nothing to be done" → your change has no effect.

**Solution**: `make clean` + `make` (delete all `.o` files, force full rebuild).

**Modern fix**: Use `-MMD` flag to auto-generate `.d` dependency files. But many legacy embedded projects don't do this.

### 12.3 Include Path Organization

Two common patterns for header file organization:

```
Pattern A: Centralized (simple)
    middleware/include/         ← all headers in one directory
    -I middleware/include

Pattern B: Distributed (this project)
    middleware/framework/      ← manager.h, module_interface.h
    middleware/signal_object/  ← velocity.h, pressure.h, bool_signal.h
    middleware/type/           ← type definitions
    utility/hysteresis/        ← hysteresis.h
    utility/timer/             ← increment_timer.h
    utility/array_container/   ← array_container.h
    ... (20+ subdirectories)

    -I middleware/framework -I middleware/signal_object -I middleware/type
    -I utility/hysteresis -I utility/timer -I utility/array_container ...
```

**Lesson**: If `#include "file.h"` fails with "cannot open source file", the fix is always: find where that `.h` actually lives, add its directory to the `-I` list.

### 12.4 Cross-Compilation Command Anatomy

```
cxrh850            ← Compiler (Green Hills for RH850 MCU)
  -c               ← Compile only, don't link (produce .o)
  -Onone           ← No optimization (easier debugging)
  --no_rtti        ← No runtime type info (saves memory)
  -pack=none       ← No struct packing
  --stdl -gcc      ← Standard library + GCC compatibility mode
  -I path1         ← Include search path (repeat for each directory)
  -I path2
  source.cc        ← Input file
  -o output.o      ← Output file
```

---

## 13. Common Coding Mistakes (First-Time Embedded C++ Writer)

### 13.1 Syntax Mistakes

| Mistake | Example | Fix |
|---------|---------|-----|
| Missing comma between parameters | `func(a b)` | `func(a, b)` |
| Missing semicolon after class | `class X { }` | `class X { };` |
| Case sensitivity | `void update()` | `void Update()` — must match declaration |
| Missing `#endif` | `#ifndef` without closing | Always pair `#ifndef` / `#endif` |
| Nested function definition | Method defined inside another method's body | Each method must be a separate definition |
| Missing `Class::` prefix in .cc | `void Update()` | `void Function::Update()` |
| Missing `void` return type | `Function::Update() {}` | `void Function::Update() {}` |
| Missing `const` on method impl | `Signal& OutputRef()` in .cc | Must match `.h`: `Signal& OutputRef() const` |
| `::` vs `:` confusion | `class Func::public Base` | `class Func : public Base` — `::` is scope, `:` is inheritance |
| Include guard typo | `#ifndef MY_MODLE_H` | Guard name should match filename — typo compiles but breaks protection |
| Extra `()` on member variable | `return member_()` | `return member_` — not a function call |

### 13.2 Type Confusion Mistakes

| Mistake | Why It's Wrong | Rule |
|---------|---------------|------|
| `HysteresisSint32` for `float32` input | Type mismatch: Hysteresis template type must match `GetValue()` return type | Check input signal's `.h` → `NumericSignal<float32>` → use `HysteresisFloat32` |
| `GetValue()` on Hysteresis | Hysteresis has no `GetValue()` | SignalObject → `GetValue()` ; Hysteresis → `GetState()` |
| `EnumElectricalOnOffState(output, ...)` | `EnumXxx` is the raw enum class, not SignalObject | `EnumXxx` = enum values ; `Xxx` (without Enum) = SignalObject wrapper |
| `ElectricalOnOffStateType` as return type | `...Type` is the enum typedef, not the signal object | OutputRef returns `const ElectricalOnOffState&` (the SignalObject) |
| `EnumXxx::XxxType::VALUE` (extra layer) | Enum values are directly accessible on the wrapper class | `EnumElectricalOnOffState::OFF` — not `::ElectricalOnOffStateType::OFF` |
| Bare `VALID` without namespace | `VALID` is not a global constant | Must write `signal_object::EnumSignalState::VALID` |
| Hysteresis.Update(config) | Update() needs **current sensor reading**, not config | Config sets thresholds at construction; Update() takes live measurement value |
| Manual threshold comparison | `if (pressure > hysteresis.On_thresh)` | Hysteresis is a black box: `Update(value)` → `GetState()`. Never access internals |

### 13.3 Framework API Mistakes

| Mistake | Reality | Lesson |
|---------|---------|--------|
| `manager.RegisterModule(*this)` | `manager.RegistModule(*this)` | API names can be non-standard English; always check the actual framework `.h` |
| Double-nesting namespace | `namespace X { namespace X {` | One namespace block per scope. Nested identical namespaces compile but create a wrong hierarchy |
| OutputRef defined inside Update() | Method defined as nested function | Every method must be a separate top-level definition in the `.cc` file |

### 13.4 Persistent Weak Points (Found Across Multiple Practice Modules)

```

Persistent bugs that appear across multiple practice sessions:
- :: vs : (inheritance uses single colon, scope uses double)
- Forgetting void return type on .cc method definitions
- Forgetting Function:: prefix on .cc method definitions
- Missing semicolon after class/struct closing brace
- Duplicate namespace nesting
All resolved by Day 3 practice.
```

### 13.5 Day 4 Practice: Full Rewrite with Hysteresis + IncrementTimer (April 24)

**Task**: Clear all code, rewrite parking_lamp from scratch with both Hysteresis + IncrementTimer.
**Note**: First submission used yesterday's code without rewriting — doesn't count. Below is the real rewrite result.

**Result: 3 errors (1 compile + 2 logic) → fixed → BUILD SUCCESS**

**Compile Error: Missing `&` on Manager parameter**
```
Wrong:  framework::Manager manager    // Value copy — Manager is non-copyable
Right:  framework::Manager& manager   // Pass by reference

Manager is a framework singleton — always pass by reference, never copy.
```

**Logic Error 1: Local variable shadowing member variable (NEW error type!)**
```
Wrong:  signal_object::ElectricalOnOffState parking_lamp_output_(output, ...);
        ^ This DECLARES a new local variable with same name as member
        ^ Local destroyed at function end; member stays at initial value OFF

Right:  parking_lamp_output_ = signal_object::ElectricalOnOffState(output, ...);
        ^ This ASSIGNS to the existing member variable

Rule: With type name = declaration. Without type name = assignment.
Compiler does NOT warn (variable shadowing is legal C++).
Mnemonic: "In Update(), assign without type name"
```

**Logic Error 2: Missing timer_.Update()**
```
Timer three-step pattern must be complete:
1. timer_.Update();       ← MISSED! Without this, counter never increments, IsTimeUp() always false
2. condition → timer_.Clear();
3. timer_.IsTimeUp() → output decision

Mnemonic: "Timer three steps: Update, Clear, IsTimeUp — skip none"
```

**Correctly written (progress confirmed)**:
```
✅ Clear condition uses || not && (previous logic error fixed)
✅ All #includes correct including increment_timer.h
✅ Config with both Timer and Hysteresis configs
✅ Initializer list for Timer correct
✅ All Day 1-3 persistent bugs eliminated (::, namespace, void, Function::, class;)
```

**Error nature evolution**:
```
Day 1-3: Syntax/structural errors (compiler catches them)
Day 4:   1 syntax slip + 2 logic errors (compiler CANNOT catch — more dangerous)

Variable shadowing = classic C++ trap:
├── Compiler gives no error
├── Behavior completely wrong (member never updated)
├── Must understand "declaration vs assignment" to avoid
└── Common bug source in real production code
```

These mistakes were made on BOTH reverse_buzzer and parking_lamp — not one-time typos:

1. ElectricalOnOffState three-layer type chain confusion
   → Enum wrapper (EnumElectricalOnOffState) vs SignalObject (ElectricalOnOffState) vs typedef (ElectricalOnOffStateType)
   → Persistent: appeared in both Module #1 and Module #2

2. Enum value access path errors
   → Forgetting signal_object:: prefix, adding extra ::Type:: layer, using bare VALID
   → Root cause: not fully understanding the typedef → class → enum hierarchy

3. Include guard spelling
   → MODE instead of MODEL (reverse_buzzer), PARATICE instead of PRACTICE (parking_lamp)
   → Rule: guard name = filename in ALL_CAPS + _H

Interview tip: These persistent mistakes show self-awareness and systematic debugging approach.
When asked "What mistakes have you made?", discussing these shows growth mindset.
```

### 13.5 The "Read vs Write" Gap

```
What you think you know from reading:     What you discover when writing:
────────────────────────────────         ────────────────────────────────
"Config is defined in .h"            →   Forgot to #include the header
"Constructor uses initializer list"  →   Forgot comma between parameters
"Hysteresis checks thresholds"       →   Don't know GetState() vs GetValue()
"Output uses ElectricalOnOffState"   →   Confused Enum prefix vs SignalObject
".cc methods need class prefix"      →   Forgot to write Function::

Conclusion: Reading = understanding "what"
            Writing = knowing "how"
            The gap is only bridged by doing.
```

## 14. Constructor Initializer List — Deep Understanding

### 14.1 Why const& Members MUST Use Initializer Lists

```
C++ Rule: A const reference must be bound at the moment of creation.
          It cannot be "created first, assigned later."

❌ Assignment in body (FAILS for const&):
   Function::Function(const Config& config, ...) {
       config_ = config;    // ERROR: config_ is const&, cannot reassign
   }

✅ Initializer list (the ONLY way):
   Function::Function(const Config& config, ...)
       : config_(config),   // Bound at creation time
         pressure_(p),
         ...
   { }

This is not a style preference — it's a language rule.
```

### 14.2 Initializer List vs Constructor Body

```
Initializer list (after colon, before braces):
├── Runs BEFORE the constructor body
├── This is where members are CREATED
├── const& members can ONLY be initialized here
├── Non-default-constructible objects MUST be initialized here
└── HysteresisFloat32 config, SignalObject initial values go here

Constructor body (inside braces):
├── Runs AFTER all members are already created
├── Used for operations, not initialization
├── manager.RegistModule(*this) goes here
└── Cannot bind const references here (too late)
```

### 14.3 What Goes Where (Cheat Sheet)

```
Initializer list:                          Constructor body:
─────────────                              ──────────────
config_(config)                            manager.RegistModule(*this)
sensor_ref_(sensor)                        // any registration / setup
hysteresis_(config.hysteresis_config)       // logging, validation
output_(initial_value, VALID)              // nothing else usually
```

## 15. Embedded Type System — The Three-Layer Pattern

### 15.1 Signal Type Chain (enum → class → SignalObject)

```
Layer 1: Raw enum (the actual values)
   enum ElectricalOnOffStateType { OFF, ON };

Layer 2: Wrapper class (container for the enum)
   class EnumElectricalOnOffState {
       enum ElectricalOnOffStateType { OFF, ON };
   };

Layer 3: SignalObject typedef (value + validity state)
   typedef SignalObject<EnumElectricalOnOffState::ElectricalOnOffStateType>
       ElectricalOnOffState;

Usage rules:
├── Function parameters, member types, return types → Layer 3 (ElectricalOnOffState)
├── Enum values in if/switch/assignment → Layer 2 (EnumElectricalOnOffState::ON)
├── SignalObject constructor → Layer 2 value + EnumSignalState::VALID/INVALID
└── Never use Layer 1 directly (it's inside Layer 2)
```

### 15.2 Include Dependency Chain (Transitive Includes)

```
Your .h  →  #include "electrical_on_off_state.h"
                →  #include "signal_object.h"
                        →  #include "enum_signal_state.h"  ✅ auto-included

Rule: If A includes B and B includes C, then A can use C's types.
      No need to redundantly #include enum_signal_state.h.

Practical tip: #include the highest-level type you need.
               Lower-level headers come along for free.
```

### 15.3 HysteresisFloat32 — Tool, Not Struct

```
Common misconception: Hysteresis is a struct with thresholds you compare manually.

❌ Wrong (treating it as a struct):
   if (pressure > hysteresis.On_thresh) { ... }

✅ Right (treating it as a black-box tool):
   hysteresis.Update(pressure.GetValue());    // Feed value in
   hysteresis.GetState();                     // Get bool result out

Key distinction:
├── SignalObject → GetValue() (extract the wrapped value)
├── HysteresisFloat32 → GetState() (get the judgment result, returns bool)
├── Hysteresis handles thresholds internally — you don't access them
└── Config only sets up thresholds at construction time
```

### 15.4 Enum Value Access Path — Quick Reference

```
ON/OFF enum values       → signal_object::EnumElectricalOnOffState::ON / ::OFF
VALID/INVALID status     → signal_object::EnumSignalState::VALID / ::INVALID
Construct SignalObject   → signal_object::ElectricalOnOffState(enum_value, state)

Memory rule: "Enum-prefixed class :: value" for enum access
             "No-Enum class" for constructing the SignalObject wrapper

Common mistakes:
├── ElectricalOnOffStateType::OFF   ← wrong (Type is a typedef, not a scope)
├── EnumElectricalOnOffState::ElectricalOnOffStateType::OFF  ← wrong (extra layer)
├── just VALID                       ← wrong (needs full path with namespace)
└── EnumElectricalOnOffState::OFF   ← correct ✅

Two-phase Hysteresis usage pattern:
├── Construction:  HysteresisFloat32 hysteresis_(config.hysteresis)  ← Config sets thresholds
├── Runtime:       hysteresis_.Update(sensor.GetValue())             ← Feed live measurement
├── Result:        hysteresis_.GetState()                            ← Get bool judgment
└── Same pattern as IncrementTimer: Construct(Config) → Update() → IsTimeUp()
```

## 16. Progress Tracking — Independent Module Writing

### 16.1 Module #1: reverse_buzzer (with reference code nearby)

```
Date: April 17, 2026
Errors made: 12 (syntax: 5, type: 4, framework: 2, structure: 1)
Key struggles: #include awareness, Type vs SignalObject, RegistModule spelling
Result: Compiled successfully after corrections
```

### 16.2 Module #2: parking_lamp (from pure memory, no references)

```
Date: April 21, 2026
Challenge: Copilot & IntelliSense disabled, no reference code
Errors made: 17 (.h: 5+3, .cc: 7+2)

Original errors (found during first write):
├── :: vs : confusion (namespace access vs inheritance)
├── const& initialization by assignment instead of initializer list
├── Mixing declaration and method call in constructor
├── Missing void return type
├── Treating Hysteresis as a struct (manual threshold comparison)
├── Variable declared at global scope instead of local
├── Missing const on method implementation
├── Extra parentheses on member variable (treating it as function call)

Additional errors found in code review:
├── ⑬ Hysteresis::Update() parameter confusion (passed Config instead of sensor reading)
├── ⑭ Enum path with extra layer (ElectricalOnOffStateType::OFF → just EnumElectricalOnOffState::OFF)
├── ⑮ Bare VALID without signal_object::EnumSignalState:: prefix
├── ⑯ Include guard typo PARATICE → PRACTICE
├── ⑰ Same-name namespace nested twice (namespace X { namespace X { ... } })

Errors NOT repeated from reverse_buzzer:
├── #include list → all correct ✅
├── Class semicolon → correct ✅
├── Constructor parameter commas → correct ✅
├── RegistModule spelling → correct ✅
├── Update() capitalization → correct ✅
├── OutputRef as independent method → correct ✅

Trend: Error types shifted from "basic syntax" to "deeper concepts"
       (initializer list semantics, type system understanding, API usage patterns)
```

### 16.3 Module #2 Day 2: parking_lamp rewrite (April 22, 2026)

```
Challenge: Files cleared, rewrite from memory — second attempt
Errors made: 20 (.h: 13, .cc: 7)
Error count went UP (17→20), but error QUALITY improved significantly

Major improvements (Day 1 bugs now FIXED):
├── Hysteresis.Update(pressure.GetValue()) → parameter correct ✅ (was: passed Config)
├── EnumElectricalOnOffState::ON → enum path correct ✅ (was: extra ::Type:: layer)
├── signal_object::EnumSignalState::VALID → full path ✅ (was: bare VALID)
├── ElectricalOnOffState(output, state) → construction correct ✅
├── Include guard spelling → PRACTICE correct ✅ (was: PARATICE)
├── Update() core logic: entirely correct ✅

Persistent bugs (appeared 2+ times — targeted practice needed):
├── :: vs : for inheritance (3rd occurrence)
├── Namespace double-nesting (3rd occurrence)
├── Missing void return type (3rd occurrence)
├── Missing Function:: prefix on OutputRef (3rd occurrence)
├── Class closing }; semicolon (2nd occurrence)

New concept gaps exposed:
├── const placement: added "const" after parameter names (syntax error)
│   Rule: const goes BEFORE the type, not after the variable name
├── Constructor params vs member variables: included hysteresis and output as params
│   Rule: params = injected from outside; members = constructed internally
├── Namespace classification confusion (framework:: vs signal_object:: vs utility::)
├── struct body: used () instead of {}
├── #include missing quotes and wrong filename

Trend: Core logic mastered → errors now in "scaffolding" (syntax, structure, naming)
       Like piano: melody is correct, fingering still needs polish

Self-correction process:
├── 1st submission: 20 errors
├── 2nd submission: fixed 14 errors independently, 8 remaining
├── 3rd submission: all fixed → compiled successfully ✅
├── Self-fixed items show concept internalization (namespace, inheritance syntax, etc.)
├── Needed-help items reveal persistent gaps (const placement, struct semicolons, :: vs .)

Result: Compiled to .o successfully ✅
```

### 16.4 Daily Repetition Practice (Started April 22, 2026)

```
Method:
├── Clear .h and .cc files each morning
├── Rewrite from memory only — no reference code
├── Compile to verify → record errors
├── Track which errors are new vs repeated

Purpose:
├── Build muscle memory for API usage, #include order, initializer list syntax
├── Expose blind spots — getting stuck = not truly understood yet
├── Shift from "can read" to "can write" (interview differentiator)
└── Interviewable insight: "I practice writing embedded modules from memory daily
    to bridge the read-write gap"

Error Evolution (interview talking point):
├── Module #1 errors: basic syntax (missing semicolons, wrong case, missing includes)
├── Module #2 Day 1: deeper concepts (type system, API patterns, initialization rules)
├── Module #2 Day 2: core logic correct, errors shifted to syntax scaffolding
├── Module #2 Day 3: only 6 errors in first draft, all 5 persistent bugs fixed ✅
├── Trend: 17 → 20 → 6 errors (65% reduction over 3 days)
├── Persistent errors: identified, tracked, and eliminated through daily practice
└── Shows: systematic learning, self-awareness, continuous improvement
```

### 16.5 Module #2 Day 3: parking_lamp rewrite (April 23, 2026)

```
Errors in first draft: 6 (down from 17 on Day 1)
All 5 persistent bugs from Day 1-2 now FIXED:
├── : vs :: for inheritance ✅
├── Namespace double-nesting ✅
├── Missing void return type ✅
├── Missing Function:: prefix ✅
├── Class closing }; semicolon ✅

Remaining errors (minor):
├── Missing .h extension on #include "kcommon"
├── Missing const on BoolSignal input (parameter + member)
├── Wrong #include (signal_object.h instead of module_interface.h)
├── Missing _ suffix on member variable in Update()
├── GetState() vs GetValue() on BoolSignal
├── Extra semicolons after function definitions

Result: Fixed const inconsistency → compiled successfully ✅
```

### 16.6 IncrementTimer — Second Tool Pattern (Learned April 23, 2026)

```
Source module: afjs_lever_stroke_cross_check_diagnosis_model
Purpose: Cross-check diagnosis for redundant sensors using debounce timer

IncrementTimer vs HysteresisFloat32:
├── Hysteresis: value-based filtering (threshold hysteresis)
│   Update(sensor_value) → GetState() → bool
├── Timer: time-based filtering (debounce)
│   Update() → Clear() → IsTimeUp() → bool

IncrementTimer three-step pattern:
  1. timer_.Update();           // Count up every cycle
  2. timer_.Clear();            // Reset when condition clears
  3. timer_.IsTimeUp();         // Check if timeout reached

New patterns learned from this module:
├── Ring buffer for sensor history smoothing
├── Fault masking (don't check if upstream is already failed)
├── Fault latching (once failed, stay failed — fail-safe)
├── Non-reference members (bool, float32, arrays for internal state)
```

### 16.7 Diagnostic/Fault Handling Architecture (Learned April 23, 2026)

```
Source modules: fault_state_detection_model, diagnosis_check_model

Four-State Fault Machine:
  EnumFailureState::NO → CHECK → OCCUR → RECOVER → NO
  ├── NO:      Normal operation
  ├── CHECK:   Anomaly detected, awaiting confirmation (debounce)
  ├── OCCUR:   Fault confirmed
  └── RECOVER: Anomaly cleared, awaiting recovery confirmation

Three-Layer Architecture:
  Layer 1 (Hardware): Sensor voltage checks, wiring diagnostics, CAN timeouts
  Layer 2 (Diagnostic): FailureDetector manages state machine + debounce + masking
  Layer 3 (Reporting): 250+ failure codes → CANape display + diagnostic logs

Third Tool Pattern — FailureDetector:
  HysteresisFloat32  → Update(value)  → GetState()   → bool         "threshold?"
  IncrementTimer     → Update()       → IsTimeUp()   → bool         "timeout?"
  FailureDetector    → Update(bool)   → SignalRef()   → 4-state      "fault confirmed?"

Template Method Pattern in fault_state_detection_model:
  ├── Base class defines Update() with switch-case state machine
  ├── virtual IsFaultCondition() — subclass overrides "what is a fault"
  ├── virtual IsRecoverCondition() — subclass overrides "what is recovery"
  └── Separates "how to manage states" from "what counts as a fault"

Fault Masking:
  ├── FaultTransitionMaskOrder suppresses diagnostics during unreliable conditions
  ├── Examples: cold start, low voltage, upstream CAN failure
  └── Principle: "Don't diagnose when your inputs are unreliable"

Design Patterns Identified (7 total):
  Composite, DI, Strategy, State Machine, Template Method, Observer, Star Topology FSM
```

### 16.8 Cross-Check Module Deep Dive (April 23, 2026)

```
Key insights from line-by-line analysis of afjs_lever_stroke_cross_check_diagnosis:

1. Negated mask logic: mask = !(all_conditions_ok)
   └── Bracket = "safe to check?", negation = "need to mask?"
   └── De Morgan: !(A && B && C) = !A || !B || !C = "any one abnormal → mask"

2. Range-based cross-check (not direct comparison):
   └── Direct |s1-s2| comparison → false positives during fast lever movement
   └── Solution: compare s2 against s1's historical range [min,max of last 3 cycles]
   └── "Allow being slow to follow, but not failing to follow"
   └── [0] excluded from range (too new, may contain noise spike)

3. Timer order: Update() THEN Clear() (never reversed):
   └── Correct: 0→+1→Clear→0 (normal=0) vs 0→+1 (abnormal=1) → distinguishable
   └── Reversed: 0→Clear→0→+1 (normal=1) vs 0→+1 (abnormal=1) → indistinguishable!

4. SignalObject unified construction pattern:
   └── ALL signal objects: (value, signal_state)
   └── BoolSignal(false, VALID), ElectricalOnOffState(::OFF, VALID), Pressure(0.0F, VALID)
   └── bool = C++ keyword (no namespace), enums = project-defined (need full path)
   └── Design: signal carries both "what is the value" AND "is this value trustworthy"
```



---

## 17. CAN/J1939 Communication Protocol (For Code Reading)

### 17.1 CAN Bus Basics

```
CAN = Controller Area Network — the standard internal communication bus
      in vehicles, construction equipment, and industrial machines.

One CAN bus   = a group chat channel
Each ECU      = a participant in the chat
Each message  = a broadcast message with a "topic ID"

Key characteristics:
├── Broadcast: sender does NOT specify recipient → all ECUs on the bus receive it
├── Selective listening: each ECU decides which topic IDs to process
├── Max 8 bytes per frame (64 bits)
├── Priority: lower CAN ID = higher priority (bit-level arbitration)
└── Differential pair wiring: noise-resistant (CAN-H vs CAN-L)
```

### 17.2 CAN Frame Structure

```
┌──────────────────┬──────────────────────────────────┐
│  CAN ID (29-bit) │  Data (up to 8 bytes)            │
│  "topic number"  │  [B0, B1, B2, B3, B4, B5, B6, B7]│
└──────────────────┴──────────────────────────────────┘

CAN ID = message topic identifier (e.g., 0x00FEF500 = Ambient Conditions)
Data   = up to 8 bytes = 64 bits, can pack multiple signals into one frame
```

### 17.3 J1939 = Application Layer on Top of CAN

```
CAN   = envelope format (size, address fields)  → defines HOW to send 8 bytes
J1939 = letter content format (which line = what) → defines WHAT the 8 bytes mean

J1939 is the industry standard for heavy vehicles and construction equipment.
Used by: trucks, buses, construction machines, agricultural equipment, marine engines.
```

### 17.4 PGN and SPN — Message & Signal Identification

```
PGN (Parameter Group Number) = message topic number
  Example: PGN 0xFEF5 = 65269 = "Ambient Conditions"

SPN (Suspect Parameter Number) = individual signal number within a PGN
  Example: SPN 108 = Barometric Pressure, SPN 171 = Ambient Temperature

One PGN (message) contains multiple SPNs (signals):
  PGN 65269 = SPN 108 (pressure) + SPN 171 (temperature) + others

The PGN↔SPN mapping is defined in the J1939 standard — lookup table, no formula.
```

### 17.5 Physical Value Conversion (Core Formula)

```
physical_value = raw_value × resolution + offset

Examples:
  Barometric pressure: 200 × 0.0005 + 0     = 0.1 kPa
  Temperature:         9600 × 0.03125 + (-273) = 27°C

Why resolution exists:
  CAN bus can only transmit integers (raw bytes).
  Resolution converts between floating-point physical values and integer bus values.
  It's like currency exchange: "1 unit on the bus = 0.0005 kPa in real life"
```

### 17.6 Valid Range (J1939 Special Values)

```
J1939 reserves high-end values as error codes (for uint8):
  0–250    = valid measurement data
  251      = parameter not available
  252      = error indicator
  253      = reserved
  254      = parameter abnormal
  255      = not requested

In config: valid_range = {250, 0} means raw values 0–250 are VALID.
If raw > 250 → signal state = INVALID (it's an error code, not real data).
```

### 17.7 Byte Order (Endianness)

```
Big-endian:    high byte first (human reading order): 0x22B8 → [0x22, 0xB8]
Little-endian: low byte first (reversed):             0x22B8 → [0xB8, 0x22]

Analogy:
  Big-endian    = "2026/05/14" (year→month→day, big to small)
  Little-endian = "14/05/2026" (day→month→year, small to big)

CAN/J1939 uses big-endian by default.
1-byte signals don't care about endianness (only 1 byte, no order issue).
```

### 17.8 CAN ID Structure (29-bit Extended ID)

```
CAN ID example: 0x 00 FE F5 00
                  ①  ②  ③  ④

① Priority + reserved + data page (first byte)
② PF (PDU Format) = message category
③ PS (PDU Specific) = specific number OR destination address
④ SA (Source Address) = sender's address

Extracting PGN: drop ① and ④, keep ②③:
  0x00 FE F5 00  →  PGN = 0xFEF5 = 65269 (Ambient Conditions)

PF ≥ 240 (0xF0): broadcast message → PS is part of PGN
PF < 240:        peer-to-peer      → PS is destination address (PGN = PF only)
```

### 17.9 CAN Module Data Flow

```
CAN bus → [8 raw bytes in buffer]
              ↓
         GetFrame(buffer, &size) → get buffer + status (UPDATED/NOT_UPDATED)
              ↓
         DecodeStatus(buffer, config) → check if raw value is in valid_range → VALID/INVALID
              ↓
         DecodeData(buffer, config) → extract bits per layout → × resolution + offset → physical value
              ↓
         SignalObject(physical_value, VALID/INVALID) → downstream modules use via Ref()
```

### 17.10 CAN Config = Processing Rules, Not Data

```
config file = instruction manual (how to process data) → fixed at compile time, never changes
buffer      = actual data (received from bus at runtime) → may change every cycle

Config contains NO actual measurement values — only "where to extract, how to calculate."

Config 6 fields (per signal):
  ① is_little_endian: byte order (false = big-endian for J1939)
  ② offset:          physical_value = raw × resolution + offset
  ③ valid_range:     {upper, lower} — raw values in range are VALID
  ④ default_value:   initial value before first frame received
  ⑤ resolution:      pointer to resolution constant
  ⑥ bit_layout:      {bit_length, byte_start, bit_start}
```

### 17.11 Interview Vocabulary for CAN/J1939

```
"I've read and understood CAN communication modules in the codebase —
 J1939 protocol implementation including PGN-based message identification,
 SPN signal decoding with resolution/offset conversion, and signal validity
 checking based on J1939's reserved error codes."

"The system uses a config-driven approach where each CAN signal's decoding
 rules (bit layout, resolution, offset, valid range) are defined in configuration
 files, completely separated from the decoding logic. This makes adding new
 signals a configuration task, not a code change."
```

---

## 18. Complete Diagnostic Signal Chain (6 Layers)

### 18.1 Overview — From Sensor to Safety Action

```
The diagnostic system follows a strict layered pipeline.
Each layer has a single responsibility. No layer skips ahead.

Layer 0:  device::        Sensor input (raw measurement → SignalObject)
Layer 1:  fault_judge      Fault detection (IsFaultCondition? → 4-state machine)
Layer 2:  FailureCheck     Aggregation + reporting (Array packaging)
Layer 2a: MaskOr/MaskAnd   Fault masking combination
Layer 3:  Array            Aggregation for downstream consumers
Layer 4:  DiagFailureJudge Translation (FailureSignal → bool)
Layer 5:  so_switch        Signal source switching (real value ↔ safe value)
Layer 6:  Business modules Transparent consumption (don't know about faults)

KEY INSIGHT: downstream modules don't check for faults themselves.
The diagnostic chain ensures they always receive a "safe" signal automatically.
```

### 18.2 Layer 1 — Template Method Pattern (fault_state_detection_model)

```
The base class fault_state_detection_model::Function defines the diagnostic flow:
  ┌─────────────────────────────────────────────────┐
  │  Update() — the Template Method                 │
  │  switch (state) {                               │
  │    case NO:    if (IsFaultCondition()) → CHECK   │
  │    case CHECK: debounce → OCCUR                  │
  │    case OCCUR: if (IsRecoverCondition()) → RECOVER│
  │    case RECOVER: debounce → NO                   │
  │  }                                              │
  └─────────────────────────────────────────────────┘

72 subclasses inherit from this base class.
Each subclass ONLY overrides:
  ├── IsFaultCondition()   — "what is a fault for THIS sensor?"
  └── IsRecoverCondition() — "what is recovery for THIS sensor?"

The subclass does NOT implement:
  ├── State machine logic (handled by base class)
  ├── Debounce timing (handled by FailureDetector in base class)
  ├── Mask checking (handled by base class constructor parameter)
  └── Output signal management (handled by base class)

This is the classic Template Method design pattern:
  "The algorithm structure is fixed; individual steps are customizable."
```

### 18.3 Layer 4 — DiagFailureJudge and DiagNormalJudge

```
DiagFailureJudge: iterate FailureSignal array →
  any OCCUR or RECOVER? → IsFailure = true

DiagNormalJudge: iterate FailureSignal array →
  ALL are NORMAL? → IsDiagNormal = true (inverse of above)

These are mirror-image modules: one looks for "any bad", the other confirms "all good."
```

### 18.4 Layer 5 — so_switch (Signal Source Switching)

```
Core logic (literally 1 line):
  output_ = input_.GetValue() ? true_so_ : false_so_;

input_ = bool (is there a fault?)
true_so_  = safe/fallback value (e.g., 825 RPM idle speed)
false_so_ = real sensor value (from CAN or local sensor)

Result: downstream modules always get a valid signal.
  Normal operation → real sensor value flows through
  Fault detected   → safe fallback value substituted automatically

One so_switch output can be shared by multiple downstream modules (const& reference).
```

### 18.5 Fault Masking (MaskOr / MaskAnd)

```
Masking = "temporarily suppress fault detection during unreliable conditions"

Examples of when to mask:
├── Engine just started (oil pressure hasn't stabilized yet)
├── Low voltage (sensor readings may be inaccurate)
├── Upstream CAN communication already failed (can't trust data)

MaskOr:  any condition ON → mask ON  (OR logic, "any reason to suppress")
MaskAnd: all conditions ON → mask ON (AND logic, "all conditions must agree")

Safety Philosophy:
  "判定前小心谨慎（mask宽松屏蔽），判定后立刻行动（LimpHome立即保护）"
  "Cautious BEFORE confirming a fault (mask generously to avoid false alarms),
   Decisive AFTER confirming (trigger LimpHome immediately for protection)"
```

### 18.6 Interview Vocabulary for Diagnostics

```
"The system has a multi-layered diagnostic architecture:
 fault detection modules use a Template Method pattern — the base class
 provides the state machine (NO→CHECK→OCCUR→RECOVER), and 72 subclasses
 each override only 'what counts as a fault' and 'what counts as recovery.'

 Confirmed faults propagate through an aggregation layer, get translated
 to boolean flags, and drive automatic signal switching — the downstream
 business modules never see the fault directly; they just receive a safe
 fallback value transparently.

 Fault masking suppresses diagnostics during unreliable conditions like
 cold start or low voltage, following the principle: be cautious before
 confirming a fault, but act decisively once confirmed."
```

---

## 19. C++ Template vs Template Method Pattern

### 19.1 Critical Distinction

```
These two concepts share the word "template" but are COMPLETELY DIFFERENT:

┌─────────────────────────────────┬─────────────────────────────────┐
│     C++ Template (generic)      │   Template Method (design pattern)│
├─────────────────────────────────┼─────────────────────────────────┤
│ Problem: "types differ,         │ Problem: "flow is the same,      │
│          logic is the same"     │          steps differ"           │
│                                 │                                  │
│ Mechanism: compile-time         │ Mechanism: runtime               │
│            code generation      │            virtual dispatch      │
│                                 │                                  │
│ Keyword: template<typename T>   │ Keyword: virtual / = 0           │
│                                 │                                  │
│ Example: so_switch<Speed>       │ Example: fault_judge subclass    │
│          so_switch<Pressure>    │          overrides IsFaultCond() │
│                                 │                                  │
│ Files: .h only (no .cc)         │ Files: .h + .cc (both needed)    │
│                                 │                                  │
│ # in system: 45 modules         │ # in system: 72 subclasses       │
│                                 │              (1 base class)      │
│                                 │                                  │
│ Analogy: cookie cutter —        │ Analogy: recipe with blanks —    │
│  same shape, any dough          │  same steps, fill in specifics   │
└─────────────────────────────────┴─────────────────────────────────┘
```

### 19.2 Why C++ Templates Must Be in .h Files

```
When the compiler processes a .cc file that uses so_switch<Speed>:
  It needs to generate machine code for so_switch with Speed as the type.
  But if the template body is in a separate .cc file, the compiler can't see it.
  (Each .cc is compiled independently — it can only see #included .h files.)

Solution: put the entire template implementation in the .h file.
  When any .cc file #includes the .h, the compiler has the full template body
  and can generate the type-specific code on the spot.

This is different from normal classes, where .h has declarations and .cc has implementations.
Template = special case where everything must be visible at compile time.
```

### 19.3 Pure Virtual Interface (Third Category)

```
Beyond C++ templates and Template Method, there's a third module category:

Pure virtual interface:
  ├── ALL methods are pure virtual (= 0)
  ├── NO implementation in the base class at all
  ├── Subclass must implement EVERY method
  ├── Used when subclasses have completely different behavior
  └── Example: AdjustPhaseInterface (6 subclasses)

Comparison:
  C++ template:    same logic, different types    (compile-time)
  Template Method: same flow, different steps     (runtime, partial override)
  Pure interface:  nothing shared, just a contract (runtime, full override)
```

---

## 20. Constructor Initialization — Base Class with Parameters

### 20.1 When You Must Explicitly Call the Base Constructor

```
Rule:
  If the base class has a parameterized constructor → you MUST call it in the
  initializer list of the derived class constructor.

  If the base class has a default (no-parameter) constructor → it's called
  automatically; you don't need to write anything.

Example (must call explicitly):
  class FaultJudge : public fault_state_detection_model::Function {
      FaultJudge(const Config& config, const MaskOrder& mask, Manager& mgr)
          : Function(config.super, mask, mgr),   // ← MUST call base constructor
            sensor_input_(config.sensor_ref)      // ← then init own members
      { }
  };

Example (automatic, no explicit call needed):
  class MyModule : public ModuleInterface {
      MyModule(const Config& config, Manager& mgr)
          : config_(config)     // ← No ModuleInterface(...) needed
      { mgr.Register(*this); }
  };

Why? ModuleInterface has a no-parameter constructor.
     fault_state_detection_model::Function requires (config, mask, manager).
```

### 20.2 Three-Layer Inheritance Chain

```
In the diagnostic system:
  ModuleInterface (framework, no-param constructor)
       ↑ inherits
  fault_state_detection_model::Function (base, 3-param constructor)
       ↑ inherits
  oil_temp_sensor_fault_judge (concrete subclass)

The concrete subclass constructor must:
  1. Call Function(config.super, mask, manager)  ← explicit, because Function has params
  2. Function internally calls ModuleInterface() ← automatic, because ModuleInterface has no params
  3. Initialize its own members (sensor references, etc.)

Engineering method: "Don't memorize what to inherit.
  Find a similar existing module → check what it inherits → do the same."
```

### 20.3 Struct Members Cannot Be References

```
WRONG:
  struct Config {
      const SomeClass& parent_config;   // ❌ Reference member in struct
  };

WHY: struct members must be assignable (for copying, aggregate init, etc.).
     References cannot be reassigned after initialization.

CORRECT:
  struct Config {
      SomeClass::Config super;          // ✅ Value copy (nested struct)
  };

Then in the constructor:
  FaultJudge(const Config& config, ...)
      : Function(config.super, ...)     // Pass the nested value to base
  { }
```

---

## 21. Variable Shadowing — The Silent Killer

```
One of the most dangerous C++ traps (compiler does NOT warn):

WRONG (in Update() method):
  signal_object::Pressure output_(value, VALID);
  ↑ This DECLARES a new local variable with the same name as the member variable!
  ↑ The local variable is destroyed when the function returns.
  ↑ The actual member variable output_ is never updated → always has initial value.

CORRECT:
  output_ = signal_object::Pressure(value, VALID);
  ↑ No type name = assignment to existing member variable.

Rule: "In Update(), assignments never have a type name in front."
  Type name before variable = declaration (new variable)
  No type name             = assignment (existing variable)

This is legal C++ (variable shadowing is allowed), so the compiler stays silent.
Only understanding the difference between "declaration" and "assignment" prevents this bug.
```

---

## 22. CAN/J1939 vs Application Module Comparison

```
Application modules you write:     CAN communication modules:
├── Input:  other module's signal   ├── Input:  8 raw bytes from CAN bus
├── Process: if/else/state machine  ├── Process: decode (extract bits × resolution + offset)
├── Output: SignalObject            ├── Output: SignalObject (exactly the same)

The only difference: CAN modules have an extra "raw bytes → physical value" step.
After conversion, the output SignalObject is identical to any application module's output.

Interview insight: "The CAN layer and the application layer both produce SignalObjects
 with value and validity. Application modules don't need to know whether their input
 came from a CAN message or a local sensor — the interface is uniform."
```

---

## 23. Git Version Control (Brief)

```
Core workflow:
  Working directory → git add → Staging area → git commit → Local repo → git push → Remote repo

Daily usage (3 commands):
  git add .
  git commit -m "Add xxx_module"
  git push

Key concepts for interviews:
├── Feature branch workflow: create branch → develop → merge via pull request
├── Code review: PR review before merging to main
├── .gitignore: exclude build artifacts (*.o, *.lib, *.obj)
├── Descriptive commit messages: explain what and why

Interview one-liner:
  "I use Git for version control — feature branching, descriptive commits,
   and pull request workflow for code review before merging."
```

---

## 24. Practice Writing Progression (Updated May 22, 2026)

### 24.1 Module Writing Statistics

```
Total modules written from scratch: 27
  ├── With reference code: 4 (fan_control, overspeed_warning, engine_temp_monitor, parking_lamp)
  ├── Without reference code: 23

Module types practiced:
  ├── Combinational logic (if/else)
  ├── Two-phase state machine + timer + hysteresis
  ├── Three-phase state machine + timer + hysteresis
  ├── Multi-output modules (2 outputs)
  ├── Edge detection (prev_ variable pattern)
  ├── Counter + threshold
  ├── Multi-input priority arbiter
  ├── Mode switching + mixed output types
  ├── Multi-timer coordination (3 timers + 4 states)
  ├── Diagnostic fault_judge (Template Method subclass) × 4
  ├── Float arithmetic + diagnostic check
  └── Hysteresis + Timer combined (two-dimensional debounce, Design B)

Compilation pass rate (no-reference modules, chronological):
  13 rounds → 2 → 2 → 2 → 1 → 2 → 1 → 1 → 1 → 5 → multi → 3 → 2 → 1
  (Stabilized at 1-2 rounds for medium complexity; new patterns may need more)

Error progression for fault_judge modules (latest batch):
  7 errors → 6 → 5 → 0 (oil_temp → coolant → fuel_pressure → vehicle_speed)
```

### 24.2 Error Categories — What Was Eliminated

```
ELIMINATED (zero occurrences in last 5+ modules):
  ├── enum class (C++11, not supported by C++03 compiler)
  ├── Function nesting (getter inside Update body)
  ├── Spelling errors (Fucntion, signale_object)
  ├── State machine logic / switch structure
  ├── const on output variables
  ├── Update() used as bool return
  ├── .h/.cc constructor signature mismatch
  ├── Manager value-passing instead of reference

RESIDUAL (low-frequency, handwriting errors):
  ├── Member variable missing underscore suffix (cabin_temp vs cabin_temp_)
  ├── Include filename typos (incement_timer)
  ├── GetValue() capitalization (getValue)

LOGIC BUGS (compiler can't catch — need code review):
  ├── = vs == in conditions (assignment vs comparison)
  ├── Reading own output instead of input signal
  ├── Timer Clear() at wrong position (should be on state transition, not every cycle)
  ├── prev_ variable declared as local instead of member (resets every cycle)
  ├── Variable shadowing (see section 21)
```

### 24.3 Pre-Compilation Checklist

```
□ Manager& is reference (not value passing)?
□ .h and .cc constructor signatures match exactly?
□ All member variables end with _ suffix?
□ Update() capitalized? Not used as bool?
□ GetValue() / GetState() spelled correctly?
□ Output variables NOT declared const?
□ All used types #included?
□ All #include filenames spelled correctly?
□ Commas between parameters, commas in initializer list?
□ timer.Update() BEFORE switch statement?
□ Every case assigns a value to output?
□ :: (double colon) not : (single colon)?
□ Class definition ends with }; (semicolon)?
□ Conditions use == not =?
□ Edge detection prev_ is a member variable (not local)?
□ Clear() called on state ENTRY (not exit)?
□ HysteresisFloat32::Update() receives input value?
□ void Update() declared in .h?
□ Local variables assigned on ALL code paths?
□ Base class constructor called in initializer list (if it has parameters)?
```

### 24.4 Interview Talking Point

```
"I've written 27 embedded C++ modules from scratch — including state machines,
 diagnostic fault detectors, multi-timer controllers, hysteresis+timer combined
 debounce modules, and edge detection modules. I also built portable PC simulation
 projects to verify my logic cycle-by-cycle without cross-compilation.
 I started with 13 compilation rounds to get one module right; now I consistently
 compile medium-complexity modules in a single pass. I track every error,
 categorize them, and systematically eliminate recurring patterns. My error
 types shifted from basic syntax to logic-level bugs that compilers can't catch,
 which taught me the value of code review and unit testing."
```

---

## 25. C++ Supplementary — const_cast

```
Syntax:
  const_cast<NonConstType&>(const_object).Method();

Reading order (peel from inside out):
  ① const_object          → get the const object
  ② const_cast<...>(...)  → remove const restriction
  ③ .Method()             → call the non-const method

Why needed:
  If a member is declared const, you can't call non-const methods on it.
  Sometimes library classes don't declare methods as const even though they
  should be (design flaw). const_cast is the workaround.

  This is common in real codebases — not ideal, but pragmatic.
```

---

## 26. Updated Design Patterns Summary (9 Total)

```
Patterns identified in the system:

1. Composite         — Manager tree (parent→children cascading Update() calls)
2. Dependency Injection — Macro-based wiring at compile time (INSTANTIATION)
3. Strategy          — Config injection (same module, different parameters)
4. State Machine     — switch-case + enum (star topology, three-phase)
5. Template Method   — fault_state_detection_model (base defines flow, sub overrides steps)
6. Observer          — SignalObject ref sharing (producer→consumer via const& reference)
7. Star Topology FSM — All states route through central idle state
8. C++ Template      — so_switch<T>, so_comparator<T> (type-generic, header-only)
9. Pure Interface    — AdjustPhaseInterface (all pure virtual, subclass implements everything)
```



---

## 27. RTOS Fundamentals

```
Task vs ISR (Interrupt Service Routine):
  Task: managed by RTOS scheduler, has priority, can sleep/wait
  ISR:  triggered by hardware, preempts ALL tasks, must be microsecond-short

  ISR rules:
  - No sleeping, no waiting, no mutex locking
  - Copy data → set flag → return immediately
  - Runs outside RTOS scheduler (hardware-driven)

volatile keyword:
  Problem: compiler caches variables in registers → ISR updates memory but
           task reads stale register value
  Solution: volatile bool flag; → forces every read from memory
  Rule: any variable shared between ISR and Task → must be volatile

Critical Section (shared data protection):
  Method 1 — Disable interrupts:
    disable_interrupts();
    shared_data = value;    // safe
    enable_interrupts();
    → simple but blocks ALL interrupts → must be very short

  Method 2 — Mutex:
    mutex_lock(&m);
    shared_data = value;    // safe
    mutex_unlock(&m);
    → only blocks competing tasks, not all interrupts

Priority Inversion + Priority Inheritance:
  Problem: Low holds lock → High waits → Medium preempts Low
           → High is indirectly blocked by Medium!
  Solution: RTOS temporarily raises Low to High's priority
           → Medium can't preempt → Low finishes fast → High proceeds

K37TM Task Structure:
  fast_task  (1-5ms)  — sensor sampling, ADC reads
  main_task  (10ms)   — all 400+ business modules Update()
  slow_task  (100ms)  — logging, diagnostic reports
```

---

## 28. CAN Interrupt — Top-half / Bottom-half Pattern

```
When a CAN frame arrives:

① Hardware triggers CAN receive interrupt
② ISR (top-half) — microseconds:
   - Copy 8 bytes from CAN controller register → memory buffer
   - Set flag: can_frame_ready = true
   - Return immediately (NO parsing, NO conversion)
③ Task (bottom-half) — in main_task's 10ms cycle:
   - Check can_frame_ready flag
   - Read 8 bytes from buffer
   - Parse J1939 format (DecodeData, resolution × raw + offset)
   - Write to SignalObject for downstream modules

Why split?
  ISR must be fast → long ISR delays other interrupts → system instability
  Parsing can wait → do it in the 10ms task cycle

Analogy: courier (ISR) drops package at door → you (Task) open it when ready

Connection to K37TM code you've read:
  CAN modules (FEF500, F00400) → their Update() = bottom-half
  GetFrame() = reads buffer that ISR filled
  DecodeData() = parsing = bottom-half core work
  ISR itself is in middleware/device driver layer (source not visible)
```

---

## 29. Practice Module Progress Update (26 Total)

```
Business logic modules:     18 (parking_lamp through idle_speed_controller)
Diagnostic modules (prev):   4 (oil_temp, coolant_temp, fuel_pressure, vehicle_speed)
Diagnostic modules (new):    2 (mismatch_detection, clutch_slip_fault_judge)
                            ──
Total:                      26

clutch_slip_fault_judge_model key learnings:
  - Inherits fault_state_detection_model::Function (Template Method)
  - IsFaultCondition: clutch engaged (FL||RL) && speeds exceed thresholds
  - IsRecoverCondition: speed ≤ threshold || clutch == NN (use || not &&)
  - tm_state.h is in Application library, NOT middleware
  - Build path: set AP=.\..\..\..\Application\1_14_0 + -I%AP%\signal_object\include
  - Operator precedence: (a == FL || a == RL) && b — must parenthesize ||
```



---

## 30. Hysteresis + Timer Combined Pattern (Two-Dimensional Debounce)

### 30.1 Why Combine Two Tools

```
Hysteresis alone:  prevents oscillation in VALUE dimension (dead zone)
                   but responds instantly → one-cycle spike can still trigger
Timer alone:       prevents oscillation in TIME dimension (sustained confirmation)
                   but uses single threshold → can oscillate right at the threshold
Combined:          both dimensions covered → robust against noise AND oscillation

Analogy:
  Hysteresis = "don't count unless value is clearly above/below threshold"
  Timer      = "don't act unless condition holds for N cycles"
  Combined   = "value must be clearly high AND stay clearly high for N cycles"
```

### 30.2 Two Design Strategies for Turn-OFF Timer

When using timer in an ON state to delay turn-off:

```
Design A: Minimum display time
  Timer starts from entering STATE_ON
  → After N cycles, IsTimeUp() is always true
  → Any temperature drop (hyst=false) causes IMMEDIATE turn-off
  → Timer only protects the first N cycles after turn-on
  → Problem: if ON for >N cycles, drop causes instant OFF (no delay)

Design B: Turn-off confirmation delay (PREFERRED)
  Timer is continuously Cleared while hyst=true (temperature still high)
  → Timer only starts counting when hyst becomes false (temperature dropped)
  → Must stay low for N cycles before turning OFF
  → Every temperature drop gets a full N-cycle confirmation window

Key difference:
  Design A: timer runs from entry → eventually always expired
  Design B: timer runs from condition change → always provides fresh delay
```

### 30.3 Implementation Pattern

```cpp
case STATE_ON:
    output = ON;
    // Turn off: diagnostic failure (immediate) OR sustained low temperature
    if (!is_diag_normal || (!hyst.GetState() && timer.IsTimeUp())) {
        output = OFF;
        status = STATE_OFF;
        timer.Clear();
    } else if (hyst.GetState()) {   // ← KEY: Clear while condition still holds
        timer.Clear();               //    Timer only counts after condition breaks
    }
    break;
```

### 30.4 Symmetric Design (STATE_OFF ↔ STATE_ON)

```
STATE_OFF → ON:
  Condition:  diag_normal && hyst=true && timer.IsTimeUp()
  Clear when: hyst=false (temp not high → don't count toward confirmation)

STATE_ON → OFF:
  Condition:  !diag_normal (immediate) || (hyst=false && timer.IsTimeUp())
  Clear when: hyst=true (temp still high → don't count toward turn-off)

Both states: timer counts only AFTER the hysteresis condition changes.
```

---

## 31. Practice Module Progress Update (27 Total)

```
Business logic modules:     18 (parking_lamp through idle_speed_controller)
Diagnostic modules:          6 (oil_temp, coolant_temp, fuel_pressure,
                                vehicle_speed, mismatch, clutch_slip)
Business + sim:              1 (transmission_oil_temp_warning — K37TM + sim)
                            ──
Total:                      27

transmission_oil_temp_warning key learnings:
  - First module combining Hysteresis + IncrementTimer
  - Design A vs Design B timer strategy (confirmed delay preferred)
  - Symmetric timer Clear logic in both STATE_OFF and STATE_ON
  - is_diag_normal check added in STATE_OFF→ON condition (safety)
  - Ported to standalone sim project (tm_oil_temp_warning_sim)
  - CMake + MSVC build on PC — no K37TM dependencies
```

---

## 32. Portable Simulation Project Pattern

```
For any new module, create a standalone sim project:

Structure:
  my_module_sim/
  ├── CMakeLists.txt          ← cmake_minimum_required + add_executable
  └── src/
      ├── main.cpp            ← test scenarios with cycle-by-cycle output
      ├── framework/          ← simplified ModuleInterface + Manager
      ├── signals/            ← Signal<T> template (value + validity)
      ├── utility/            ← Hysteresis, IncrementTimer (header-only)
      └── modules/            ← your module .h + .cc

Benefits:
  ├── Runs on any PC with VS2022 (no cross-compiler needed)
  ├── Can see every cycle's input/output → verify logic visually
  ├── Breakpoint debugging available
  ├── Self-contained — no external dependencies
  └── Portable — copy folder to any machine and build

Build commands:
  cd build
  cmake .. -G "Visual Studio 17 2022" -A x64    ← first time only
  cmake --build . --config Release               ← after code changes
  .\Release\my_module_sim.exe                    ← run

Workflow for new modules:
  1. Write .h + .cc in modules/ (using sim signal types)
  2. Update CMakeLists.txt (add .cc to add_executable)
  3. Write test scenarios in main.cpp
  4. Build → run → verify cycle-by-cycle output
```


---

## 33. Signal Tracing — Hands-On Upstream Tracing

### 33.1 Practical Example: tm_oil_temp Full Upstream Chain

```
Traced tm_oil_temperature_model's inputs through 5 layers:

Layer 1: tm_oil_temperature_model (business logic)
  ├── temperature        ← tm_oil_temp.TemperatureRef()
  ├── engine_status      ← CAN J1939 0x00FF2900
  ├── is_diag_normal     ← diag_normal_judge
  └── oil_temp_freq_mdo  ← MDO storage

Layer 2: tm_oil_temp (TemperatureSensorByAnalogInput)
  ├── ADC → interpolation table (16 points) → temperature °C
  ├── low_voltage_fault_mask  ← function_db982ca5.OutputRef()
  └── high_voltage_fault_mask ← function_db982ca5.OutputRef() (same)

Layer 3: function_db982ca5 (so_buffer_model — 1-cycle delay)
  └── input ← mask_or_function_17cf2b8a.ResultRef()

Layer 4: mask_or_function_17cf2b8a (MaskOrFunction — OR merge)
  └── request ← fault_transition_mask_order_45107544

Layer 5: fault_transition_mask_order_45107544 (data array — END)
  ├── WaitForSystemStable  (system initializing → sensor unstable)
  ├── LowVoltageBatt       (low battery → ADC unreliable)
  └── EngineStopAtKeyOff   (engine off → sensor de-powered)
```

### 33.2 Fault Mask = Suppress, Not Trigger

```
Common confusion: fault_mask sounds like it triggers faults. It's the OPPOSITE.

fault_mask = ON  → SUPPRESS fault detection (don't check sensor voltage)
fault_mask = OFF → NORMAL fault detection (check if voltage in valid range)

Why suppress? During startup / low voltage / engine off, sensor readings are
inherently unreliable. Diagnosing them would produce false fault reports.

Key: mask only affects fault DIAGNOSIS, not the temperature VALUE itself.
     .TemperatureRef() keeps outputting temperature regardless of mask state.
```

### 33.3 MaskOrFunction — Data-Driven OR Logic

```
Same MaskOrFunction class instantiated 20+ times with different condition arrays.
Code never changes — only the input data (which conditions to OR-merge) differs.

Update() logic:
  result = OFF
  for each condition in request_array:
    if condition == ON → result = ON
  output = result

This is data-driven design: one generic class + many different data configurations.
```

### 33.4 Key Observation: Same Class, 3 Instances

```
oil_temperature_model::Function is instantiated 3 times:
  hst_oil_temperature_model  (HST oil)
  tm_oil_temperature_model   (transmission oil)
  axle_oil_temperature_model (axle oil)

Each uses different:
  - config (thresholds)
  - temperature source sensor
  - diagnostic judge
  - MDO storage target

Same code, different wiring → configuration separation in action.
```



---

## 34. Signal Tracing — CAN Output Chain + System Boundaries

### 34.1 CAN Output Chain (Downstream to System Boundary)

```
Traced tm_oil_temp downstream to CAN bus — a short 3-layer chain:

Layer 1: tm_oil_temp (TemperatureSensorByAnalogInput)
  └── .TemperatureRef() → raw temperature value from ADC

Layer 2: StatusOverwriteFunction<Temperature>
  ├── input:  temperature value
  └── status: diag_normal_judge.IsDiagNormalRef()
  → If diagnosis abnormal → overwrites signal status to INVALID
  → Value itself unchanged, only status flag modified

Layer 3: Tm1SenderRtcdbrx0x000100ms (CAN Sender — SYSTEM BOUNDARY)
  ├── Packs 15 signals into one 28-byte CAN frame
  ├── Each signal: EncodeData (value) + EncodeStatus (validity)
  ├── Sends via proxy_.SetFrame() every 100ms
  └── KDOG2 auto-generated code (never hand-written)

Key design: StatusOverwrite ensures receivers know when a value
is unreliable. They see INVALID status → use fallback/default.
```

### 34.2 CAN Sender Module Pattern

```
Structure of auto-generated CAN sender:
  Update()       → do nothing (no computation)
  OutputUpdate() → encode all signals into buffer → send

uint8 buffer[N];
memset(buffer, 0x00, sizeof(buffer));
for each signal:
    EncodeData<SrcType, DstType>(buffer, config, signal.GetValue());
    EncodeStatus<SrcType, DstType>(buffer, config, signal.GetState());
proxy_.SetFrame(buffer, sizeof(buffer));

Config determines: byte offset, bit width, scaling for each signal.
Multiple signals packed into one frame — standard CAN practice.
```

### 34.3 System Boundaries — When to Stop Tracing

```
UPSTREAM endpoints (where signals originate):
├── Physical sensor (ADC)        → e.g., tm_oil_temp
├── CAN/network input (receiver) → e.g., J1939 engine_status
├── MDO (non-volatile storage)   → e.g., mdo.GetValue()
└── Compile-time constant (Config) → e.g., *_config struct

DOWNSTREAM endpoints (where signals terminate):
├── CAN/network output (sender)  → e.g., tm1_sender_rtcdbrx_*
├── MDO write                    → e.g., tm_oil_temperature_model
├── No OutputRef() consumers     → search "module." finds nothing
└── Hardware actuator output     → e.g., relay/solenoid driver

Practical rule:
  Upstream: param is xxx.SomeRef() → keep tracing xxx
            param is config/manager/MDO → STOP
  Downstream: search "module." finds consumer → keep tracing
              search "module." finds nothing → STOP
```

### 34.4 Handling Branches During Tracing

```
Branches are normal — a module often has multiple inputs from different sources.

Strategy:
1. Note all branches at the junction point
2. Pick one branch, trace it to its endpoint (depth-first)
3. Come back, trace the next branch
4. Judge effort vs value:
   ├── Main signal path (temperature, fault) → TRACE
   ├── Mask conditions → read name, understand meaning, usually STOP
   ├── Config / manager → STOP (compile-time constant)

Example: failure_signal_0c599085_body[] has 2 elements
  ├── [0] tm_oil_temp.LowVoltageFailureRef()     → traced first
  └── [1] dg_tm_oil_...fault_judgement.FailureSignalRef() → traced second
Both traced to their respective endpoints independently.
```

### 34.5 diag_normal_judge Input Chain (Two Fault Sources)

```
diag_normal_judge receives failure_signal array (2 elements):

Source 1: Sensor built-in low voltage check
  tm_oil_temp.LowVoltageFailureRef()
  → Detects: voltage too low (short to ground)

Source 2: External high voltage diagnosis module
  dg_tm_oil_temperature_sensor_open_or_hots_fault_judgement.FailureSignalRef()
  → Detects: voltage too high (open circuit or short to power)
  → Receives: fault_mask from mask_or_function + raw voltage from tm_oil_temp

Both normal → IsDiagNormal = true → downstream modules operate normally.
Either faulty → IsDiagNormal = false → downstream modules use fallback.
```

### 34.6 Module Progress Update

```
Total modules written from scratch: 29
Signal tracing chains completed: 3 (upstream + downstream MDO + downstream CAN)
New concepts this session:
  - StatusOverwriteFunction (signal validity override before CAN send)
  - CAN sender auto-generated pattern (KDOG2)
  - System boundary identification method
  - Branch handling strategy for signal tracing
```

---

## 35. Multi-Level Protection State Machine Pattern

### 35.1 Three-State Protection (Escalation + De-escalation)

```
Pattern: NORMAL → WARNING → CRITICAL with independent confirmation

                warning_hyst + confirm_timer    critical_hyst + confirm_timer
    NORMAL ─────────────────────→ WARNING ─────────────────────→ CRITICAL
      ↑                             │                               │
      └── !warning + recover_timer ─┘                               │
                                    ↑                               │
                                    └── !critical + recover_timer ──┘
      ↑                                                             │
      └──────────── diag_abnormal (force reset to NORMAL) ──────────┘

Key design decisions:
├── Each level has its own Hysteresis thresholds
├── Shared confirm_timer (reused for each escalation step)
├── Shared recover_timer (reused for each de-escalation step)
├── De-escalation is step-by-step (CRITICAL→WARNING→NORMAL, never skip)
├── Diagnostic abnormal → force immediate reset to NORMAL (safety override)
└── Multiple outputs: warning_lamp (WARNING+CRITICAL) + power_limit (CRITICAL only)
```

### 35.2 Timer Clear Analysis Method

```
When analyzing if a timer_.Clear() is necessary at a state transition:

Ask two questions:
  1. Does the NEXT state CHECK this timer? (IsTimeUp / GetState)
  2. Will it be CLEARED before the next check?

If nobody checks it → Clear is redundant (but harmless)
If someone checks it AND it won't be cleared before that → Clear is NEEDED

Example analysis for confirm_time_:
  WARNING → CRITICAL transition: Clear confirm_time_?
    → In CRITICAL, nobody checks confirm_time_ → REDUNDANT
    → CRITICAL→WARNING clears it anyway → confirmed REDUNDANT

  NORMAL → WARNING transition: Clear confirm_time_?
    → In WARNING, confirm_time_ used for CRITICAL escalation → NEEDED
```

### 35.3 Safety Design: When to Skip Levels

```
Question: Should NORMAL→CRITICAL direct skip be allowed?

Answer depends on PHYSICAL properties of the signal:

Slow-changing (thermal inertia):
  Oil temperature, coolant temperature, ambient temperature
  → Change takes 30-60 seconds minimum
  → 2× confirm_timer delay (~1 second) is negligible
  → Sequential escalation is FINE, no skip needed

Fast-changing (electrical/mechanical):
  Current, voltage, pressure (pipe burst)
  → Can change in milliseconds
  → Software state machine may be too slow
  → Use HARDWARE interrupt for emergency protection
  → Software state machine handles gradual degradation only

Interview insight: Asking "what if temperature spikes instantly?"
shows safety-critical thinking. Answer: "Oil has thermal inertia,
so sequential escalation is sufficient. For fast signals like
overcurrent, hardware protection bypasses the software entirely."
```

### 35.4 Updated Module Count

```
Total modules written from scratch: 29
  ├── Basic control: 18 (parking_lamp, reverse_buzzer, fan_control, etc.)
  ├── Diagnostics: 6 (oil_temp_sensor, mismatch_detection, etc.)
  ├── Signal processing: 1 (sensor_value_validator)
  └── Multi-level protection: 1 (multi_level_oil_temp_protection)

Module types mastered:
  State machine (2-state, 3-state, star topology)
  Hysteresis + Timer combination
  Edge detection
  Fault detection (Template Method pattern)
  Sensor redundancy / fallback
  Signal status overwrite
  Multi-level escalation/de-escalation
```
