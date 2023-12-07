#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

DeviceAddress insideThermometer;
void printAddress(DeviceAddress deviceAddress);

void initDallas() {
  // locate devices on the bus
  Serial.print(F("Locating devices..."));
  sensors.begin();
  Serial.print(F("Found "));
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(F(" devices."));

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  Serial.print(F("Device 0 Address: "));
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);

  Serial.print(F("Device 0 Resolution: "));
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
float temperatureC = 0;

float getTemperature(){
  return temperatureC;
}
void task_dallas() {
  static long tm = 0;
  if (millis() - tm > 5000) {
    tm = millis();

    sensors.requestTemperatures();
    temperatureC = sensors.getTempC(insideThermometer);
    if (temperatureC == DEVICE_DISCONNECTED_C)
    {
      Serial.println(F("Error: Could not read temperature data"));
      return;
    }
    Serial.print(F("Temp C: "));
    Serial.println(temperatureC);
  }
}
