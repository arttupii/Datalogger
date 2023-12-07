#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>

const int chipSelect = 10;
#define LED_PIN 3

bool recording = false;

void(* resetFunc) (void) = 0;

float getVoltage() {
  return 15.3;
}
float getCurrent() {
  return 3.4;
}

void dumpToFile() {
  File dataFile = SD.open("/datalog.txt", FILE_WRITE);
  time_t ltime;
  time(&ltime);
  dataFile.print(millis() / 1000); dataFile.print(";");
  dataFile.print(ctime(ltime)); dataFile.print(";");
  dataFile.print(temperatureC); dataFile.print(";");
  dataFile.print(getVoltage()); dataFile.print(";");
  dataFile.print(getCurrent()); dataFile.println(";");
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
    lcd_printf(1, "aika = %ds", interval);
  } else {
    lcd_printf(1, "aika = %dmin", interval / 60);
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
