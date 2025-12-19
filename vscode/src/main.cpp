#include <M5Unified.h>
#include "WiFi.h"
#include <Preferences.h>

int Num_networks = 0;
String AppSSID;
//String AppPassword;

void DisplayFunction(const char *buttonA, const char *buttonB, const char *buttonC);
bool SelectWiFi();
bool CheckWiFi();
void GetPreferences();
void PutPreferences();

extern int CheckWiFi_time;
enum States {Select = 0, Check} state;

void setup() {
  M5.begin();
  M5.Power.begin();

  M5.Lcd.setBrightness(200);
  M5.setLogDisplayIndex(0);
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_INFO);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);
  M5_LOGI("Initializing ...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  M5_LOGI("WIFI SCAN");
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  delay(500);

  GetPreferences();
  if (AppSSID.isEmpty()) {
    state = Select;
    Num_networks = WiFi.scanNetworks();
  } else {
    state = Check;
    CheckWiFi_time = 0;
  }
}

void loop() {
  M5.update();
  switch (state) {
  case Select:
    if (SelectWiFi()) {
      PutPreferences();
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(2);
      M5.Lcd.print("Checking ...");
      CheckWiFi_time = 0;
      delay(500);
      state = Check;
    }
    break;
  case Check:
    if (CheckWiFi()) {
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(2);
      M5.Lcd.print("Scanning ...");
      delay(500);
      state = Select;
    }
    break;
  }
  delay(100);
}

void DisplayFunction(const char *buttonA, const char *buttonB, const char *buttonC)
{
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.fillRect(30, 220, 70, 16, WHITE);
  M5.Lcd.setCursor(30, 220);
  M5.Lcd.printf(buttonA);
  M5.Lcd.fillRect(135, 220, 70, 16, WHITE);
  M5.Lcd.setCursor(135, 220);
  M5.Lcd.printf(buttonB);
  M5.Lcd.fillRect(230, 220, 70, 16, WHITE);
  M5.Lcd.setCursor(230, 220);
  M5.Lcd.println(buttonC);
}

const int SelectWiFi_rows = 10;
const int SelectWiFi_x = 10;
const int SelectWiFi_y = 32;
const int SelectWiFi_w = 300;
const int SelectWiFi_h = 16;
bool SelectWiFi_redraw = true;
int SelectWiFi_top = 0;
int SelectWiFi_cur = 0;

bool SelectWiFi()
{
  if (SelectWiFi_redraw) {
    M5.Lcd.clear();
    DisplayFunction(" Prev ", "  OK  ", " Next ");
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Select WiFi");
    for (int i = 0; i < SelectWiFi_rows; i ++) {
      int index = i + SelectWiFi_top;
      if (index >= Num_networks) {
        break;
      }
      M5.Lcd.setCursor(SelectWiFi_x, i * SelectWiFi_h + SelectWiFi_y);
      bool cur = index == SelectWiFi_cur;
      if (cur) {
        M5.Lcd.setTextColor(BLACK);
        M5.Lcd.fillRect(SelectWiFi_x, i * SelectWiFi_h + SelectWiFi_y, SelectWiFi_w, SelectWiFi_h, WHITE);
      }
      M5.Lcd.printf("%d:", index + 1);
      M5.Lcd.println(WiFi.SSID(index));
      if (cur) {
        M5.Lcd.setTextColor(WHITE);
      }
    }
    SelectWiFi_redraw = false;
  }
  if (M5.BtnA.wasPressed()) {
    if (SelectWiFi_cur > 0) {
      SelectWiFi_cur --;
      SelectWiFi_redraw = true;
      if (SelectWiFi_cur < SelectWiFi_top) {
        SelectWiFi_top -= SelectWiFi_rows / 2;
        if (SelectWiFi_top < 0) {
          SelectWiFi_top = 0;
        }
      }
    }
  }
  if (M5.BtnC.wasPressed()) {
    if (SelectWiFi_cur < Num_networks - 1) {
      SelectWiFi_cur ++;
      SelectWiFi_redraw = true;
      if (SelectWiFi_cur >= SelectWiFi_top + SelectWiFi_rows) {
        SelectWiFi_top += SelectWiFi_rows / 2;
      }
    }
  }
  if (M5.BtnB.wasPressed()) {
    AppSSID = WiFi.SSID(SelectWiFi_cur);
    return true;
  }
  return false;
}

const int CheckWiFi_x = 16;
const int CheckWiFi_y = 32;
const int CheckWiFi_timeout = 30000;
bool CheckWiFi_redraw = true;
int CheckWiFi_time = 0;

bool CheckWiFi()
{
  if (millis() > CheckWiFi_time) {
    if (CheckWiFi_time != 0) {
      M5.Lcd.setBrightness(0);
      M5.Power.deepSleep(CheckWiFi_timeout*1000);
    }
    CheckWiFi_time = millis() + CheckWiFi_timeout;
    Num_networks = WiFi.scanNetworks();
    CheckWiFi_redraw = true;
  }
  if (CheckWiFi_redraw) {
    M5.Lcd.clear();
    DisplayFunction("", "", " Back ");
    M5.Lcd.setCursor(CheckWiFi_x, CheckWiFi_y);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("Checking WiFi: ");
    M5.Lcd.setCursor(CheckWiFi_x, CheckWiFi_y + 20);
    M5.Lcd.print(AppSSID);

    bool found = false;
    for (int i = 0; i < Num_networks; i ++) {
      if (WiFi.SSID(i) == AppSSID) {
        found = true;
        break;
      }
    }
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(CheckWiFi_x + 120, CheckWiFi_y + 80);
    if (found) {
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.print("OK");
    } else {
      M5.Lcd.setTextColor(RED);
      M5.Lcd.print("NG");
    }
    CheckWiFi_redraw = false;
  }
  return M5.BtnC.wasPressed();
}

const char PreferenceName[] = "m5stack";

void GetPreferences()
{
  Preferences preferences;
  preferences.begin(PreferenceName);
  AppSSID = preferences.getString("ID");
//  AppPassword = preferences.getString("password");
  preferences.end();
}

void PutPreferences()
{
  Preferences preferences;
  preferences.begin(PreferenceName);
  preferences.putString("ID", AppSSID);
//  preferences.putString("password", AppPassword);
  preferences.end();
}
