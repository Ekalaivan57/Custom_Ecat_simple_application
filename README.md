# Custom EtherCAT Simple Application

**Company**: ERL Spectra  
**Target Hardware Bus**:
- Position 0: `EK1100` EtherCAT Coupler
- Position 1: `EL5101` 1-Channel Incremental Encoder (5V)
- Position 2: `EL5072` 2-Channel Inductive Sensor Interface (LVDT)
- Position 3: `EL4008` 8-Channel Analog Output (0-10V, 12-bit)
- Position 4: `EL2004` 4-Channel Digital Output (24V, 0.5A)

---

## 📁 Project Structure

```
Custom_Ecat_simple_application/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # Build and execution guide
└── application/
    ├── define.h                # Hardware macros, vendor/product IDs & PDO maps
    ├── ecat_app.h              # Application API header
    ├── ecat_app.c              # Application logic implementation (uses ecat_api)
    └── main.c                  # Main entry point and signal handler
```

---

## 🛠️ Build Instructions

Ensure the custom library `ecat_api` is installed system-wide (`sudo make install` in `/home/erl/Custom_EtherCAT/`).

```bash
cd /home/erl/Custom_Ecat_Appication/Custom_Ecat_simple_application
mkdir -p build && cd build
cmake ..
make
```

---

## 🚀 Execution

Run with root privileges to grant EtherCAT raw network socket access:

```bash
sudo ./Custom_Ecat_App
```

---

## 📊 Sample Telemetry Output

```
Cyclic time (ms):   500.0 | Cycle count:     500 | LVDT Value:    -291958 | Analog output value: 10.00 V | Encoder value: 12345 | Digital output Value: CH1=ON  CH2=OFF CH3=ON  CH4=OFF
```