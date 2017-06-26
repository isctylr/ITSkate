/***************************************************************
*                         SkateRemote.ino
* 
* This program is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.If not, see <http://www.gnu.org/licenses/>.
*****************************************************************
*
* @author: Isaac Taylor
*
****************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_newWire.h> //Uses a new TwoWire interface on pins 4,3
#include <Adafruit_FRAM_I2C.h>
#include "VescUart.h"
#include "datatypes.h"

// #define DEBUG

// #define OLED

//// BEGIN oled config
#ifdef OLED

#define RESET_OLED 12
Adafruit_SSD1306_newWire oled(RESET_OLED);

#endif
//// END oled config

// BEGIN fram config
Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
uint16_t framAddr = 0;
// END fram config


// BEGIN ble config
#define B_RX 0
#define B_TX 1
#define BLE_BAUD 115200

#define ble Serial1
//SoftwareSerial ble(B_RX, B_TX); // RX, TX  
// No SoftwareSerial since feather has a hardware Serial available!
// END ble config

// BEGIN joystick config
// The joystick has "360 degrees" plus a button when pushed down
#define P_UD A7 
#define P_LR A1
#define P_B 16

// Only up and down need to be very accurate, l/r is just for menu nav

// END joystick config

// BEGIN lipo config
#define VBATPIN A0
#define CHARGING 13
float tx_bat;
bool charging = false;
//END lipo config

// BEGIN vesc communication
#define DIA_WHEEL  76 //mm
#define RATIO_GEAR  2 //TODO: Maybe configure these in remote?

#define C_BTN 5
#define Z_BTN 6

remotePackage remPack;
struct bldcMeasure VescMeasuredValues;

float speed = 0;
float distanceTravel = 0;

int fcount = 0;

const float ratioRpmSpeed = ((DIA_WHEEL * 3.14156) / RATIO_GEAR) * 60 / 1000000; //RPM to Km/h
const float  rationRotDist = ((DIA_WHEEL * 3.14156) / RATIO_GEAR) / 1000000; //RPM to travelled km
// END vesc communication

// BEGIN lights
#define R_LED 10
#define G_LED 11
bool rled = false, gled = false;
// END lights

// BEGIN MODES
typedef enum
{
  LOAD = 0,
  RUN = 1,
  MENU = 2,
  
  ERR = 3,
  BLE_PROG = 4 // Enter by ??, exit by sending '|~'
} MODES;
 
MODES mode = RUN;
MODES prevMode;

String errcode = "";
//END MODES

// BEGIN variables
short mScreen, p_mScreen, mPoint, p_mPoint;
short debouncing = 0;
int flasht = 0;

bool expert = false; // fram: 0x1
bool metric = false; //       0x2
short up = 0, down = 0; //    0x3, 0x4
int center = 512; //          0x5
int vescCells = 10; //        0x6

bool gfram; //                 Not stored in mem. Triggers if fram fails.
// END variables

void setup() {

  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, HIGH);
  gled=rled=true;

  pinMode(CHARGING, INPUT);

#ifdef DEBUG
  while (!Serial);
#endif
  Serial.begin(9600);

#ifdef OLED
  oled.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  draw_init();
#endif



  ble.begin(BLE_BAUD);

  pinMode(P_B, INPUT);
  pinMode(C_BTN, INPUT);
  pinMode(Z_BTN, INPUT);

  if (fram.begin()) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
#ifdef DEBUG    
    Serial.println("Found I2C FRAM");
#endif
    gfram = true;
  } else {
#ifdef DEBUG
    Serial.println("No I2C FRAM found ... check your connections\r\n");
#endif
    gfram = false;
    mode = ERR;
    errcode = "FRam connection failed";
  }
  
  if (gfram) {
    if(fram.read8(0x0) != 0x1) {
#ifdef DEBUG
      Serial.println("First time setup. Writing default values to fram");
#endif
      fram.write8(0x0, 0x1);
      fram.write8(0x1, expert); 
      fram.write8(0x2, metric);
      fram.write8(0x3, up);
      fram.write8(0x4, down);
      fram.write8(0x5, center);
      fram.write8(0x6, vescCells);
    } else {
      expert = fram.read8(0x1);
      metric = fram.read8(0x2);
      up = fram.read8(0x3);
      down = fram.read8(0x4);
      center = fram.read8(0x5);
      vescCells = fram.read8(0x6);
#ifdef DEBUG
      Serial.println("Reading values from memory... ");
      Serial.print("Expert mode: ");
        Serial.println(expert);
      Serial.print("Metric? ");
        Serial.println(metric);
      Serial.print("Calibration values: ");
        Serial.print(up); Serial.print(" "); Serial.print(center);
        Serial.print(" "); Serial.println(down);
      Serial.print("Vesc battery size: ");
        Serial.println(vescCells);
#endif
    }
  }
  digitalWrite(G_LED, gled=false);
  digitalWrite(R_LED, rled=false);
}

void loop() {

charging = !digitalRead(CHARGING);

if (charging) {
  if (flasht > 10) {
    digitalWrite(G_LED, gled=!gled);
    flasht = 0;
  }
  flasht++;
} else {
//  ana
}
  
  if (mode == ERR) {
    Serial.println(errcode);
    digitalWrite(G_LED, gled=false);
    digitalWrite(R_LED, rled=!rled);

#ifdef OLED
    draw_err();
#endif
    delay(1000);

  } else if (mode == BLE_PROG) {

    char c, p;
    if (Serial.available()) {
      p = c;
      c = Serial.read();
      if(p == '|' && c == '~') {
        mode = RUN;
      } else {
        Serial1.print(c);
      }
    }

    if (Serial1.available()) {
      c = Serial1.read();
      Serial.print(c);    
    }

  } else if (mode == RUN) {
//    oled.clearDisplay();
    /*
     * Speed
     * Vesc battery
     * Remote Battery
     * Distance Travelled
     * Beginner/Expert Mode
     */

    // TODO: Calibration values included for P_UD
    remPack.valXJoy = map(analogRead(P_LR), 0, 1023, 255, 0);
    remPack.valYJoy = map(analogRead(P_UD), 0, 1023, 0, 255);
    remPack.valLowerButton = LOW; // C & Z... Not implemented
    remPack.valUpperButton = digitalRead(P_B);
    
    VescUartSetNunchukValues(remPack);
