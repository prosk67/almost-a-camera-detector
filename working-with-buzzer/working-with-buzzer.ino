// IR Camera Detector - Complete Working Version
// Detects IR light from security cameras

// Sensor pins
#define CNY70_PIN 32
#define IR_RECEIVER_PIN 26
#define PHOTODIODE_AO 35
#define PHOTODIODE_DO 34
#define LED_PIN 2
#define BUZZER_PIN 19

// Optimized thresholds
#define IR_RECEIVER_THRESHOLD 400
#define PHOTODIODE_THRESHOLD 600
#define CNY70_THRESHOLD 3000
#define CNY70_ENABLED true
#define SAMPLES 15

// Detection requirements
#define CONSECUTIVE_NEEDED 2
#define DETECTION_CONFIDENCE 1

int consecutiveDetections = 0;
int cny70_baseline = 0;
int ir_receiver_baseline = 0;
int photodiode_baseline = 0;

// Detection history
int detectionHistory[5] = {0, 0, 0, 0, 0};
int historyIndex = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(CNY70_PIN, INPUT);
  pinMode(IR_RECEIVER_PIN, INPUT);
  pinMode(PHOTODIODE_AO, INPUT);
  pinMode(PHOTODIODE_DO, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  IR CAMERA DETECTOR - READY TO SCAN   ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  delay(2000);
  calibrateSensors();
}

void loop() {
  // Read all sensors
  int cny70 = readSensor(CNY70_PIN);
  int irReceiver = readSensor(IR_RECEIVER_PIN);
  int photodiodeAnalog = readSensor(PHOTODIODE_AO);
  int photodiodeDigital = digitalRead(PHOTODIODE_DO);
  
  // Calculate deviations
  int cny70_deviation = abs(cny70 - cny70_baseline);
  int ir_deviation = abs(irReceiver - ir_receiver_baseline);
  int photo_deviation = abs(photodiodeAnalog - photodiode_baseline);
  
  // Detection logic
  bool cny70_detect = false;
  if (CNY70_ENABLED) {
    cny70_detect = (cny70 < 1000 && cny70_deviation > CNY70_THRESHOLD);
  }
  
  bool ir_receiver_detect = false;
  if (ir_receiver_baseline < 4000) {
    ir_receiver_detect = (irReceiver < (ir_receiver_baseline - IR_RECEIVER_THRESHOLD));
  }
  
  bool photo_analog_detect = (photodiodeAnalog > (photodiode_baseline + PHOTODIODE_THRESHOLD)) ||
                             (photodiodeAnalog < (photodiode_baseline - PHOTODIODE_THRESHOLD));
  
  bool photo_digital_detect = (photodiodeDigital == LOW);
  
  // Count detections
  int detectionCount = 0;
  if (cny70_detect) detectionCount++;
  if (ir_receiver_detect) detectionCount++;
  if (photo_analog_detect) detectionCount++;
  if (photo_digital_detect) detectionCount++;
  
  // Update history
  detectionHistory[historyIndex] = detectionCount;
  historyIndex = (historyIndex + 1) % 5;
  
  // Calculate average
  int recentDetections = 0;
  for(int i = 0; i < 5; i++) {
    recentDetections += detectionHistory[i];
  }
  float avgDetections = recentDetections / 5.0;
  
  // Display readings
  Serial.println("\n┌─────────────────────────────────────┐");
  Serial.println("│        SENSOR STATUS                │");
  Serial.println("├─────────────────────────────────────┤");
  
  Serial.print("│ CNY70:      ");
  Serial.print(cny70);
  Serial.print(" (Δ");
  Serial.print(cny70_deviation);
  Serial.print(")");
  if (!CNY70_ENABLED) {
    Serial.print(" [DISABLED]");
  } else if (cny70_detect) {
    Serial.print(" ✓✓✓");
  }
  Serial.println();
  
  Serial.print("│ IR Rx:      ");
  Serial.print(irReceiver);
  Serial.print(" (Δ");
  Serial.print(ir_deviation);
  Serial.print(")");
  if (ir_receiver_baseline >= 4000) {
    Serial.print(" [BROKEN]");
  } else if (ir_receiver_detect) {
    Serial.print(" ✓✓✓");
  }
  Serial.println();
  
  Serial.print("│ Photo (AO): ");
  Serial.print(photodiodeAnalog);
  Serial.print(" (Δ");
  Serial.print(photo_deviation);
  Serial.print(")");
  if (photo_analog_detect) Serial.print(" ✓✓✓");
  Serial.println();
  
  Serial.print("│ Photo (DO): ");
  Serial.print(photodiodeDigital == LOW ? "ACTIVE  " : "INACTIVE");
  if (photo_digital_detect) Serial.print(" ✓✓✓");
  Serial.println();
  
  Serial.println("├─────────────────────────────────────┤");
  
  // Update streak
  if (detectionCount >= DETECTION_CONFIDENCE) {
    consecutiveDetections++;
  } else {
    consecutiveDetections = 0;
  }
  
  Serial.print("│ Active: ");
  Serial.print(detectionCount);
  Serial.print("/");
  int totalSensors = (CNY70_ENABLED ? 1 : 0) + 
                     (ir_receiver_baseline < 4000 ? 1 : 0) + 2;
  Serial.print(totalSensors);
  Serial.print(" │ Streak: ");
  Serial.print(consecutiveDetections);
  Serial.print("/");
  Serial.print(CONSECUTIVE_NEEDED);
  Serial.print(" │ Avg: ");
  Serial.print(avgDetections, 1);
  Serial.println("    │");
  Serial.println("└─────────────────────────────────────┘");
  
  // Alert logic with buzzer
  if (consecutiveDetections >= CONSECUTIVE_NEEDED && avgDetections >= 0.8) {
    Serial.println("\n🔴 CAMERA DETECTED! 🔴");
    Serial.println("▶ IR source confirmed at this location");
    digitalWrite(LED_PIN, HIGH);
    
    // 3 ascending beeps - CAMERA FOUND ALERT
    tone(BUZZER_PIN, 1000);
    delay(150);
    noTone(BUZZER_PIN);
    delay(50);
    tone(BUZZER_PIN, 1200);
    delay(150);
    noTone(BUZZER_PIN);
    delay(50);
    tone(BUZZER_PIN, 1500);
    delay(300);
    noTone(BUZZER_PIN);
    
  } else if (detectionCount >= 1) {
    Serial.println("\n🟡 Possible IR Source");
    Serial.println("▶ Keep scanning this area...");
    
    // Single short beep for possible detection
    tone(BUZZER_PIN, 800);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);
    
  } else {
    Serial.println("\n🟢 Normal - No IR detected");
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
  
  delay(200);
}

