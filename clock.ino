int task_clock_set(bool init) {
  static char second = 0, minute = 0, hour = 0, dayOfWeek = 0, dayOfMonth = 1, month = 1, year = 23;
  lcd.setBacklight(1);
  static int m = 0;

  if (init) {
    m = 0;
    DateTime now = rtc.now();
    second=now.second(); minute=now.minute(); hour=now.hour();dayOfMonth=now.day(); month=now.month(); year=now.year();
    return 1;
  }

  if (m < 3) {
    lcd_printf(0, F("%02d:%02d:%02d"), hour, minute, second);
  } else {
    lcd_printf(0, F("%02d.%02d.20%02d  exit"), dayOfMonth, month, year);
  }

  char *v = NULL;
  char max = 59;
  char min = 0;
  switch (m) {
    case 0: lcd_printP(1, F("^^")); v = &hour; min = 0; max = 23; break;
    case 1: lcd_printP(1, F("   ^^")); v = &minute; break;
    case 2: lcd_printP(1, F("      ^^")); break;
    case 3: lcd_printP(1, F("^^")); v = &dayOfMonth; min = 1; max = 31; break;
    case 4: lcd_printP(1, F("   ^^")); v = &month; min = 1; max = 12; break;
    case 5: lcd_printP(1, F("      ^^^^")); v = &year; min = 23; max = 50; break;
  };

  button.update();
  if (button.button_ok()) {
    m++;
  }

  if (v != NULL) {
    if (button.button_up()) (*v)++;
    if (button.button_down()) (*v)--;
    if (((*v)) > max) (*v) = min;
    else if (((*v)) < min) (*v) = max;
  }
  if (m == 2 && (button.button_up() || button.button_down())) {
    second = 0;
  }

  
  if (m == 6) {
    rtc.adjust(DateTime(year+2000, month, dayOfMonth, hour, minute, second));
    return 0;
  }
  return 1;
}
