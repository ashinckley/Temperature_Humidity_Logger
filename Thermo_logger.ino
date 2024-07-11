#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>

// Pin definitions
const uint8_t SD_CS_PIN = 10; // Chip select pin for SD card
const int DHTPin = 4;         // DHT11 sensor connected to D4
#define DHTTYPE DHT11

// Create objects for the sensors and SD card
DHT dht(DHTPin, DHTTYPE);
RTC_DS1307 rtc;
SdFat sd;
File dataFile;

char fileName[32];

// Function to return the current date and time
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();
  // Return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());
  // Return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");

  // Initialize DHT sensor
  dht.begin();

  // Initialize SD card
  Serial.println("Initializing SD card...");
  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(4))) {
    Serial.println("SD card initialization failed!");
    while (true); // Stay in a loop if the SD card initialization fails
  }
  Serial.println("SD card initialized.");

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (true); // Stay in a loop if the RTC initialization fails
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // Set the date and time to the time when this code is compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("RTC has been set.");
  }

  // Set callback for file timestamps
  SdFile::dateTimeCallback(dateTime);

  // Create a new file named with the current date and time
  DateTime now = rtc.now();
  sprintf(fileName, "/%04d%02d%02d_%02d%02d%02d.txt", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  dataFile = sd.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.println("Temperature and Humidity Log");
    dataFile.println("DateTime, Temperature(F), Humidity(%)");
    dataFile.close();
    Serial.print("Created file: ");
    Serial.println(fileName);
  } else {
    Serial.println("Failed to create file!");
    while (true); // Stay in a loop if the file creation fails
  }

  Serial.println("Setup complete.");
}

void loop() {
  Serial.println("Loop started...");

  // Read temperature and humidity from DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(5000); // Wait for 5 seconds before trying again
    return;
  }

  // Convert temperature to Fahrenheit
  float tF = t * 9.0 / 5.0 + 32.0;

  DateTime now = rtc.now();

  // Print to serial monitor
  Serial.print("Temperature: ");
  Serial.print(tF);
  Serial.print(" °F, Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("DateTime: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Log temperature and humidity to SD card
  dataFile = sd.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(" ");
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    dataFile.print(now.second(), DEC);
    dataFile.print(", ");
    dataFile.print(tF);
    dataFile.print(", ");
    dataFile.println(h);
    dataFile.close();
  } else {
    Serial.println("Failed to open file for writing!");
  }

  Serial.println("Loop complete.");

  delay(30000); // 30 seconds delay between readings
}