int readSensor(int pin) {
  long sum = 0;
  for(int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(500);
  }
  return sum / SAMPLES;
}

void calibrateSensors() {
  Serial.println("\n┌─────────────────────────────────────┐");
  Serial.println("│         CALIBRATION MODE            │");
  Serial.println("└─────────────────────────────────────┘");
  Serial.println("\n📌 Point sensors AWAY from IR sources");
  Serial.println("📌 Keep distance steady for CNY70");
  Serial.println("\nStarting calibration in:");
  
  for(int i = 5; i > 0; i--) {
    Serial.print("  ");
    Serial.print(i);
    Serial.println(" seconds...");
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(700);
  }
  
  Serial.println("\n⏳ Sampling 150 readings...");
  
  long cny70_sum = 0;
  long ir_sum = 0;
  long photo_sum = 0;
  
  for(int i = 0; i < 150; i++) {
    cny70_sum += analogRead(CNY70_PIN);
    ir_sum += analogRead(IR_RECEIVER_PIN);
    photo_sum += analogRead(PHOTODIODE_AO);
    
    if (i % 15 == 0) {
      Serial.print("█");
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
    delay(40);
  }
  
  Serial.println(" ✓");
  
  cny70_baseline = cny70_sum / 150;
  ir_receiver_baseline = ir_sum / 150;
  photodiode_baseline = photo_sum / 150;
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║      CALIBRATION COMPLETE ✓           ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.print("║ CNY70 Baseline:      ");
  Serial.print(cny70_baseline);
  if (cny70_baseline < 4000) Serial.print("    ");
  Serial.println("        ║");
  
  Serial.print("║ IR Receiver Baseline: ");
  Serial.print(ir_receiver_baseline);
  if (ir_receiver_baseline < 4000) Serial.print("   ");
  Serial.println("        ║");
  
  Serial.print("║ Photodiode Baseline:  ");
  Serial.print(photodiode_baseline);
  if (photodiode_baseline < 4000) Serial.print("   ");
  Serial.println("        ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  // Sensor health check
  Serial.println("\n📊 SENSOR HEALTH:");
  
  if (CNY70_ENABLED) {
    if (cny70_baseline >= 4000) {
      Serial.println("⚠  CNY70: Will only work close to IR source (5-15cm)");
    } else if (cny70_baseline < 500) {
      Serial.println("✓  CNY70: Good - optimal for IR detection");
    } else {
      Serial.println("✓  CNY70: Operational");
    }
  } else {
    Serial.println("⚠  CNY70: DISABLED");
  }
  
  if (ir_receiver_baseline >= 4000) {
    Serial.println("⚠  IR Receiver: Saturated - check wiring");
  } else if (ir_receiver_baseline < 1000) {
    Serial.println("⚠  IR Receiver: Too low - check connections");
  } else {
    Serial.println("✓  IR Receiver: Operational");
  }
  
  if (photodiode_baseline >= 3500) {
    Serial.println("✓  Photodiode: Good sensitivity");
  } else {
    Serial.println("ℹ  Photodiode: Adjust potentiometer for more sensitivity");
  }
  
  Serial.println("\n🔍 DETECTION STARTED - Scan slowly!");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  digitalWrite(LED_PIN, LOW);
  delay(2000);
}