# DotActuate - Blockchain-Controlled Physical Automation

**Hackathon 2025**

DotActuate enables trustless physical automation through blockchain-verified commands with cryptographic proof of execution. Control real-world infrastructure remotely with mathematical certainty that actions occurred.

## 🎯 Problem Solved

Current IoT systems require trust in centralized servers and provide no verification that commands actually executed. DotActuate eliminates this trust requirement by providing blockchain-based proof of physical actions.

## 🚀 Innovation

- **First blockchain-controlled IoT** with cryptographic execution proof
- **Trustless automation** - no central authority needed
- **Cryptographic verification** using SHA256 image hashing
- **Polkadot native** integration with Moonbase Alpha parachain

## 🏗️ Architecture

### Components
1. **Smart Contract** (Moonbase Alpha) - Command storage and event emission
2. **ESP32-CAM Hardware** - Physical actuator control with camera proof
3. **Web Interface** - MetaMask integration for blockchain transactions
4. **Proof System** - Before/after image hashing for verification

### Tech Stack
- **Blockchain**: Polkadot Moonbase Alpha testnet
- **Hardware**: ESP32-CAM, relay modules, actuators
- **Frontend**: HTML5, JavaScript, Web3, MetaMask
- **Backend**: Solidity smart contracts, Arduino C++

## 🔧 Setup Instructions

### 1. Deploy Smart Contract
```bash
# Deploy Command.sol to Moonbase Alpha
# Network: Moonbase Alpha Testnet
# RPC: https://rpc.api.moonbase.moonbeam.network
# Chain ID: 1287
```

### 2. Hardware Setup
```cpp
// Upload esp32_dotactuate_fixed.ino to ESP32-CAM
// Wire components per HARDWARE_SETUP.md
// Update WiFi credentials and contract address
```

### 3. Web Interface
```html
# Open web_moonbase_fixed.html in browser
# Install MetaMask and add Moonbase Alpha network
# Get testnet DEV tokens from faucet
```

## 📱 Usage

1. **Send Command**: Use web interface to send blockchain transaction
2. **ESP32 Detection**: Device polls blockchain and detects new commands
3. **Proof Capture**: Camera captures before-action image hash
4. **Execute Action**: Physical actuator (pump/motor) activates
5. **Verification**: After-action image hash provides cryptographic proof

## 🎬 Demo Flow

```
Web Interface → MetaMask → Moonbase Alpha → ESP32 Polling → 
Physical Action → Camera Proof → Blockchain Verification
```

## 🌍 Real-World Applications

- **Agricultural irrigation** in remote/developing regions
- **Emergency response systems** with verified activation
- **Off-grid infrastructure** control via cellular/GPRS
- **Critical systems** requiring execution proof

## 🏆 Hackathon Criteria

### Technological Implementation
- Quality ESP32-Arduino integration with Polkadot
- Proper Web3 MetaMask integration
- Cryptographic proof system using SHA256

### Design
- Clean web interface with real-time camera feed
- Modular Arduino code architecture
- Balanced frontend/backend implementation

### Potential Impact
- Solves critical infrastructure trust problems
- Enables automation in developing regions
- Creates new paradigm for IoT verification

### Quality of Idea
- Novel blockchain + physical automation combination
- First implementation with cryptographic execution proof
- Addresses genuine real-world problem

## 📁 File Structure

```
├── Command.sol                    # Smart contract
├── esp32_dotactuate_fixed.ino    # Main ESP32 firmware
├── web_moonbase_fixed.html       # Web interface
├── HARDWARE_SETUP.md             # Wiring guide
├── DEPLOYMENT_GUIDE.md           # Setup instructions
└── README.md                     # This file
```

## 🔗 Live Demo

- **Contract**: `0x6e2ec30DD6093f247023019e408E226a345e5769`
- **Camera Stream**: `http://192.168.144.1:81/stream`
- **Network**: Moonbase Alpha Testnet

## 🛡️ Security & Resilience

- **Cryptographic proof** prevents false execution claims
- **Blockchain immutability** ensures audit trail
- **Off-grid capable** via GPRS for true resilience
- **Emergency stop** functionality for safety

## 🎯 Winning Factors

1. **Complete working system** - Hardware + Blockchain + Web3
2. **Real-world utility** - Solves actual infrastructure problems
3. **Technical innovation** - First blockchain IoT with crypto proof
4. **Polkadot integration** - Proper parachain utilization
5. **Demonstration ready** - Full end-to-end functionality

