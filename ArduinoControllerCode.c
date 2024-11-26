#include "Arduino.h"
#include "uRTCLib.h"

// uRTCLib rtc;
uRTCLib rtc(0x68);

char daysOfTheWeek[7][12] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"}; 

const int dry = 470;
const int wet = 190;
const int relayPin = 3;
#define SIZE 23
#define T1 27
#define T2 21
#define T3 15

int moistValues[SIZE] = {0};
int tempValues[SIZE] = {0};

void setup() {
  pinMode(relayPin, OUTPUT);
  Serial.begin(9600);

  //DS3231  
  delay(500); // wait for console opening

  URTCLIB_WIRE.begin();

  // Comment out below line once you set the date & time.
  // Following line sets the RTC with an explicit date & time
  rtc.set(0, 0, 22, 7, 6, 10, 24);
  // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
}

// Utility function to calculate the average of an array, ignoring zeros
float calculateAverage(int values[], int size) {
  int sum = 0, count = 0;
  for (int i = 0; i < size; i++) {
    if (values[i] != 0) {
      Serial.print("value[");Serial.print(i);Serial.print("] = ");
      Serial.println(values[i]);
      sum += values[i];
      count++;
    }
  }
  
  Serial.print("  sum = ");Serial.println(sum);
  Serial.print("count = ");Serial.println(count);
  
  return (count > 0) ? sum / float(count) : 0;
}

// Reset all values to 0
void resetLists(int values[], int size) {
  for (int i = 0; i < size; i++) {
    values[i] = 0;
  }
}

// Watering function
static void wateringByMin(int n) {
  Serial.print("Relay On for ");
  Serial.print(n);
  Serial.println(" minutes!");
  digitalWrite(relayPin, LOW); // Activate relay
  delay(1000 * n); // Water for n minutes
  
  Serial.println("Relay Off");
  digitalWrite(relayPin, HIGH); // Deactivate relay
}

// Function to determine watering time based on moisture and temperature
void determineWateringTime(int averageMoist, int averageTemp) {
  int wateringTime = 0;
  
  if (averageMoist >= 45) {
    return; // Enough moisture, no need to water
  } else if (averageMoist >= 38) {
    wateringTime = (averageTemp >= T1) ? 15 : (averageTemp >= T2) ? 10 : (averageTemp >= T3) ? 5 : 1;
  } else if (averageMoist >= 31) {
    wateringTime = (averageTemp >= T1) ? 21 : (averageTemp > T2) ? 14 : (averageTemp >= T3) ? 7 : 2;
  } else if (averageMoist >= 24) {
    wateringTime = (averageTemp >= T1) ? 27 : (averageTemp >= T2) ? 18 : (averageTemp >= T3) ? 9 : 3;
  } else if (averageMoist >= 17) {
    wateringTime = (averageTemp >= T1) ? 33 : (averageTemp >= T2) ? 22 : (averageTemp >= T3) ? 11 : 4;
  } else if (averageMoist >= 10) {
    wateringTime = (averageTemp >= T1) ? 39 : (averageTemp >= T2) ? 26 : (averageTemp >= T3) ? 13 : 5;
  } else {
    wateringTime = (averageTemp >= T1) ? 45 : (averageTemp >= T2) ? 30 : (averageTemp >= T3) ? 15 : 6;
  }

  if (wateringTime > 0) {
    wateringByMin(wateringTime);
  }
  else {
    Serial.println("No need to water!");
  }
}

void loop() {
  rtc.refresh();
  int sensorVal = analogRead(A0);

  // Display current date and time
  Serial.print(rtc.year()); Serial.print('/');
  Serial.print(rtc.month()); Serial.print('/');
  Serial.print(rtc.day()); Serial.print(" (");
  Serial.print(daysOfTheWeek[rtc.dayOfWeek() - 1]); Serial.print(") ");
  Serial.print(rtc.hour()); Serial.print(':');
  Serial.print(rtc.minute()); Serial.print(':');
  Serial.println(rtc.second());

  // Measure moisture and temperature every 30 minutes (except at 00:30)
  if (rtc.hour() != 2 && rtc.minute() == 30) {
    int avgMoist = map(sensorVal, wet, dry, 100, 0);
    int index = rtc.hour() - 1;
    moistValues[index] = avgMoist;
    tempValues[index] = rtc.temp() / 100;
  
      Serial.print("   Moisture at ");
      Serial.print(rtc.hour());
      Serial.print(":");
      Serial.print(rtc.minute());
      Serial.print(" = ");
      Serial.println(moistValues[index]);
  
      Serial.print("Temperature at ");
      Serial.print(rtc.hour());
      Serial.print(":");
      Serial.print(rtc.minute());
      Serial.print(" = ");
      Serial.println(tempValues[index]);
  }

  // At 00:30, check if watering is needed
  if (rtc.hour() == 2 && rtc.minute() == 30) {
    // Calculate average moisture and temperature
    float avgMoist = calculateAverage(moistValues, SIZE);
    Serial.print("   Average Moisture: ");
    Serial.println((int)avgMoist);
    
    float avgTemp = calculateAverage(tempValues, SIZE);
    Serial.print("Average Temperature: ");
    Serial.println((int)avgTemp);

    // Determine watering time
    determineWateringTime((int)avgMoist, (int)avgTemp);

    // Reset moisture and temperature values to 0
    resetLists(moistValues, SIZE);
    resetLists(tempValues, SIZE);
  }

  //Serial.println("OVER");
  delay(60000); // Wait 1 minute
}