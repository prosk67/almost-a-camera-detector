/*
 * ESP32 433MHz RF Module Tester - Auto-Switching
 * Automatically switches between TRANSMITTER and RECEIVER modes
 * 
 * TRANSMITTER Wiring:
 * RF Transmitter VCC  -> 5V
 * RF Transmitter GND  -> GND
 * RF Transmitter DATA -> GPIO 5
 * 
 * RECEIVER Wiring:
 * RF Receiver VCC  -> 5V
 * RF Receiver GND  -> GND
 * RF Receiver DATA -> GPIO 4
 * 
 * HOW IT WORKS:
 * - Transmits for 10 seconds
 * - Receives for 10 seconds
 * - Repeats forever
 * 
 * Connect BOTH modules at the same time to test!
 * 
 * Install Library: RCSwitch by sui77
 */

#include <RCSwitch.h>

// ========================================
// CONFIGURATION
// ========================================
#define TX_PIN 5              // Transmitter DATA pin
#define RX_PIN 4              // Receiver DATA pin

#define TX_DURATION 10000      // Transmit for 10 seconds
#define RX_DURATION 10000      // Receive for 10 seconds
// ========================================

RCSwitch mySwitch = RCSwitch();

// Mode tracking
enum Mode { TRANSMITTER, RECEIVER };
Mode currentMode = TRANSMITTER;
unsigned long modeStartTime = 0;
unsigned long cycleCount = 0;

// Transmitter variables
unsigned long txMessageCount = 0;
unsigned long lastTxTime = 0;
#define TX_INTERVAL 2000       // Send every 2 seconds

// Receiver variables
unsigned long rxTotalReceived = 0;
unsigned long rxLastReceiveTime = 0;
unsigned long rxModeTotal = 0;  // Signals received in current RX mode

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n");
  Serial.println("╔═══════════════════════════════════════════════════╗");
  Serial.println("║  ESP32 433MHz RF - AUTO-SWITCHING TESTER          ║");
  Serial.println("╚═══════════════════════════════════════════════════╝");
  Serial.println();
  Serial.print("📡 Transmitter Pin: GPIO ");
  Serial.println(TX_PIN);
  Serial.print("📻 Receiver Pin:    GPIO ");
  Serial.println(RX_PIN);
  Serial.println();
  Serial.println("⏱️  Mode Schedule:");
  Serial.print("   • Transmit for ");
  Serial.print(TX_DURATION / 1000);
  Serial.println(" seconds");
  Serial.print("   • Receive for ");
  Serial.print(RX_DURATION / 1000);
  Serial.println(" seconds");
  Serial.println("   • Repeat forever");
  Serial.println();
  Serial.println("🔌 Make sure BOTH modules are connected!");
  Serial.println();
  Serial.println("═══════════════════════════════════════════════════");
  Serial.println();
  
  // Start in transmitter mode
  switchToTransmitter();
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long timeInMode = currentTime - modeStartTime;
  
  // Check if it's time to switch modes
  if (currentMode == TRANSMITTER && timeInMode >= TX_DURATION) {
    switchToReceiver();
  } else if (currentMode == RECEIVER && timeInMode >= RX_DURATION) {
    switchToTransmitter();
  }
  
  // Run the appropriate mode loop
  if (currentMode == TRANSMITTER) {
    transmitterLoop();
  } else {
    receiverLoop();
  }
}

// ============================================================
// MODE SWITCHING
// ============================================================
void switchToTransmitter() {
  cycleCount++;
  currentMode = TRANSMITTER;
  modeStartTime = millis();
  
  // Disable receiver
  mySwitch.disableReceive();
  
  // Enable transmitter
  mySwitch.enableTransmit(TX_PIN);
  mySwitch.setProtocol(1);
  mySwitch.setPulseLength(350);
  mySwitch.setRepeatTransmit(3);
  
  Serial.println("\n\n");
  Serial.println("╔═══════════════════════════════════════════════════╗");
  Serial.println("║          🔴 SWITCHED TO TRANSMITTER MODE          ║");
  Serial.println("╚═══════════════════════════════════════════════════╝");
  Serial.print("Cycle #");
  Serial.print(cycleCount);
  Serial.print(" | Transmitting for ");
  Serial.print(TX_DURATION / 1000);
  Serial.println(" seconds...");
  Serial.println("═══════════════════════════════════════════════════");
  Serial.println();
  
  lastTxTime = 0;  // Force immediate transmission
}

