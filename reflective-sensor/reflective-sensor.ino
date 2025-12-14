/*
 * ESP32 CNY70 Reflective Sensor - Comprehensive Logger (CORRECTED)
 * GPIO 32 Configuration
 * 
 * WIRING:
 * CNY70 Pin 1 -> 220Ω -> 3.3V
 * CNY70 Pin 2 -> GND
 * CNY70 Pin 3 -> 10kΩ -> 3.3V + GPIO 32
 * CNY70 Pin 4 -> GND
 */

const int CNY70_PIN = 32;  // GPIO 32 - reliable ADC pin

// Statistics tracking
unsigned long totalReadings = 0;
unsigned long detectionCount = 0;
unsigned long lastDetectionTime = 0;
bool currentlyDetecting = false;

// Rolling average for smoothing
const int SAMPLES = 10;
int readings[SAMPLES];
int readIndex = 0;
int total = 0;
int average = 0;

// Calibration
int minValue = 4095;
int maxValue = 0;
int threshold = 2048;
bool calibrated = false;

// Timing
unsigned long sessionStart = 0;
unsigned long lastLogTime = 0;
const int LOG_INTERVAL = 200;  // Log every 200ms

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════════════════════════╗");
  Serial.println("║     ESP32 CNY70 REFLECTIVE SENSOR - COMPREHENSIVE LOGGER   ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝");
  Serial.println();
  Serial.print("📍 Sensor Pin: GPIO ");
  Serial.println(CNY70_PIN);
  Serial.print("🕐 Session Start: ");
  Serial.println(millis());
  Serial.println("════════════════════════════════════════════════════════════");
  Serial.println();
  
  // Initialize rolling average array
  for (int i = 0; i < SAMPLES; i++) {
    readings[i] = 0;
  }
  
  // Auto-calibration
  Serial.println("🔧 AUTO-CALIBRATION MODE (8 seconds)");
  Serial.println("   ⚠️  IMPORTANT: Move sensor NOW!");
  Serial.println("   1. Hold sensor in the air (nothing close) - 2 seconds");
  Serial.println("   2. Hold WHITE paper 2-5mm away - 3 seconds");
  Serial.println("   3. Hold BLACK surface 2-5mm away - 3 seconds");
  Serial.println();
  
  unsigned long calibStart = millis();
  int calibCount = 0;
  
  while (millis() - calibStart < 8000) {  // Extended to 8 seconds
    int reading = analogRead(CNY70_PIN);
    calibCount++;
    
    if (reading < minValue) minValue = reading;
    if (reading > maxValue) maxValue = reading;
    
    // Update every 100ms
    if (calibCount % 10 == 0) {
      Serial.print("   [");
      int elapsed = (millis() - calibStart) / 1000;
      for (int i = 0; i < 8; i++) {
        Serial.print(i < elapsed ? "█" : "░");
      }
      Serial.print("] Current: ");
      Serial.print(reading);
      Serial.print(" | Min: ");
      Serial.print(minValue);
      Serial.print(" | Max: ");
      Serial.println(maxValue);
    }
    delay(100);
  }
  
  threshold = (minValue + maxValue) / 2;
  calibrated = true;
  sessionStart = millis();
  
  Serial.println();
  Serial.println("✅ CALIBRATION COMPLETE!");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.print("   Minimum (White):    ");
  Serial.print(minValue);
  Serial.println(" / 4095");
  Serial.print("   Maximum (Black):    ");
  Serial.print(maxValue);
  Serial.println(" / 4095");
  Serial.print("   Threshold:          ");
  Serial.println(threshold);
  Serial.print("   Range:              ");
  Serial.print(maxValue - minValue);
  Serial.println(" (higher is better)");
  
  // Range quality assessment
  int range = maxValue - minValue;
  Serial.print("   Quality:            ");
  if (range > 2000) {
    Serial.println("EXCELLENT ✨");
  } else if (range > 1000) {
    Serial.println("GOOD ✓");
  } else if (range > 500) {
    Serial.println("MODERATE ⚠");
  } else {
    Serial.println("POOR ⚠⚠⚠");
    Serial.println();
    Serial.println("   ⚠ LOW RANGE DETECTED! Try:");
    Serial.println("      - Position sensor 2-5mm from surface");
    Serial.println("      - Use high-contrast surfaces (white paper vs black tape)");
    Serial.println("      - Check resistor values (220Ω and 10kΩ)");
  }
  
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println();
  Serial.println("📊 STARTING CONTINUOUS MONITORING...");
  Serial.println();
  printHeader();
}

