#include <stdarg.h>


unsigned char checksum(const char* values) {
  uint8_t checksum = 0;
  int len = strlen(values);
  for (int i = 0; i < len; i++) checksum ^= values[i];
  return checksum;
}

void lcd_printP(int line, const __FlashStringHelper* t) {
  static char buffer[17];
  strncpy_P(buffer, (const char*)t, 16);
  lcd_print(line, buffer);
}

void lcd_print(int line, const char*t) {
  unsigned s[2];
  unsigned s2 = checksum(t);

  if (s[line] == s2) return;
  s[line] = s2;

  lcd.setCursor(strlen(t), line);
  for (int i = 0; i < 16 - strlen(t); i++) {
    lcd.print(" ");
  }
  lcd.setCursor(0, line);
  lcd.print( t);
}

void lcd_on_off(bool on) {
  if (on) lcd.setBacklight(1);
  else lcd.setBacklight(0);
}
void task_backligth_on_off() {
  static long tm = millis();

  static Button button(0);
  if (button.button_any()) {
    lcd.setBacklight(1);
    tm = millis();
  }
  if (millis() - tm > 10000) {
    tm = millis();
    lcd.setBacklight(0);
    menulevel = 0;
  }
}

void lcd_printf(int line, const char* str, ...) {
  va_list args;
  va_start(args, str);
  static char buffer[17];
  // char array to store token
  vsnprintf(buffer, sizeof(buffer), str, args); // Use nanoprintf for formatting.

  lcd_print(line, buffer);

  va_end(args);
}
