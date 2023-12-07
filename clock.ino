int task_clock_set(bool init) {
  static char second = 0, minute = 0, hour = 0, dayOfWeek = 0, dayOfMonth = 1, month = 1, year = 22;
  lcd.setBacklight(1);
  static int m = 0;

  if (init) {
    m = 0;
    /*  time_t tim;   // not a primitive datatype
        time(&tim);
        struct tm * tm = localtime(&tim);

        second=tm->tm_sec; minute=tm->tm_min; hour=tm->tm_hour;dayOfMonth=tm->tm_mday; month=tm->tm_mon+1; year=tm->tm_year;*/
    return 1;
  }

  char buf[17];
  if (m < 3) {
    sprintf(buf, "%02d:%02d:%02d", hour, minute, second);
  } else {
    sprintf(buf, "%02d.%02d.20%02d  exit", dayOfMonth, month, year);
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

  lcd_print(0, buf);
  if (m == 6) {
    return 0;
  }
  return 1;
}