void loop() {
  // Read sensor
  int rawValue = analogRead(CNY70_PIN);
  totalReadings++;
  
  // Update rolling average
  total = total - readings[readIndex];
  readings[readIndex] = rawValue;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % SAMPLES;
  average = total / SAMPLES;
  
  // Calculate percentage (0% = white, 100% = black)
  int percentage = 0;
  if (maxValue > minValue) {
    percentage = map(rawValue, minValue, maxValue, 0, 100);
    percentage = constrain(percentage, 0, 100);
  }
  
  // Digital detection state (CORRECTED: LOW value = white/detected)
  bool detected = (rawValue < threshold);
  
  // Edge detection
  if (detected && !currentlyDetecting) {
    detectionCount++;
    lastDetectionTime = millis();
    currentlyDetecting = true;
    
    Serial.println();
    Serial.println("🎉━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.print("   ⚡ DETECTION EVENT #");
    Serial.print(detectionCount);
    Serial.print(" at ");
    Serial.print((millis() - sessionStart) / 1000.0, 3);
    Serial.println("s");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println();
  } else if (!detected && currentlyDetecting) {
    unsigned long duration = millis() - lastDetectionTime;
    currentlyDetecting = false;
    
    Serial.println();
    Serial.println("👋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.print("   ↓ Detection ended (duration: ");
    Serial.print(duration);
    Serial.println("ms)");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println();
  }
  
  // Comprehensive logging
  if (millis() - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = millis();
    printDetailedLog(rawValue, average, percentage, detected);
  }
  
  delay(20);  // 50Hz sampling
}

void printHeader() {
  Serial.println("┌──────┬─────────┬─────────┬──────┬──────────┬────────┬──────────────────┬──────────┐");
  Serial.println("│ Time │   Raw   │  Avg    │  %   │ Digital  │ State  │   Visual Bar     │  Surface │");
  Serial.println("├──────┼─────────┼─────────┼──────┼──────────┼────────┼──────────────────┼──────────┤");
}

void printDetailedLog(int raw, int avg, int pct, bool detected) {
  static int lineCount = 0;
  
  // Reprint header every 20 lines
  if (lineCount > 0 && lineCount % 20 == 0) {
    Serial.println("└──────┴─────────┴─────────┴──────┴──────────┴────────┴──────────────────┴──────────┘");
    Serial.println();
    printHeader();
  }
  lineCount++;
  
  // Time column
  Serial.print("│");
  char timeStr[7];
  sprintf(timeStr, "%5lu", (millis() - sessionStart) / 1000);
  Serial.print(timeStr);
  Serial.print("s");
  
  // Raw value column
  Serial.print("│ ");
  char rawStr[8];
  sprintf(rawStr, "%4d", raw);
  Serial.print(rawStr);
  Serial.print("   ");
  
  // Average value column
  Serial.print("│ ");
  char avgStr[8];
  sprintf(avgStr, "%4d", avg);
  Serial.print(avgStr);
  Serial.print("   ");
  
  // Percentage column
  Serial.print("│ ");
  char pctStr[5];
  sprintf(pctStr, "%3d", pct);
  Serial.print(pctStr);
  Serial.print("%");
  
  // Digital state column (CORRECTED)
  Serial.print("│ ");
  if (raw < threshold) {
    Serial.print("  LOW   ");
  } else {
    Serial.print("  HIGH  ");
  }
  
  // Detection state column (CORRECTED)
  Serial.print("│ ");
  if (detected) {
    Serial.print(" 🔴ON ");
  } else {
    Serial.print(" ⚫OFF");
  }
  Serial.print(" ");
  
  // Visual bar graph (inverted - more bars = darker)
  Serial.print("│ ");
  int bars = map(pct, 0, 100, 0, 15);
  bars = constrain(bars, 0, 15);
  for (int i = 0; i < 15; i++) {
    if (i < bars) Serial.print("█");
    else Serial.print("░");
  }
  Serial.print(" ");
  
  // Surface type column (CORRECTED)
  Serial.print("│ ");
  if (pct < 25) {
    Serial.print("WHITE ⚪");
  } else if (pct < 50) {
    Serial.print("LIGHT 🔘");
  } else if (pct < 75) {
    Serial.print("GRAY  🔘");
  } else {
    Serial.print("BLACK ⚫");
  }
  Serial.print(" ");
  
  Serial.println("│");
  
  // Print summary every 50 lines
  if (lineCount % 50 == 0) {
    Serial.println("└──────┴─────────┴─────────┴──────┴──────────┴────────┴──────────────────┴──────────┘");
    printSessionStats();
    Serial.println();
    printHeader();
  }
}

void printSessionStats() {
  Serial.println();
  Serial.println("📈 SESSION STATISTICS");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.print("   Total Readings:      ");
  Serial.println(totalReadings);
  Serial.print("   Detection Events:    ");
  Serial.println(detectionCount);
  Serial.print("   Session Duration:    ");
  Serial.print((millis() - sessionStart) / 1000.0, 1);
  Serial.println("s");
  Serial.print("   Avg Reading Rate:    ");
  Serial.print(totalReadings / ((millis() - sessionStart) / 1000.0), 1);
  Serial.println(" readings/sec");
  if (detectionCount > 0) {
    Serial.print("   Avg Detection Rate:  ");
    Serial.print(detectionCount / ((millis() - sessionStart) / 1000.0), 2);
    Serial.println(" events/sec");
  }
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
}