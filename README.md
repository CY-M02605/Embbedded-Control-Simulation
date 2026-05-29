# Embedded Control Simulation

A PC-based simulation of embedded control logic for a **hydraulic oil over-temperature warning system**. This project demonstrates a modular, extensible framework commonly used in automotive and industrial embedded software.

## Architecture

```mermaid
graph TD
    A[main.cc] -->|creates & configures| B[Manager]
    A -->|creates| C[TemperatureSignal]
    A -->|creates| D[BoolSignal - Diagnostics]
    A -->|creates| E[HydraulicTempWarningFunction]
    E -->|registers to| B
    E -->|reads| C
    E -->|reads| D
    E -->|uses| F[Hysteresis]
    E -->|uses| G[IncrementTimer]
    E -->|outputs| H[Normal Warning Lamp]
    E -->|outputs| I[Critical Warning Lamp]
    B -->|calls Update on each cycle| E
```

## Warning Logic

### Two-Level Alert System

| Level | Condition | Behavior |
|-------|-----------|----------|
| **Normal** | Temp >= 95 C for 3 consecutive cycles, diagnostics OK | Normal lamp ON |
| **Critical** | Temp >= 100 C, regardless of diagnostics | Critical lamp ON (forced) |

### State Machine (Normal Warning)

```mermaid
stateDiagram-v2
    [*] --> OFF
    OFF --> ON : temp >= high_threshold + sustained 3 cycles + diag OK
    ON --> OFF : temp <= low_threshold OR diag NG
    ON --> ON : temp still above low_threshold AND diag OK
    OFF --> OFF : conditions not met
```

### Key Design Decisions

- **Hysteresis**: Uses separate high (95 C) and low (85 C) thresholds to prevent lamp flickering when temperature oscillates near a single threshold.
- **Debounce Timer**: Temperature must exceed the threshold for 3 consecutive cycles before triggering, filtering out sensor noise.
- **Fail-safe (Critical)**: When temperature reaches critical level (>= 100 C), the system forces the warning lamp ON regardless of diagnostic status, ensuring safety even during communication failure.
- **Signal Validity**: If input signals are invalid (e.g., sensor disconnected), the system turns off lamps and marks output as INVALID.

## Project Structure

```
hydraulic_temp_warning_system/
+-- CMakeLists.txt
+-- main.cc                              # Entry point, test simulation
+-- src/
    +-- framework/
    |   +-- module_interface.h           # Abstract base class (pure virtual Update)
    |   +-- manager.h                    # Module scheduler (register + UpdateAll)
    +-- modules/
    |   +-- hydraulic_temp_warning_module.h   # Warning module declaration
    |   +-- hydraulic_temp_warning_module.cc  # Warning module implementation
    +-- signals/
    |   +-- signals.h                    # Signal template class with validity
    +-- utility/
        +-- hysteresis.h                 # Hysteresis comparator (reusable)
        +-- increment_timer.h            # Debounce timer (reusable)
```

## Build & Run

Requires **CMake 3.10+** and a C++11 compatible compiler.

```bash
cd hydraulic_temp_warning_system
mkdir build
cd build
cmake ..
cmake --build .
.\Debug\hydraulic_temp_warning_system.exe   # Windows
```

## Future Plans

- **Arduino Port**: Migrate logic to real hardware with ADC temperature sensing, GPIO LED control, and timer interrupts for scheduling.
- **FreeRTOS Integration**: Multi-task architecture with separate tasks for sensor reading, warning logic, and display.
- **Additional Modules**: Engine speed monitoring, system status dashboard ? demonstrating framework extensibility.
