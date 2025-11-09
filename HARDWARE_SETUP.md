# Hardware Setup Guide

## Required Components:
- ESP32-CAM module
- Relay module (5V)
- Water pump (12V)
- LED indicator
- Breadboard and jumper wires
- 12V power supply
- Emergency stop button

## Wiring Connections:

### ESP32-CAM Pins:
```
GPIO 2  → Relay IN (Pump control)
GPIO 13 → LED + (Status indicator)
GPIO 14 → Emergency Stop Button
GND     → Common ground
5V      → Relay VCC
```

### Relay Module:
```
VCC → ESP32 5V
GND → ESP32 GND
IN  → ESP32 GPIO 2
NO  → Pump positive wire
COM → 12V power supply positive
```

### Water Pump:
```
Positive → Relay NO (Normally Open)
Negative → 12V power supply negative
```

### Status LED:
```
Positive → ESP32 GPIO 13
Negative → ESP32 GND (through 220Ω resistor)
```

### Emergency Stop:
```
One terminal → ESP32 GPIO 14
Other terminal → ESP32 GND
```

## Power Supply:
- ESP32-CAM: 5V via USB or external supply
- Pump: 12V external supply
- Relay: 5V from ESP32

## Safety Notes:
- Always use proper isolation between high voltage and ESP32
- Test all connections before powering on
- Keep water away from electronics
- Use appropriate fuses for pump circuit
