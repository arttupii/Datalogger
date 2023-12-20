#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>
#include<ADS1115_WE.h>
#include<Wire.h>
#include <EEPROM.h>

#define ADC_I2C_ADDRESS 0x48

ADS1115_WE adc = ADS1115_WE(ADC_I2C_ADDRESS);

const int chipSelect = 10;
#define LED_PIN 3

bool recording = false;

void(* resetFunc) (void) = 0;

#define UPDATE_CALIBRATION_VALUE(x) EEPROM.put(0x00, x)
#define GET_CALIBRATION_VALUE(x) EEPROM.get(0x00, x)

void calibrateCurrent(){
   lcd_printP(0, F("Kalibrointi"));
   lcd_printP(1, F("Aloitettu"));

   adc.setVoltageRange_mV(ADS1115_RANGE_0256); 
   
   delay(1000);
   lcd.setCursor(0, 1);
   float c = 0.0;
   lcd.print(F("|          |"));
   lcd.setCursor(1, 1);
   for(int i=0;i<10;i++) {
        lcd.print(F("-"));
        c+=readChannel(ADS1115_COMP_2_3);
        delay(500);
   }
   lcd_printP(0, F("Kalibrointi"));
   lcd_printP(1, F("Valmis"));
   UPDATE_CALIBRATION_VALUE(c);
   delay(3000);
}

void initADC() {

  if (!adc.init()) {
    Serial.println("ADS1115 not connected!");
  }


  adc.setVoltageRange_mV(ADS1115_RANGE_4096); 

  adc.setCompareChannels(ADS1115_COMP_0_1); //comment line/change parameter to change channel
  adc.setConvRate(ADS1115_860_SPS); //uncomment if you want to change the default
  adc.setMeasureMode(ADS1115_SINGLE); //comment line/change parameter to change mode

  adc.setPermanentAutoRangeMode(false);
}
float getVoltage() {
  adc.setVoltageRange_mV(ADS1115_RANGE_4096); 
  float t =  readChannel(ADS1115_COMP_0_1) * 15.0;
  Serial.println(t);
  return t;
}
float getCurrent() {
  float c;
  GET_CALIBRATION_VALUE(c);
  
  adc.setVoltageRange_mV(ADS1115_RANGE_0256); 
  
  float i = readChannel(ADS1115_COMP_2_3);
    
  return (i-c)*1000;
}

float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);

  for (int i = 0; i < 10; i++) {
    adc.startSingleMeasurement();
    while (adc.isBusy()) {}
    voltage += adc.getResult_V(); // alternative: getResult_mV for Millivolt
    delay(1);
  }
  return voltage / 10.0;
}

void dumpToFile() {
  File dataFile = SD.open("/datalog.txt", FILE_WRITE);
  DateTime now = rtc.now();

  dataFile.print(millis() / 1000); dataFile.print(F(";"));

  dataFile.print(now.year()); dataFile.print(F("-")); dataFile.print(now.month()); dataFile.print(F("-")); dataFile.print(now.day()); dataFile.print(F(" "));
  dataFile.print(now.hour()); dataFile.print(F(":")); dataFile.print(now.minute()); dataFile.print(F(":")); dataFile.print(now.second()); dataFile.print(F(";"));


  dataFile.print(floatToStr(temperatureC)); dataFile.print(F(";"));
  dataFile.print(floatToStr(getVoltage())); dataFile.print(F(";"));
  dataFile.print(floatToStr(getCurrent())); dataFile.println(F(";"));
  dataFile.close();
}

void task_led() {
  static long tm = millis();
  static unsigned char i = 0;
  if (millis() - tm > 10) {
    tm = millis();
    i++;
    if (recording && (i % 100 == 0)) {
      pinMode(LED_PIN, OUTPUT);
      digitalWrite(LED_PIN, 1);
    } else {
      pinMode(LED_PIN, OUTPUT);
      digitalWrite(LED_PIN, 0);
    }
  }
}

int task_start_recording(bool init) {
  static int interval;
  static bool mounted = false;
  static File dataFile;

  button.update();

  if (button.button_combo()) {
    if (mounted) {
      mounted = false;
    }
    recording = false;
    lcd_on_off(1);
    lcd_printP(0, F("VOIT POISTAA"));
    lcd_printP(1, F("MUISTIKORTTIN"));
    delay(10000);
    resetFunc();
    return 0;
  }

  if (init) {
    Serial.println(F("Init recording"));
    EEPROM.get(0x00, interval);
    mounted = false;
    lcd_printP(0, F("ALUSTETAAN"));
    lcd_printP(1, F("MUISTIKORTTI"));
  }

  if (!mounted) {
    if (!SD.begin()) {
      Serial.println(F("Card failed, or not present"));
      mounted = false;
      lcd_printP(0, F("MKORTTI VIRHE!"));
      lcd_printP(1, F(""));
    } else {
      mounted = true;
      Serial.println(F("Card ok"));
    }
    return 1;
  }
  recording = true;
  lcd_printP(0, F("Tallennetaan...."));
  {
    static unsigned long tm = millis();
    if (millis() - tm > 500) {
      tm = millis();
      static char i = 0;
      i++;
      if (i & 0x1) {
        lcd_printP(1, F(" * * * * * * * *"));
      } else {
        lcd_printP(1, F("* * * * * * * * "));
      }
    }
  }
  {
    static unsigned long tm = millis();
    if (millis() >= tm) {
      tm = tm + (1 * 1000);
      Serial.println(F("Record to file"));
      dumpToFile();
    }
  }
  return 1;
}


int task_interval_set(bool init) {
  button.update();
  static int interval;

  if (init) {
    EEPROM.get(0x00, interval);
    return 1;
  }
  lcd_printP(0, F("Mittausvali"));
  if (interval <= 60) {
    lcd_printf(1, F("aika = %ds"), interval);
  } else {
    lcd_printf(1, F("aika = %dmin"), interval / 60);
  }
  if (button.button_up()) {
    if (interval < 5) {
      interval++;
    } else if (interval < 30) {
      interval += 5;
    } else if (interval < 60) {
      interval += 15;
    } else {
      interval += 60;
    }
  }
  if (button.button_down()) {
    if (interval > 60) {
      interval -= 60;
    } else if (interval > 5) {
      interval -= 5;
    } else {
      interval--;
    }
  }

  if (interval < 1) {
    interval = 1800;
  }
  if (interval > 1800) {
    interval = 1;
  }

  if (button.button_ok()) {
    EEPROM.put(0x00, interval);
    return 0;
  }
  return 1;
}
