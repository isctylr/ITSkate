#line 2

//#include <Arduino.h>

#ifdef OLED

String menuItems[4];

void draw_init() {
  //oled.display();
  //oled.setTextSize(1);
  //oled.setTextColor(WHITE);

}

void draw_load() {
  
}
/*
* MENU 
* - (0) Beginner mode (S0) / Expert Mode (S10)
* - (1) Calibrate Throttle -> (S11)
* - (2) Unit types -> (S12) : km/h vs. mph
* - (3) Exit and Save
*/

void draw_menu(int screen, int point) {
  switch(screen) {
    case 0:
    {
      menuItems[0] = "Beginner Mode";
      menuItems[1] = "Calibrate Throttle";
      menuItems[2] = "Unit Types";
      menuItems[3] = "Exit and Save";
      break;
    }
    case 10:
    {
      menuItems[0] = "Expert Mode";
      menuItems[1] = "Calibrate Throttle";
      menuItems[2] = "Unit Types";
      menuItems[3] = "Exit and Save";
      break;
    }
    case 11:
      // Throttle Calibration
      break;
    case 12:
      // Units options
      if (metric) menuItems[0] = "km/hr";
      else menuItems[0] = "mi/hr";
      menuItems[1] = "Back";
      menuItems[2] = menuItems[3] = "";
      break;
  }
  for (int i=0; i<4; i++) {
    oled.setCursor(10, 10+ i*15);
    if (i == mPoint) oled.print(">");
    oled.setCursor(15, 10+ i*15);
    oled.print(menuItems[i]);
  }
}

void draw_err() {
  
}

#endif