/*
    while(ble.available()) {

        VescMeasuredValues.avgMotorCurrent = ble.parseFloat();
        VescMeasuredValues.avgInputCurrent = ble.parseFloat();
        VescMeasuredValues.dutyCycleNow = ble.parseFloat();
        VescMeasuredValues.rpm = ble.parseInt();
        VescMeasuredValues.inpVoltage = ble.parseFloat();
        VescMeasuredValues.ampHours = ble.parseFloat();
        VescMeasuredValues.ampHoursCharged = ble.parseFloat();
        VescMeasuredValues.tachometer = ble.parseInt();
        VescMeasuredValues.tachometerAbs = ble.parseInt();        
    }
    */

    if (VescUartGetValue(VescMeasuredValues)) {
#ifdef DEBUG
        SerialPrint(VescMeasuredValues);
#endif
        if (!charging) digitalWrite(G_LED, gled=true);
        digitalWrite(R_LED, rled=false); // TODO: battery?
        fcount = 0;
    }
    else
    {
#ifdef DEBUG
      Serial.println("Failed to get data from UART!");
#endif // DEBUG
      if (fcount >= 10) {
        digitalWrite(R_LED, rled=true);
        if (!charging) digitalWrite(G_LED, gled=false);
      }
      fcount++;
    }

    tx_bat = analogRead(VBATPIN);
    //tx_bat *= 3.3;
    //tx_bat /= 128; //TODO: this gives voltage but feels lossy for percentage
    //tx_bat = map(tx_bat, 3.2, 4.2, 0, 100); // Let's get percent, yeah?
    tx_bat = map(tx_bat, 0, 1023, 0, 100);
    speed = VescMeasuredValues.rpm * ratioRpmSpeed;
    distanceTravel = VescMeasuredValues.tachometer * rationRotDist;

#ifdef DEBUG
    Serial.print("Battery percent: ");
    Serial.print(tx_bat); Serial.print(" Joystick: ");
    Serial.print(remPack.valXJoy); Serial.print(" ");
    Serial.println(remPack.valYJoy);
    Serial.print("C: "); Serial.println(remPack.valUpperButton);
    Serial.print("Z: "); Serial.println(remPack.valLowerButton);
    delay(50); //TODO
#endif

    delay(10);
  } else if (mode == LOAD && prevMode != mode) {
    prevMode = mode;
#ifdef OLED
    draw_load();
#endif
  } else if (mode == MENU) {
    // First time in the menu, 
    if (prevMode != MENU) {
      if (expert) {
        mScreen = 10;
        mPoint = 0;
      } else {
        mScreen = mPoint = 0;
      }
      p_mScreen = p_mPoint = 100; // Why not? Just not 0.
      prevMode = MENU;
    }
    if ((mScreen != p_mScreen) || (mPoint != p_mPoint)) {
      p_mScreen = mScreen;
      p_mPoint = mPoint;
#ifdef OLED
      draw_menu(mScreen, mPoint);
#endif      
    }
    
    if (debouncing > 90) {
      if (analogRead(P_LR) >= 950) {
        if (mPoint < 4) 
          mPoint++;
        debouncing = 0;
      } else if (analogRead(P_LR) <= 100) {
        if (mPoint > 0)
          mPoint--;
        debouncing = 0;
      } else if (digitalRead(P_B) == HIGH) {
        mScreen = mScreen + mPoint + 10;
        // Beginner/expert mode...
        if (mScreen == 10) expert = true;
        if (mScreen == 20) {
          mScreen = 0; 
          expert = false;
        }
        debouncing = 0;
      }
    }
  }
  debouncing++;
  if(debouncing >= 250) debouncing = 91;
}
