# Gas Pump Module

This Repository is used to incorporate the Gas Pump handling software, which interact with the physical pump device, allowing it to be locked/unlocked, so that an employee can supply the company vehicle with fuel. After the supply is completed, the pump will signal the supply manager (included in the composer) of the amount supplied and stock left 

# Architecture

The `gas-pump_device` folder contains the code to build and flash the project into an ESP32 device, which connects to the `mqtt5` broker, servicing the `ditto` digital twin service. This means that the information sent/received by the ESP32 is always communicated to ditto and propagated to/from Kafka (located in the **Orchestrator** repository).

# Configuring and executing

For the first time, please analyze the instructions in `gas-pump_device`, `mqtt5` and `ditto` folders, for instructions to configue and execute the services manually. After, the scripts `run.sh` or `stop.sh` can be used to control the execution of the system.