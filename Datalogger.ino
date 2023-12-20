#include <LiquidCrystal_I2C.h>
#include  <Wire.h>
#include<ADS1115_WE.h>
#include<Wire.h>
#include "RTClib.h"

//RTClib

RTC_DS3231 rtc;

#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_OK 3

int menulevel = 0;

LiquidCrystal_I2C lcd(0x27,  16, 2);
int pin_button_up = 4;
int pin_button_down = 5;
int pin_button_ok = 6;

class Button {
  public:
    static unsigned long tm;

    Button(unsigned filter_time) {
      static bool init = true;
      if (init) {
        init = false;
        pinMode(pin_button_up, INPUT_PULLUP);
        pinMode(pin_button_down, INPUT_PULLUP);
        pinMode(pin_button_ok, INPUT_PULLUP);
        Serial.println("Button init");
      }
    }
    ~Button() {}

    int update() {
      char up = digitalRead(pin_button_up) == 0;
      char down = digitalRead(pin_button_down) == 0;
      char ok = digitalRead(pin_button_ok) == 0;
      char combo =  (digitalRead(pin_button_up) == 0) && (digitalRead(pin_button_ok) == 0);
      _up_now = 0;
      _down_now = 0;
      _ok_now = 0;

      static unsigned long tm = millis();
      if (millis() - tm > 10) {
        tm = millis();
        if (up != _up_last) _up_now = up;
        if (down != _down_last) _down_now = down;
        if (ok != _ok_last) _ok_now = ok;
        _up_last = up;
        _down_last = down;
        _ok_last = ok;
      }
      if (combo) {
        tm = millis();
        _combo_now = combo;
      }
    }

    int button_any() {
      return digitalRead(pin_button_up) == 0 || digitalRead(pin_button_down) == 0 || digitalRead(pin_button_ok) == 0;
    }
    int button_combo() {
      return _combo_now;
    }
    int button_up() {
      return _up_now;
    }
    int button_down() {
      return _down_now;
    }
    int button_ok() {
      return _ok_now;
    }

  private:
    char _up_now, _down_now, _ok_now, _combo_now;
    char _up_last, _down_last, _ok_last, _combo_last;
};
static Button button(100);


void setup()
{
  lcd.init();
  Serial.begin(115200);
  //task_clock_set(true);
  initDallas();
  initADC();
  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    while (1) delay(10);
  }
    if (rtc.lostPower()) {
    Serial.println(F("RTC lost power, let's set the time!"));
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop()
{
  task_backligth_on_off();
  task_mainmenu();
  task_dallas();
}
class Button;

void task_update_status_on_screen() {
  static long tm = millis();
  if (millis() - tm > 1000) {
    tm = millis();
    float v = getVoltage();
    float a = getCurrent();
    float c = getTemperature();
    lcd_printf(0, F("%c%d.%dV %c%d.%dA"), v<0?'-':' ' ,abs((int)v), abs(((int)(v*10.0))%10), a<0?'-':' ', abs((int)a), abs(((int)(a*10.0))%10));
    lcd_printf(1, F("%d.%dC"), (int)c, ((int)(c*10.0))%10);
  }
}

void task_show_time_date_on_screen() {
  static long tm = millis();
  if (millis() - tm > 1000) {
    tm = millis();
    DateTime now = rtc.now();

    lcd_printf(0, F("%02d:%02d:%02d"), now.hour(), now.minute(), now.second());
    lcd_printf(1, F("%02d.%02d.%d"), now.day(), now.month(), now.year());
  }
}

void task_mainmenu() {

  button.update();

  if (button.button_up()) {
    menulevel++;
    Serial.println("up");
  } else if (button.button_down()) {
    menulevel--;
    Serial.println("down");
  }

  switch (menulevel) {
    case 0:
      task_update_status_on_screen();
      break;
    case 1:
      task_show_time_date_on_screen();
      break;
    case 2:
      lcd_printP(0, F("1. Aloita "));
      lcd_printP(1, F("   tallennus"));
      if (button.button_ok()) {
        task_start_recording(true);
        while (task_start_recording(false)) {
          task_backligth_on_off();
          task_dallas();
          task_led();
          delay(0);
        }
      }
      break;
    case 3:
      lcd_printP(0, F("2. Aseta"));
      lcd_printP(1, F("   mittausvali"));
      if (button.button_ok()) {
        task_interval_set(true);
        while (task_interval_set(false)) {
          delay(0);
        }
      }
      break;
    case 4:
      lcd_printP(0, F("3. Aseta aika"));
      lcd_printP(1, F(""));
      if (button.button_ok()) {
        task_clock_set(true);
        while (task_clock_set(false)) {
          delay(0);
        }
      }
      break;
    case 5:
      lcd_printP(0, F("4. Pihtimittarin"));
      lcd_printP(1, F("   kalibrointi"));
      if (button.button_ok()) {
        calibrateCurrent();
      }
      break;
    default:
      menulevel = 0;
      break;
    case -1:
      menulevel = 5;
      break;
  };

}
