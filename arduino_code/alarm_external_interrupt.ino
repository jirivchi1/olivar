#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>

const int relePin = 7;
const byte interruptPin = 2;

RTC_DS3231 rtc;

volatile bool alarmActivated = false;

// Horas de activación diarias (9:00, 12:00, 15:00, 18:00)
const int NUM_ALARMS = 4;
const int alarmHours[] = {9, 13, 15, 18};
const int alarmMinutes[] = {0, 0, 0, 0};

void wakeUpISR() {
  alarmActivated = true;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, LOW);

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), wakeUpISR, FALLING);

  // Set up Alarm 2 to fire at midnight every day
  rtc.setAlarm2(DateTime(0, 0, 0, 0, 0, 0), DS3231_A2_Hour);

  // Set up the first Alarm 1
  setNextAlarm();

  Serial.println("Setup complete. Entering low power mode.");
}

void loop() {

  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  if (alarmActivated) {
    alarmActivated = false;
    
    DateTime now = rtc.now();
    Serial.print("Woke up at: ");
    printDateTime(now);

    if (rtc.alarmFired(1)) {
      // Alarm 1 fired - this is our main alarm
      handleAlarm();
      setNextAlarm();
    }

    if (rtc.alarmFired(2)) {
      // Alarm 2 fired - this is our daily midnight alarm
      Serial.println("Midnight alarm - resetting daily alarms");
      setNextAlarm();
    }

    // Clear both alarms
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
  }
}

void handleAlarm() {
  Serial.println("Activating relay for 10 min...");
  digitalWrite(relePin, HIGH);
  delay(600000);
  digitalWrite(relePin, LOW);
  Serial.println("Relay deactivated.");
}

void setNextAlarm() {
  DateTime now = rtc.now();
  DateTime alarmTime = findNextAlarmTime(now);
  
  rtc.setAlarm1(alarmTime, DS3231_A1_Hour);
  
  Serial.print("Next alarm set for: ");
  printDateTime(alarmTime);
}

DateTime findNextAlarmTime(const DateTime& now) {
  for (int i = 0; i < NUM_ALARMS; i++) {
    DateTime alarmTime(now.year(), now.month(), now.day(), alarmHours[i], alarmMinutes[i], 0);
    if (alarmTime > now) {
      return alarmTime;
    }
  }
  // If no alarm today, set for first alarm tomorrow
  return DateTime(now.year(), now.month(), now.day() + 1, alarmHours[0], alarmMinutes[0], 0);
}

void printDateTime(const DateTime& dt) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", 
           dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
  Serial.println(buf);
}