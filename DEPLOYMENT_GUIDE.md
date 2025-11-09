# Polka-RA Deployment Guide

## Step 1: Deploy Smart Contract

1. **Open Remix IDE**: https://remix.ethereum.org
2. **Create new file**: `Command.sol`
3. **Paste contract code** from `Command.sol`
4. **Compile**: Solidity 0.8.0+
5. **Deploy to Moonbase Alpha**:
   - Network: Moonbase Alpha Testnet
   - RPC: https://rpc.api.moonbase.moonbeam.network
   - Chain ID: 1287
   - Get testnet DEV tokens from faucet
6. **Copy contract address** and update in code

## Step 2: Setup Hardware

1. **Wire components** according to `HARDWARE_SETUP.md`
2. **Install ESP32 libraries**:
   - ESP32 Camera
   - ArduinoJson
   - WiFiClientSecure
3. **Update WiFi credentials** in `polka_ra_complete.ino`
4. **Update contract address** in code
5. **Upload code** to ESP32-CAM

## Step 3: Setup Web Interface

1. **Update contract address** in `control_panel.html`
2. **Install MetaMask** browser extension
3. **Add Moonbase Alpha network** to MetaMask:
   - Network Name: Moonbase Alpha
   - RPC URL: https://rpc.api.moonbase.moonbeam.network
   - Chain ID: 1287
   - Currency: DEV
4. **Get testnet tokens** from faucet
5. **Open control panel** in browser

## Step 4: Demo Sequence

### Live Demo Script:

1. **Show System Status**:
   ```
   "This is our off-grid resilient actuator system.
   The ESP32 is connected via WiFi, but could use GPRS for true off-grid operation."
   ```

2. **Demonstrate Web Interface**:
   ```
   "From this web interface, I can send commands to any device worldwide
   through the Moonbeam blockchain."
   ```

3. **Send Command**:
   - Click "Run Full Sequence"
   - Show MetaMask transaction
   - Wait for blockchain confirmation

4. **Show ESP32 Response**:
   ```
   "Watch the serial monitor - the ESP32 polls the blockchain,
   receives the command, and executes the physical action."
   ```

5. **Highlight Proof System**:
   ```
   "Notice the cryptographic proof - before and after image hashes
   provide immutable evidence that the action actually occurred."
   ```

6. **Explain Innovation**:
   ```
   "This creates trustless physical automation - anyone can verify
   that real-world actions happened through blockchain proof."
   ```

## Step 5: Hackathon Presentation

### Key Points to Emphasize:

1. **Problem Solved**: Off-grid infrastructure control
2. **Innovation**: Blockchain-mediated physical automation
3. **Proof System**: Cryptographic verification of actions
4. **Resilience**: Works without stable internet (GPRS capable)
5. **Real Impact**: Agriculture, emergency systems, remote monitoring

### Demo Flow:
1. Show web interface (2 min)
2. Send command via blockchain (1 min)
3. Show ESP32 execution with proof (2 min)
4. Explain architecture and benefits (3 min)
5. Q&A (2 min)

### Winning Arguments:
- "First blockchain-controlled IoT system with cryptographic proof"
- "Solves real infrastructure problems in developing regions"
- "Demonstrates true resilience - works off-grid via cellular"
- "Verifiable automation - trustless physical actions"

## Troubleshooting:

### Common Issues:
1. **WiFi connection fails**: Check credentials
2. **Contract call reverts**: Ensure contract is deployed correctly
3. **Camera init fails**: Check wiring and power supply
4. **MetaMask issues**: Ensure correct network and gas

### Debug Commands:
```cpp
Serial.println("Debug: " + variable);  // Add debug prints
client.setInsecure();                  // Skip SSL verification
delay(1000);                          // Add delays for stability
```

## Success Metrics:
- ✅ Contract deployed and verified
- ✅ Web interface sends transactions
- ✅ ESP32 receives and executes commands
- ✅ Camera captures proof images
- ✅ System demonstrates full automation loop

**Your system is now ready to win the hackathon!** 🏆
