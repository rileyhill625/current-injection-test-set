#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_INA219.h>

Adafruit_MCP4725 dac;
Adafruit_INA219 ina219;

// ---- Current source calibration ----
// 10-ohm sense resistor: command_voltage = target_mA/1000 * 10
// DAC 12-bit (0-4095) over measured ~4.6V VCC
const float VCC = 4.6;
const float R_SENSE = 10.0;

// ---- Relay (IDMT) settings ----
const float I_PICKUP = 100.0;   // mA
const float TMS = 0.10;
const float K = 0.14, A = 0.02;
const int RELAY_PIN = 8;

// ---- Command: set the injected current ----
float commanded_mA = 110.0;     

// ---- State ----
float trip_accumulator = 0.0;
unsigned long last_time = 0;
bool tripped = false;
int print_counter = 0;

void setCurrent_mA(float target_mA) {
  float command_voltage = (target_mA / 1000.0) * R_SENSE;
  int dac_value = (int)((command_voltage / VCC) * 4095);
  if (dac_value > 4095) dac_value = 4095;
  if (dac_value < 0) dac_value = 0;
  dac.setVoltage(dac_value, false);
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);   // relay inactive = contact closed = load energized
  dac.begin(0x60);
  if (!ina219.begin()) { Serial.println("INA219 not found"); while(1); }
  Serial.println("Injection test set + relay ready.");
  setCurrent_mA(commanded_mA);
  last_time = millis();
}

void loop() {
  // measure the actual injected current
  float measured = ina219.getCurrent_mA();
  if (measured < 0) measured = -measured;

  unsigned long now = millis();
  float dt = (now - last_time) / 1000.0;
  last_time = now;

  print_counter++;
  bool show = (print_counter % 5 == 0);

  if (tripped) {
    if (measured < I_PICKUP) {
      tripped = false; trip_accumulator = 0;
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay reset.");
    }
    delay(100);
    return;
  }

  if (measured > I_PICKUP) {
    float M = measured / I_PICKUP;
    float denom = pow(M, A) - 1.0;
    if (denom < 0.0001) denom = 0.0001;
    float trip_time = TMS * (K / denom);
    trip_accumulator += dt / trip_time;
    if (trip_accumulator > 1.0) trip_accumulator = 1.0;

    if (show) {
      Serial.print("commanded="); Serial.print(commanded_mA);
      Serial.print(" measured="); Serial.print(measured);
      Serial.print(" mA  trip_time="); Serial.print(trip_time, 2);
      Serial.print("s  progress="); Serial.print(trip_accumulator*100, 0); Serial.println("%");
    }

    if (trip_accumulator >= 1.0) {
      tripped = true;
      digitalWrite(RELAY_PIN, LOW);    // activate relay = trip
      Serial.print(">>> TRIP! Injected current: "); Serial.print(measured); Serial.println(" mA <<<");
    }
  } else {
    trip_accumulator -= dt * 0.5;
    if (trip_accumulator < 0) trip_accumulator = 0;
    if (show) {
      Serial.print("commanded="); Serial.print(commanded_mA);
      Serial.print(" measured="); Serial.print(measured); Serial.println(" mA  (normal)");
    }
  }
  delay(100);
}