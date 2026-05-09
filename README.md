# jaql

Production-grade computation engine foundation for quantitative finance.

## Stack

- **Core**: C++20
- **Bindings**: pybind11
- **Build**: CMake 3.25+ with presets
- **Testing**: Catch2 (C++), pytest (Python)
- **Benchmarking**: Google Benchmark

## MVP Coverage

- Date/calendar/day-count conventions
- Basic instruments: bonds, forwards, equity options (Black-Scholes)
- Flat and bootstrapped yield curves
- Basic Greeks (Delta, Gamma, Vega, Theta, Rho)
- Python bindings for MVP functionality

## Build

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Python tests

```bash
python -m pip install pytest
PYTHONPATH=build/debug python -m pytest tests/python -q
```

## Notes

- No external financial dependencies are used.
- Library remains computation-only (no persistence layer).
- CI includes compiler/build-type matrix and wheel build baseline.