void switchToReceiver() {
  currentMode = RECEIVER;
  modeStartTime = millis();
  rxModeTotal = 0;
  
  // Disable transmitter
  mySwitch.disableTransmit();
  
  // Enable receiver
  mySwitch.enableReceive(digitalPinToInterrupt(RX_PIN));
  
  Serial.println("\n\n");
  Serial.println("╔═══════════════════════════════════════════════════╗");
  Serial.println("║          🔵 SWITCHED TO RECEIVER MODE             ║");
  Serial.println("╚═══════════════════════════════════════════════════╝");
  Serial.print("Cycle #");
  Serial.print(cycleCount);
  Serial.print(" | Listening for ");
  Serial.print(RX_DURATION / 1000);
  Serial.println(" seconds...");
  Serial.println("═══════════════════════════════════════════════════");
  Serial.println();
}

// ============================================================
// TRANSMITTER FUNCTIONS
// ============================================================
void transmitterLoop() {
  unsigned long currentTime = millis();
  
  // Show countdown
  static unsigned long lastCountdown = 0;
  if (currentTime - lastCountdown >= 1000) {
    lastCountdown = currentTime;
    unsigned long remaining = (TX_DURATION - (currentTime - modeStartTime)) / 1000;
    Serial.print("📡 TX Mode | ");
    Serial.print(remaining);
    Serial.println("s remaining");
  }
  
  // Send signals every TX_INTERVAL
  if (currentTime - lastTxTime >= TX_INTERVAL) {
    lastTxTime = currentTime;
    txMessageCount++;
    
    Serial.println();
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.print("📤 Sending Test Signals (Batch #");
    Serial.print(txMessageCount);
    Serial.println(")");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // Signal 1
    Serial.println("  → Sending: 12345");
    mySwitch.send(12345, 24);
    delay(100);
    
    // Signal 2
    Serial.println("  → Sending: 54321");
    mySwitch.send(54321, 24);
    delay(100);
    
    // Signal 3
    Serial.println("  → Sending: 0xAAAAAA");
    mySwitch.send(0xAAAAAA, 24);
    delay(100);
    
    // Signal 4
    Serial.print("  → Sending Counter: ");
    Serial.println(txMessageCount);
    mySwitch.send(txMessageCount + 1000, 24);  // Add offset for easy identification
    
    Serial.println("  ✓ All signals sent!");
    Serial.println();
  }
}

// ============================================================
// RECEIVER FUNCTIONS
// ============================================================
void receiverLoop() {
  unsigned long currentTime = millis();
  
  // Show countdown
  static unsigned long lastCountdown = 0;
  if (currentTime - lastCountdown >= 1000) {
    lastCountdown = currentTime;
    unsigned long remaining = (RX_DURATION - (currentTime - modeStartTime)) / 1000;
    Serial.print("📻 RX Mode | ");
    Serial.print(remaining);
    Serial.print("s remaining | Received: ");
    Serial.println(rxModeTotal);
  }
  
  if (mySwitch.available()) {
    rxTotalReceived++;
    rxModeTotal++;
    unsigned long timeSinceLastSignal = currentTime - rxLastReceiveTime;
    rxLastReceiveTime = currentTime;
    
    unsigned long value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.println("⚠️  Unknown encoding (noise?)");
    } else {
      Serial.println();
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
      Serial.print("🎉 RECEIVED #");
      Serial.print(rxModeTotal);
      Serial.print(" (Total: ");
      Serial.print(rxTotalReceived);
      Serial.println(")");
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
      
      Serial.print("Value:       ");
      Serial.print(value);
      Serial.print(" (0x");
      Serial.print(value, HEX);
      Serial.println(")");
      
      Serial.print("Bit Length:  ");
      Serial.println(mySwitch.getReceivedBitlength());
      
      Serial.print("Protocol:    ");
      Serial.println(mySwitch.getReceivedProtocol());
      
      Serial.print("Pulse:       ");
      Serial.print(mySwitch.getReceivedDelay());
      Serial.println(" µs");
      
      // Identify signal
      Serial.print("Type:        ");
      if (value == 12345) {
        Serial.println("✓ Test Signal 1");
      } else if (value == 54321) {
        Serial.println("✓ Test Signal 2");
      } else if (value == 0xAAAAAA || value == 11184810) {
        Serial.println("✓ Test Signal 3");
      } else if (value > 1000 && value < 2000) {
        Serial.println("✓ Counter Signal");
      } else {
        Serial.println("Unknown (maybe external remote?)");
      }
      
      Serial.println();
    }
    
    mySwitch.resetAvailable();
  }
  
  delay(10);
}