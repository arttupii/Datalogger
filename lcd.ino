#include <stdarg.h>

static char lcdBuf[17];

uint16_t ChecksumFletcher16(byte *data, size_t count )
{//https://www.luisllamas.es/en/arduino-checksum/
   uint8_t sum1 = 0;
   uint8_t sum2 = 0;

   for(size_t index = 0; index < count; ++index )
   {
      sum1 = sum1 + (uint8_t)data[index];
      sum2 = sum2 + sum1;
   }
   return (sum2 << 8) | sum1;
}

void lcd_printP(int line, const __FlashStringHelper* t) {
  strncpy_P(lcdBuf, (const char*)t, 16);
  lcd_print(line, lcdBuf);
}

void lcd_print(int line, const char*t) {
  static uint16_t s[2];
  uint16_t s2 = ChecksumFletcher16(t, strlen(t));

  if (s[line] == s2) return;
  s[line] = s2;

  lcd.setCursor(strlen(t), line);
  for (int i = 0; i < 16 - strlen(t); i++) {
    lcd.print(" ");
  }
  lcd.setCursor(0, line);
  lcd.print( t);
  Serial.println(t);
}

void lcd_on_off(bool on) {
  if (on) lcd.setBacklight(1);
  else lcd.setBacklight(0);
}
void task_backligth_on_off() {
  static long tm = 0;
  if(tm==0) {
    tm = millis();
    lcd.setBacklight(1);
  }

  static Button button(0);
  if (button.button_any()) {
    lcd.setBacklight(1);
    tm = millis();
  }
  if (millis() - tm > 60000) {
    tm = millis();
    lcd.setBacklight(0);
    menulevel = 0;
  }
}

const char * floatToStr(float a){
  dtostrf(a, 4, 2, lcdBuf);
  char* p  = strchr(lcdBuf, '.');
  *p = ',';
  return lcdBuf;
}

void lcd_printf(int line, const __FlashStringHelper* str, ...) {
  va_list args;
  va_start(args, str);

  vsnprintf_P(lcdBuf, sizeof(lcdBuf), (const char *)str, args); 

  lcd_print(line, lcdBuf);

  va_end(args);
}
