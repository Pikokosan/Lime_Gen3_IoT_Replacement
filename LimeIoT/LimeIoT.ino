#include "CRC16.h"
#include "CRC.h"
//#include <NimBLEDevice.h>
#include <ArduinoBleOTA.h>
#include <BleOtaMultiservice.h>
#include <BleOtaSecurityOnConnect.h>

//need to make these changeable and saveable
#define SCOOTER_NAME "lme-UJEYGJA"
const uint32_t BLE_PASSWORD = 123456789;


//version
#define HW_NAME "Lime iot"
#define HW_VER{0,0,1} //version 0.0.1
#define SW_NAME "Crusty starfish" //named for fun
#define SW_VER{0,0,2} //version 0.0.2


// Set pins
#define RXD2 16
#define TXD2 17
const int LOCK_PIN = 12;
#define BUZZZER_PIN 13

BleOtaSecurityOnConnect security;

NimBLEServer *pServer = NULL;
NimBLECharacteristic *pMainCharacteristic;
NimBLECharacteristic *pSettingsCharacteristic;
NimBLECharacteristic *pDebugCharacteristic;


// Display Status Codes
const String DISPLAY_STATUS_SCAN = "21";
const String DISPLAY_STATUS_ERROR = "22";
const String DISPLAY_STATUS_PAUSED = "23";
const String DISPLAY_STATUS_LOCKED = "24";
const String DISPLAY_STATUS_DONE = "25";
const String DISPLAY_STATUS_CHARGING = "26";
const String DISPLAY_STATUS_DRIVING = "31";
const String DISPLAY_STATUS_DRIVING_LOW_BATTERY = "41";
const String DISPLAY_STATUS_DRIVING_ALERT = "42";
const String DISPLAY_STATUS_DRIVING_NO_PARKING = "43";
const String DISPLAY_STATUS_DRIVING_NO_RIDING = "44";
const String DISPLAY_STATUS_DRIVING_MAX_SPEED = "45";
const String DISPLAY_STATUS_UPGRADING = "51";


// Controller Codes
byte hearthBeatEscByte[16] = { 0x46, 0x43, 0x11, 0x01, 0x00, 0x08, 0x4C, 0x49, 0x4D, 0x45, 0x42, 0x49, 0x4B, 0x45, 0xBE, 0x8A };
byte onEscByte[9] = { 0x46, 0x43, 0x16, 0x61, 0x00, 0x01, 0xF1, 0xF2, 0x8F };
byte offEscByte[9] = { 0x46, 0x43, 0x16, 0x61, 0x00, 0x01, 0xF0, 0xE2, 0xAE };
byte lightOnEscByte[9] = { 0x46, 0x43, 0x16, 0x12, 0x00, 0x01, 0xF1, 0x2B, 0x26 };
byte lightOffEscByte[9] = { 0x46, 0x43, 0x16, 0x12, 0x00, 0x01, 0xF0, 0x3B, 0x07 };

// Status
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t isUnlocked = 0;
uint8_t controllerIsOn = 0;
uint8_t lightIsOn = 0;
uint8_t unlockForEver = 0;
float speed = 0;
uint8_t alarmIsOn = 0;
uint8_t throttle = 1;
byte battery = 0x00;
byte isCharging = 0x00;
String customDisplayStatus = "";

// Set Settings
int alarm_delay = 200;
int alarm_freq = 3000;
int alarm_reps = 15;
int max_speed = 28;

RTC_DATA_ATTR int bootCount = 0;

// BLE
#define SERVICE_UUID "653bb0e0-1d85-46b0-9742-3b408f4cb83f"
#define CHARACTERISTIC_UUID_MAIN "00c1acd4-f35b-4b5f-868d-36e5668d0929"
#define CHARACTERISTIC_UUID_SETTINGS "7299b19e-7655-4c98-8cf1-69af4a65e982"
#define CHARACTERISTIC_UUID_DEBUG "83ea7700-6ad7-4918-b1df-61031f95cf62"



// Display Task
TaskHandle_t UARTTask;

class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(NimBLEServer *pServer) {
    deviceConnected = false;
  }
};

//UARTTaskCode: read controller and send command to display every 300ms
void UARTTaskCode(void *pvParameters) {
  for (;;) {
    if (isUnlocked == 1) {
      sendDisplayCommand(speed, battery, customDisplayStatus != "" ? customDisplayStatus : DISPLAY_STATUS_DRIVING);
    } else {
      if (isCharging) {
        sendDisplayCommand(speed, battery, customDisplayStatus != "" ? customDisplayStatus : DISPLAY_STATUS_CHARGING);
      } else if (deviceConnected) {
        sendDisplayCommand(speed, battery, customDisplayStatus != "" ? customDisplayStatus : DISPLAY_STATUS_LOCKED);
      } else {
        sendDisplayCommand(speed, battery, customDisplayStatus != "" ? customDisplayStatus : DISPLAY_STATUS_SCAN);
      }
    }
    delay(300);
  }
}
