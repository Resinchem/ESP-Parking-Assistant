/*
 * ESP32 Parking Assistant
 * Includes captive portal and OTA Updates
 * This provides code for an ESP32 controller and a WS2812b LED strip
 * Last Updated: September, 2026
 * ResinChem Tech - Released under GNU General Public License v3.0.  There is no guarantee or warranty, either expressed or implied, as to the
 * suitability or utilization of this project, or as to the condition of this project, or whether it will be suitable to the users purposes or needs.
 * Use is solely at the end user's risk.
 */
#pragma once

#include <Wire.h>
#include <LittleFS.h>
#include <TFMPlus.h>                    //https://github.com/budryerson/TFMini-Plus (v1.5.0)
#include <WiFiClient.h>                 //Arduino ESP Core - creates a client that can connect to an IP address
#include <PubSubClient.h>               //https://github.com/knolleary/pubsubclient  Provides MQTT functions (v2.8)
#include <ArduinoOTA.h>                 //https://github.com/jandrassy/ArduinoOTA (v1.1.0)
#include <ArduinoJson.h>                //https://github.com/bblanchon/ArduinoJson (v7.3.1)
#define FASTLED_INTERNAL                //Suppress FastLED SPI/bitbanged compiler warnings (only applies after first compile)
#include <FastLED.h>                    //https://github.com/FastLED/FastLED - LED functionality (v3.7.1)
#include <VL53L0X.h>                    //VL53L0X ToF Sensor https://github.com/pololu/vl53l0x-arduino (v1.3.1) - only available on ESP32
#include <math.h>
#include "html.h"                       //html code for the web pages
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <DNSServer.h>                  //Captive portal DNS server (for auto-launching: only works on some devices/operating systems)
#include <Update.h>

#define VERSION "v0.61"
// =======================
//  GPIO PINS
// =======================
// These are just defaults.  If using different GPIO pins, you can set these via the embedded web app.
// It is not necessary to change any pins here.
#define LED_DATA_PIN 19                 // Pin connected to LED strip Data IN
#define TFMINI_RX_PIN 16                // To TF-Mini TX
#define TFMINI_TX_PIN 17                // To TF-Mini RX
#define TOF_DAT_PIN 21                  // To VL53L0X SDA PIN (data)
#define TOF_CLK_PIN 22                  // To VL53L0X SCL PIN (clock)
#define ONBOARD_LED 2                   // ESP32's onboard (blue) LED GPIO Pin
// ================================
//  System Feature Options
// ================================
#define APPNAME "Parking Assistant"
// Changing any of these requires a recompile and upload.  Normally changed for debugging purposes.
#define WIFIMODE 2                      // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both
#define MQTTMODE 1                      // 0 = Disable MQTT, 1 = Enable (will only be enabled if WiFi mode = 1 or 2 - broker must be on same network)
#define SERIAL_DEBUG 0                  // 0 = Disable (must be disabled if using ESP8266 RX/TX pins), 1 = enable
#define DNS_PORT 53                     // Standard DNS port of auto-launching captive portal or web app when running in AP Mode
#define NUM_LEDS_MAX 600                // For initialization - recommend actual max 50 LEDs if built as shown
#define MILLI_AMPS 5000;                // Default - will be defined during onboarding
#define FORMAT_LITTLEFS_IF_FAILED true  // DO NOT CHANGE!!!

//MQTT Handling - needed to handle in main loop and to avoid thread conflicts
#define MSK_LEDSTATE        0x00000001UL
#define MSK_LEDCOLOR        0x00000002UL
#define MSK_LEDBRIGHTNESS   0x00000004UL
#define MSK_CAR_PRESENCE    0x00000008UL
#define MSK_SIDE_DISTANCE   0x00000010UL
#define MSK_PARK_DISTANCE   0x00000020UL
#define MSK_ACTIVE_ZONE     0x00000040UL
#define MSK_SLEEP_BRIGHT    0x00000080UL
#define MSK_COLOR_STANDBY   0x00000100UL
#define MSK_COLOR_WAKE      0x00000200UL
#define MSK_COLOR_ACTIVE    0x00000400UL
#define MSK_COLOR_PARKED    0x00000800UL
#define MSK_COLOR_BACKUP    0x00001000UL
#define MSK_COLOR_MANUAL    0x00002000UL
#define MSK_DIST_WAKE       0x00004000UL
#define MSK_DIST_START      0x00008000UL
#define MSK_DIST_PARK       0x00010000UL
#define MSK_DIST_BACKUP     0x00020000UL
#define MSK_SIDE_DIST_LEFT  0x00040000UL
#define MSK_SIDE_DIST_RIGHT 0x00080000UL
#define MSK_SENSOR_OVERRIDE 0x00100000UL
#define MSK_LED_EFFECT      0x00200000UL
#define MSK_LIGHT_JSON      0x00400000UL
// ========================
//  Config variables
// ========================
// These variables are stored in the saved configuration.  
// Most have a 'default' and 'active' version and can be changed via web app without updating saved defaults
// Those with only a single version (non _dflt) can only be updated with a Save & Reboot
String deviceName = "parkasst";    //This value will be set during onboarding and cannot be changed via web app
// --------------------
//  GPIO Pin Variables
// --------------------
//These are initially assigned defaults, but will be updated by config file when read
//Single Vars
byte ledData_Pin = LED_DATA_PIN;
byte tfminiRX_Pin = TFMINI_RX_PIN;
byte tfminiTX_Pin = TFMINI_TX_PIN;
byte tofDat_Pin = TOF_DAT_PIN;
byte tofClk_Pin = TOF_CLK_PIN;
byte onboardLED_Pin = ONBOARD_LED;

// --------------------------
//  LEDs, Effects and Colors
// --------------------------
//Single vars
int milliamps = MILLI_AMPS;   //does not have 'active'
CRGB ledColorOff = CRGB::Black;
bool rightLEDWiring = false;
int numLEDs = 30;

//Vars with Saved defaults 
byte defaultBrightness = 100;
byte sleepBrightness_dflt = 5;

CRGB ledColorStandby_dflt = CRGB::Blue;
CRGB ledColorWake_dflt = CRGB::Lime;
CRGB ledColorActive_dflt = CRGB::Yellow;
CRGB ledColorParked_dflt = CRGB::Red;
CRGB ledColorBackup_dflt = CRGB::Red;

String ledEffect_dflt = "Out-In";

//Active vars
byte activeBrightness = 100;
byte lastBrightness = 64;                // Holding varible to return to prior state when LEDs have been off.
byte sleepBrightness = 5;

CRGB ledColorStandby = CRGB::Blue;
CRGB ledColorWake = CRGB::Green;
CRGB ledColorActive = CRGB::Yellow;
CRGB ledColorParked = CRGB::Red;
CRGB ledColorBackup = CRGB::Red;

String ledEffect_m1 = "Out-In";

// -----------------------
//  Sensors and Distances
// -----------------------
//Single vars
byte uomDistance = 1;              // 0=inches, 1=millimeters (no active var)
bool useSideSensor = false;
byte sideSensorPos = 0;            // 0=unused, 1=right side, 2=left side
byte nocarDetectedCounterMax = 10; //noCarDebounce (valid values 0 - 25) (no active var)
uint32_t maxOperationTimePark = 60;
uint32_t maxOperationTimeExit = 5;
//Vars with saved defaults
int wakeDistance_dflt = 3048;
int startDistance_dflt = 1829;   // Start countdown distance (~6')
int parkDistance_dflt = 610;     // Final parked distacce (~2')
int backupDistance_dflt = 457;   // Flash backup distance (~18")

int leftDistance_dflt = 610;     // 610 = ~24 in
int rightDistance_dflt = 508;    // 508 = ~20 in

//Active Vars
int wakeDistance = 3048;    // wake/sleep distance (~10ft)
int startDistance = 1829;   // Start countdown distance (~6')
int parkDistance = 610;     // Final parked distacce (~2')
int backupDistance = 457;   // Flash backup distance (~18")

int leftDistance = 610;     // 610 = ~24 in
int rightDistance = 508;    // 508 = ~20 in

//-------------------------
// MQTT - Only single vars (no dflt/active)
//-------------------------
//  MQTT will only be used if a server address other than '0.0.0.0' is entered via portal
byte mqttAddr_1 = 0;
byte mqttAddr_2 = 0;
byte mqttAddr_3 = 0;
byte mqttAddr_4 = 0;
int mqttPort = 0;

String mqttUser = "myusername";
String mqttPW = "mypassword";
uint16_t mqttTelePeriod = 60;
String mqttTopicSub ="parkasst";  
String mqttTopicPub = "parkasst"; 

//======== END CONFIG VARS ==========

// ===============================
//  Effects and Color arrays 
// ===============================
//  Effects are defined in defineEffects() - called in Setup
//  Effects must be handled in the lights on call
//  To add an effect:
//    - Increase array below (if adding)
//    - Add element and add name in defineEffects()
//    - Update if statement in main loop
//    - Add update function to implement effect (called by main loop)
int numberOfEffects = 5;
String Effects[5]; 

//===============================
// WiFi and Onboarding Variables
//===============================
//WiFi Info
String wifiHostName = deviceName;
String otaHostName = deviceName + "_OTA";
String wifiSSID = "";
String wifiPW = "";

bool onboarding = false;        //Will be set to true if no config file found or wifi cannot be joined
bool onboardConnecting = false;
bool triggerWiFiBegin = false;
unsigned long onboardStartMilli = 0;
unsigned long onboardSuccessMilli = 0;
bool onboardRebootReady = false;
unsigned long onboardRebootMilli = 0;
bool wifiConnected = false;
//Vars for using AP Mode Only
bool noWiFiMode = false;
String manualAPName = deviceName + "_Hotspot";
String manualAPPwd = "";

//=================
//  MQTT Variables
//=================

String mqttClient = "parkasst";
uint32_t mqttLastUpdate = 0;
bool mqttEnabled = false;              //Will be enabled/disabled depending on whether a valid IP address is defined in Settings (0.0.0.0 disables MQTT)
bool mqttConnected = false;            //Will be enabled if defined and successful connnection made.  This var should be checked upon any MQTT action.
bool prevCarStatus = false;            //For forcing MQTT update on state change
bool forceMQTTUpdate = false;          //Force MQTT update on state change
bool initialSyncRequired = false;      //Flag for initial publish of MQTT data after boot
const int MQTT_RETRY_DELAY = 5000;     //Delay (ms) between reconnect attempts
uint32_t lastMqttRetryAttempt = 0;     //Non-blocking handler for MQTT reconnection
int syncStep = 0;
unsigned long lastSyncStepTime = 0;
volatile uint32_t mqttUpdateMask = 0;  //Volatile so it can be updated by out-of-thread background tasks
const int INIT_SYNC_INTERVAL = 100;   
bool isMqttJsonProcessing = false;     //Needed to handle special light JSON payload from Home Assistant (Discovery)

//Vars for Discovery
byte macAddr[6];               //Device MAC address (array is in reverse order)
String strMacAddr;             //MAC address as string and in proper order
char uidPrefix[] = "prkast";   //Prefix for unique ID generation
char devUniqueID[30];          //Generated Unique ID for this device (uidPrefix + last 6 MAC characters)

struct DiscoverySettings {     //Structure for HA MQTT Discovery 
  bool exists = false;
  String deviceName = "";
  bool controls = true;
  bool sensors = true;
  bool config = true;
  bool diagnostics = true;
};
DiscoverySettings currentDiscovery;

// ========================
// Arduino IDE OTA Updates
// ========================
// You should not change any of these
bool ota_flag = true;                       // Must leave this as true for board to broadcast port to IDE upon boot
uint16_t ota_boot_time_window = 2500;       // minimum time on boot for IP address to show in IDE ports, in millisecs
uint16_t ota_time_window = 20000;           // time to start file upload when ota_flag set to true (after initial boot), in millsecs
uint16_t ota_time_elapsed = 0;              // Counter when OTA active
uint16_t ota_time = ota_boot_time_window;
uint8_t web_otaDone = 0;                    // Web OTA Firmware Update

//=============================
// Web app tesing and hold vars
//=============================
String whichData = "all";
//Main Page
String mainPageMode = "";
bool mainTestColorFlag = false;
bool mainTestDistFlag = false;
String mainPageColorMsg = "";
String mainPageDistMsg = "";


// ========================
// OTHER GLOBAL VARIABLES
// ========================
bool isBooting = false;
bool overrideSensors = false;   //Used by MQTT/API to override sensors for manual LED control
bool mainTestFlag = false;
bool tfMiniEnabled = false;
bool blinkOn = false;
bool blinkSideOn = false;
int intervalDistance = 0;
bool carDetected = false;
bool isAwake = false;
bool coldStart = true;
bool showStandbyLEDs = true;
bool useOnboardLED = true;
bool useBootLightsLED = true;

bool ledState = false;
CRGB ledColorManual = CRGB::White;
CRGB activeLEDColor = CRGB::White;

//Sensor measurements
int16_t tf_dist = 0;
int16_t vl_side_dist = 0;
String mqttZone = "Vacant";

byte carDetectedCounter = 0;
byte carDetectedCounterMax = 3;
byte nocarDetectedCounter = 0;
byte outOfRangeCounter = 0;
uint32_t startTime;
bool exitSleepTimerStarted = false;
bool parkSleepTimerStarted = false;

String baseIP;

//===========================
// Instantiate objects
//===========================
WiFiClient espClient;
WebServer server(80);
TwoWire bus1 = TwoWire(0);
VL53L0X side_sensor;
DNSServer dnsServer;

#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  PubSubClient client(espClient);
#endif

TFMPlus tfmini;
CRGB LEDs[NUM_LEDS_MAX];  

// ==============================
//  Function Prototypes/Declares
// ==============================
//Config Files
void readConfigFile();
void writeConfigFile(bool restart_ESP);
String getCurrentConfig();
bool saveDiscoveryConfig();
void deleteDiscoveryConfig();
String getDiscoveryConfig();
//WiFi and MQTT Setup
void setupSoftAP();
bool setupWifi();
bool setupManualAP();
bool setup_mqtt();
void reconnect();
bool reconnect_soft();
bool mqttIsConnected();
//LED and Display Functions
void defineEffects();
void allLEDsOff(bool resumeSleep = false);
void blinkLEDs(CRGB color);
void blinkSideLEDs(CRGB color, byte pos);
void updateOutIn(int curDistance);
void updateInOut(int curDistance);
void updateFullStrip(int curDistance);
void updateFullStripInv(int curDistance);
void updateSolid(int curDistance);
void updateSleepMode();
void updateOTA();
//Calcs, Conversions and Misc
int calculateInterval();
float mmToInches(int mmDistance);
int inchesToMm(float inchDistance);
//MQTT and API Processors
void callback(char* topic, byte* payload, unsigned int length);
void handleAPI();
bool processCommand(String key, String val);
void updateMQTT(String whichField);
void handleQueuedMqttUpdates();
void executeMqttUpdate(String whichField);
void handleInitMqttSync();
void handleLiveMqttUpdates();
void publishLightJsonState();
void forceMqttRefresh();
void setLEDState(bool state, CRGB color = activeLEDColor);
void overrideAllSensors(bool sensorsOff);
String getEffectName(String effect);
//Web Pages and Handlers
void setupWebHandlers();
void handleRoot();
void handleOnboard();
void handleOnboardStatus();
void runOnboardingProcesses();
void handleMainPage();
void webMainPageJson();
void webSystemPage();
void webSystemPageJson();
void handleSystem();
void webCalibratePage();
void getCalibrationData();
void webIntegratePage();
void webIntegratePageJson();
void handleIntegrations();
void handleDiscoverySave();
void publishDiscovery(bool add);
void publishVersionUpdate();
void webRestartPage();
void webFirmwareUpdate();
void handleWebUpdate();
void handleWebUpdateEnd();
void handleOTAUpdate();
void handleOTAUpdateMqtt();
void webConfigDump();
void handleInfoDump();
void infoDumpJson();
void webResetPage();
void handleNotFound();

//Color Conversions
bool isValidHex(String hex);
bool splitRGBString(String colorVal, byte &r, byte &g, byte &b);
CRGB hexToCRGB(String hexString);
void hexToRgb(String hexString, byte &r, byte &g, byte &b);
void crgbToRgb(CRGB color, byte &r, byte &g, byte &b);
String crgbToHex(CRGB color);
CRGB rgbToCRGB(byte red, byte green, byte blue);
String rgbToHex(byte red, byte green, byte blue);
CRGB stringToCRGB(String color);

// ==================================
// ----------------------------------
//  Main Setup
// ----------------------------------
// ==================================
void setup() {
  isBooting = true;
  // Serial monitor
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.begin(115200);
    Serial.println("Reading Config...");
  #endif
  esp_netif_init();
  setupWebHandlers();
  readConfigFile();

  if (onboarding) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("Entering Onboarding setup...");
    #endif
    setupSoftAP();
  } else {
    // --- Network Mode Selection ---
    bool networkReady = false;
    if (noWiFiMode) {
      networkReady = setupManualAP();
    } else {
      networkReady = setupWifi();
    }
    if (!networkReady) {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.println("Wifi connect failed. Reentering onboarding...");
      #endif
      setupSoftAP();
      onboarding = true;
    } else {
      //*** NORMAL BOOT PROCESS HERE ***
      //If enabled, turn on onboard LED - indicates successful WiFi Connection
      if ((useOnboardLED) && (!noWiFiMode)) {
        pinMode(onboardLED_Pin, OUTPUT);
        digitalWrite(onboardLED_Pin, HIGH);
      }
      // -------------
      // SETUP FASTLED  
      // -------------
      FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(LEDs, NUM_LEDS_MAX);
      FastLED.setDither(false);
      FastLED.setCorrection(TypicalLEDStrip);
      FastLED.setMaxPowerInVoltsAndMilliamps(5, milliamps);
      FastLED.setBrightness(activeBrightness);

      //-----------------
      // MQTT Setup
      //-----------------
      if (mqttEnabled) {
        //Attempt to connect to MQTT broker - if fails, disable MQTT
        if (!setup_mqtt()) {
          mqttEnabled = false;
          mqttConnected = false;
        } else {
          initialSyncRequired = true;
          syncStep = 0;
        }
      }

      // --------------
      // SETUP TFMINI
      // --------------
      // TFMini uses Serial pins, so Serial2 is used so normal serial output to console is still possible
      Serial2.begin(115200, SERIAL_8N1, tfminiRX_Pin, tfminiTX_Pin);
      delay(100);
      tfMiniEnabled = (tfmini.begin(&Serial2));

      // ---------------------------------------------------
      // Setup Side Sensor VL53L0X (only available on ESP32)
      // ---------------------------------------------------
      if (useSideSensor) {
        bus1.begin(tofDat_Pin, tofClk_Pin, 400000);
        side_sensor.setBus(&bus1);
        if (side_sensor.init()) {
          side_sensor.setTimeout(500);
          side_sensor.startContinuous(); 
        } else {
          useSideSensor = false;
          if (useBootLightsLED) {
            fill_solid(LEDs, numLEDs, CRGB::Orange);
            FastLED.show();
            delay(1000);
            allLEDsOff(false);
          }
        }
      }
      // -----------------------------------------------------------------
      // Flash LEDs red, green, blue to indicate successful hardware setup 
      // -----------------------------------------------------------------
      if (useBootLightsLED) {
        fill_solid(LEDs, numLEDs, CRGB::Red);
        FastLED.show();
        delay(500);
        fill_solid(LEDs, numLEDs, CRGB::Green);
        FastLED.show();
        delay(500);
        fill_solid(LEDs, numLEDs, CRGB::Blue);
        FastLED.show();
        delay(500);
      }
      fill_solid(LEDs, numLEDs, CRGB::Black);  //turn off LEDs
      FastLED.show();
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("LEDs Blue - FASTLED ok");
      #endif
      delay(1000);
      fill_solid(LEDs, numLEDs, CRGB::Black);
      FastLED.show();
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("LEDs Reset to off");
      #endif
      //-----------------------------
      // Setup OTA Updates
      //-----------------------------
      if (!noWiFiMode) {
        ArduinoOTA.setHostname(otaHostName.c_str());
        ArduinoOTA.onStart([]() {
          String type;
          if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
          } else { // U_FS
            type = "filesystem";
          }
          // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        });
        ArduinoOTA.begin();
      }
      //Define Effects
      defineEffects();
      // Set interval distance based on current Effect
      intervalDistance = calculateInterval();
    }
  }
  isBooting = false;
}

// =============================
//   MAIN LOOP
// =============================
void loop() {
  //Process DNS queries if running in AP mode
  if (noWiFiMode || onboarding) {
    dnsServer.processNextRequest();
  }  
  //Handle OTA updates when OTA flag set via HTML call to http://ip_address/otaupdate
  if (ota_flag && !noWiFiMode) {
    updateOTA();  //Show update on LED strip
    uint32_t ota_time_start = millis();
    while (ota_time_elapsed < ota_time) {
      ArduinoOTA.handle();  
      ota_time_elapsed = millis()-ota_time_start;   
      delay(10); 
    }
    ota_flag = false;
    updateSleepMode();
  }
  //Handle any web calls
  server.handleClient();
  if (onboarding) {
    runOnboardingProcesses();
  } else {
    if ((!overrideSensors) && (!mainTestFlag)) {
      //regular code logic here
      if (mqttEnabled && (WIFIMODE > 0 && WiFi.status() == WL_CONNECTED)) {
        mqttConnected = client.connected();
      }
      uint32_t currentMillis = millis();
      int16_t distance = 0;
      //Attempt to get reading from TFMini
      if (tfMiniEnabled) {
        if (tfmini.getData(distance)) {
          tf_dist = distance * 10;
        } else {
          tf_dist = 8888;  //Default value if reading unsuccessful
        }
      } else {
        tf_dist = 9999;  //Default value if TFMini not enabled (serial connection failed)
      }

      if ((useSideSensor) && (sideSensorPos > 0) && (leftDistance > 0) && (rightDistance > 0)) {
        vl_side_dist = side_sensor.readRangeContinuousMillimeters();
      } else {
        vl_side_dist = 0;
      }

      //Determine if car (or other object) present in any zones
      if (tf_dist <= wakeDistance) {
        if (!carDetected) {
          carDetectedCounter ++;
        }
        if (carDetectedCounter > carDetectedCounterMax) {  //eliminate trigger on noise
          carDetectedCounter = 0;
          nocarDetectedCounter = 0;
          carDetected = true;
          if (coldStart) {
            coldStart = false;
            exitSleepTimerStarted = true;
            parkSleepTimerStarted = false;
          } else {
            exitSleepTimerStarted = false;
            parkSleepTimerStarted = true;
          }
          startTime = currentMillis;
          FastLED.setBrightness(activeBrightness);
          isAwake = true;
          ledState = true;
        }
      } else {
        nocarDetectedCounter ++;
        if (nocarDetectedCounter > nocarDetectedCounterMax) {  //eliminate trigger on noise
          if (!exitSleepTimerStarted) {
            if ((carDetected) || (coldStart)) {
              exitSleepTimerStarted = true;
              coldStart = false;
              startTime = currentMillis;
            }
          }
          carDetected = false;
          carDetectedCounter = 0;
          nocarDetectedCounter = 0;
          mqttZone = "Vacant";
        }
      }

      //Update LEDs
      if ((carDetected) && (isAwake)) {
        if (tf_dist <= backupDistance) {
          //Beyond minimum distance - flash backup!
          blinkLEDs(ledColorBackup);
          mqttZone = "Backup";
          activeLEDColor = ledColorBackup;
        } else if (tf_dist <= parkDistance) {  
          //In desired parked distance
          fill_solid(LEDs, numLEDs, ledColorParked);
          mqttZone = "Parked";
          activeLEDColor = ledColorParked;
        } else if ((tf_dist > startDistance) && (tf_dist <= wakeDistance)) {
          //Beyond start distance but within wake distance
          fill_solid(LEDs, numLEDs, ledColorWake);
          mqttZone = "Wake";
          activeLEDColor = ledColorWake;
        } else if ((tf_dist <= startDistance) && (tf_dist > parkDistance)) {
          //Update based on selected effect
          if (ledEffect_m1 == "Out-In") updateOutIn(tf_dist);
          else if (ledEffect_m1 == "In-Out") updateInOut(tf_dist);
          else if (ledEffect_m1 == "Full-Strip") updateFullStrip(tf_dist);
          else if (ledEffect_m1 == "Full-Strip-Inv") updateFullStripInv(tf_dist);
          else if (ledEffect_m1 == "Solid") updateSolid(tf_dist);
          
          mqttZone = "Active";
          activeLEDColor = ledColorActive;
          //Check side sensor
          if (useSideSensor && (vl_side_dist > 0) && (vl_side_dist < 1300)) {
            // --- LEFT SIDE MOUNT (sideSensorPos == 2) ---
            if (sideSensorPos == 2) {
              if (vl_side_dist < leftDistance) blinkSideLEDs(ledColorBackup, 2); // Too far LEFT: Flash LEFT LEDs
              else if (vl_side_dist > rightDistance) blinkSideLEDs(ledColorBackup, 1); // Too far RIGHT: Flash RIGHT LEDs
            // --- RIGHT SIDE MOUNT (sideSensorPos == 1) ---
            } else if (sideSensorPos == 1) {
              if (vl_side_dist < rightDistance) blinkSideLEDs(ledColorBackup, 1); // Too far RIGHT: Flash RIGHT LEDs
              else if (vl_side_dist > leftDistance) blinkSideLEDs(ledColorBackup, 2); // Too far LEFT: Flash LEFT LEDs
            }
          }
        }
      }
      //Put system to sleep if parking or exit time elapsed 
      uint32_t elapsedTime = currentMillis - startTime;
      if (((elapsedTime > (maxOperationTimePark * 1000)) && (parkSleepTimerStarted)) || ((elapsedTime > (maxOperationTimeExit * 1000)) && (exitSleepTimerStarted  ))) {
        updateSleepMode();
        forceMQTTUpdate = true;
        isAwake = false;
        startTime = currentMillis;
        exitSleepTimerStarted = false;
        parkSleepTimerStarted = false;
      }
      //Show/Refresh LED Strip
      FastLED.show();
    }

    // ===================
    // MQTT STATE UPDATES
    // ===================
    if ((!mainTestFlag) && (!noWiFiMode)) {
      // Don't update MQTT when in test mode or if sensors are overridden
      // If MQTT enabled, check connection (Non-Blocking) and reestablish if needed.
      if (mqttEnabled && (WIFIMODE > 0 && WiFi.status() == WL_CONNECTED)) {
        if (!mqttConnected) { 
          // Calls non-blocking reconnect_soft
          reconnect_soft(); 
        } else {
          client.loop(); 
        }
      }
      //Handle any required MQTT state publishes (thread-safe)
      if ((mqttConnected) && (!isBooting)) {
        handleLiveMqttUpdates();
        unsigned long curMillis = millis();
        if (curMillis - lastSyncStepTime > INIT_SYNC_INTERVAL) {
          if (initialSyncRequired) {
            handleInitMqttSync(); 
          }
          handleQueuedMqttUpdates(); 
          lastSyncStepTime = curMillis;
        }
      }
    }
    delay(150);
  }
}

//==========================================
// CONFIGURATION FILES FUNCTIONS (LittleFS)
//==========================================
//------------------------------
// Read Main Configuration File
//------------------------------
void readConfigFile() {
  if (LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("mounted file system");
    #endif
    if (LittleFS.exists("/config.json")) {
      //file exists, reading and loading
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
            Serial.println("reading config file");
      #endif
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
                Serial.println("opened config file");
        #endif
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        //DynamicJsonDocument json(1024);
        JsonDocument json;
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if (!deserializeError) {

          #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
            Serial.println("\nparsed json");
          #endif
          // Read values here from LittleFS (use defaults for all values in case they don't exist to avoid potential boot loop)
          //DON'T NEED TO STORE OR RECALL WIFI INFO - Written to flash automatically by library when successful connection.
          deviceName = json["device_name"] | "ParkingAsst";
          noWiFiMode = json["no_wifi_mode"] | 0;
          manualAPName = json["manual_ap_name"] | String(deviceName + "_Hotspot");  //have to add String() to satisfy compiler
          manualAPPwd = json["manual_ap_pwd"] | "";
          ledData_Pin = json["led_data_pin"] | LED_DATA_PIN;
          tfminiRX_Pin = json["tfmini_rx_pin"] | TFMINI_RX_PIN;
          tfminiTX_Pin = json["tfmini_tx_pin"] | TFMINI_TX_PIN;
          tofDat_Pin = json["tof_dat_pin"] | TOF_DAT_PIN;
          tofClk_Pin = json["tof_clk_pin"] | TOF_CLK_PIN;
          onboardLED_Pin = json["onboard_led_pin"] | ONBOARD_LED;
          milliamps = json["milli_amps"] | MILLI_AMPS;

          numLEDs = constrain((int)(json["led_count"] | 30), 5, NUM_LEDS_MAX);
          rightLEDWiring = json["right_led_wiring"]|0;
          defaultBrightness = json["led_brightness_active"]|100;
          sleepBrightness_dflt = json["led_brightness_sleep"]|5;
          ledEffect_dflt = json["led_effect"]|"Out-In";
          useBootLightsLED = json["use_boot_leds"] | 1;

          //Config values stored as hex colors.  Convert to CRGB for local use
          ledColorStandby_dflt = hexToCRGB(json["color_standby"] | "#0000ff"); //default blue
          ledColorWake_dflt = hexToCRGB(json["color_wake"] | "#00ff00");       //default green
          ledColorActive_dflt = hexToCRGB(json["color_active"] | "#ffff00");   //default yellow
          ledColorParked_dflt = hexToCRGB(json["color_parked"] | "#ff0000");   //default red
          ledColorBackup_dflt = hexToCRGB(json["color_backup"] | "#ff0000");   //default red

          uomDistance = json["uom_distance"] | 1;     //default to mm
          wakeDistance_dflt = json["wake_mils"] | 3048;
          startDistance_dflt = json["start_mils"] | 1829;
          parkDistance_dflt = json["park_mils"] | 610;
          backupDistance_dflt = json["backup_mils"] | 457;
 
          useSideSensor = json["use_side_sensor"] | 0;
          sideSensorPos = json["side_sensor_pos"] | 0;
          leftDistance_dflt = json["left_distance"] | 0;
          rightDistance_dflt = json["right_distance"] | 0;

          maxOperationTimePark = json["led_park_time"] | 60;
          maxOperationTimeExit = json["led_exit_time"] | 5;
          nocarDetectedCounterMax = json["no_car_debounce"] | 10;

          mqttAddr_1 = json["mqtt_addr_1"] | 0;
          mqttAddr_2 = json["mqtt_addr_2"] | 0;
          mqttAddr_3 = json["mqtt_addr_3"] | 0;
          mqttAddr_4 = json["mqtt_addr_4"] | 0;
          //Disable MQTT if IP = 0.0.0.0
          if ((mqttAddr_1 == 0) && (mqttAddr_2 == 0) && (mqttAddr_3 == 0) && (mqttAddr_4 == 0)) {
            mqttEnabled = false;
            mqttConnected = false;
          } else {
           mqttEnabled = true;
          }
          // Force-disable MQTT if running in AP Mode
          if (noWiFiMode) {
            mqttEnabled = false;
            mqttConnected = false;
          }
          mqttPort = json["mqtt_port"]|0;
          mqttTelePeriod = json["mqtt_tele_period"]|60;
          mqttUser = json["mqtt_user"]|"mqttuser";
          mqttPW = json["mqtt_pw"]|"mqttpwd";
          mqttTopicSub = json["mqtt_topic_sub"]|"parkasst";
          mqttTopicPub = json["mqtt_topic_pub"]|"parkasst";
 
         //=== Set or calculate other globals =====
          wifiHostName = deviceName;
          mqttClient = deviceName;
          otaHostName = deviceName + "_OTA";
          useOnboardLED = (onboardLED_Pin > 0);
         //=== Set all 'active' vars to the initial defaults
         activeBrightness = defaultBrightness;
         lastBrightness = activeBrightness;
         sleepBrightness = sleepBrightness_dflt;
         ledColorStandby = ledColorStandby_dflt;
         ledColorWake = ledColorWake_dflt;
         ledColorActive = ledColorActive_dflt;
         ledColorParked = ledColorParked_dflt;
         ledColorBackup = ledColorBackup_dflt;
         ledEffect_m1 = ledEffect_dflt;
         wakeDistance = wakeDistance_dflt;
         startDistance = startDistance_dflt;
         parkDistance = parkDistance_dflt;
         backupDistance = backupDistance_dflt;
         leftDistance = leftDistance_dflt;
         rightDistance = rightDistance_dflt;
           
        } else {
          #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
            Serial.println("failed to load json config");
          #endif
          onboarding = true;
        }
        configFile.close();
      } else {
        onboarding = true;
      }
    } else {
      onboarding = true;
    }
    LittleFS.end();
  } else {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println("failed to mount FS");
    #endif
    onboarding = true;
  }
}

//---------------------------------
// Write main config file to flash (LittleFS)
//---------------------------------
void writeConfigFile(bool restart_ESP) {
  // Writes new settings to LittleFS (new boot defaults)
  if (LittleFS.begin()) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println("Attempting to update boot settings");
    #endif
    JsonDocument json;
    json.clear();
    json["device_name"] = deviceName;
    json["no_wifi_mode"] = (noWiFiMode ? 1 : 0);
    json["manual_ap_name"] = manualAPName;
    json["manual_ap_pwd"] = manualAPPwd;       
    json["led_data_pin"] = ledData_Pin;
    json["tfmini_rx_pin"] = tfminiRX_Pin;
    json["tfmini_tx_pin"] = tfminiTX_Pin;
    json["tof_dat_pin"] = tofDat_Pin;
    json["tof_clk_pin"] = tofClk_Pin;
    json["onboard_led_pin"] = onboardLED_Pin;
    json["led_count"] = numLEDs;
    json["right_led_wiring"] = (rightLEDWiring ? 1 : 0);
    json["led_brightness_active"] = defaultBrightness;
    json["led_brightness_sleep"] = sleepBrightness_dflt;
    json["led_effect"] = ledEffect_dflt;
    json["use_boot_leds"] = (useBootLightsLED ? 1 : 0);
  // ===================  NEED TO CONVERT FROM CRGB to Hex/RGB ============
    json["color_standby"] = crgbToHex(ledColorStandby_dflt);
    json["color_wake"] = crgbToHex(ledColorWake_dflt);
    json["color_active"] = crgbToHex(ledColorActive_dflt);
    json["color_parked"] = crgbToHex(ledColorParked_dflt);
    json["color_backup"] = crgbToHex(ledColorBackup_dflt);

    json["uom_distance"] = uomDistance;
    json["wake_mils"] = wakeDistance_dflt;
    json["start_mils"] = startDistance_dflt;
    json["park_mils"] = parkDistance_dflt;
    json["backup_mils"] = backupDistance_dflt;
    json["use_side_sensor"] = (useSideSensor ? 1 : 0);
    json["side_sensor_pos"] = sideSensorPos;
    json["left_distance"] = leftDistance_dflt;
    json["right_distance"] = rightDistance_dflt;

    json["no_car_debounce"] = nocarDetectedCounterMax;
    json["led_park_time"] = maxOperationTimePark;
    json["led_exit_time"] = maxOperationTimeExit;

    json["mqtt_addr_1"] = mqttAddr_1;
    json["mqtt_addr_2"] = mqttAddr_2;
    json["mqtt_addr_3"] = mqttAddr_3;
    json["mqtt_addr_4"] = mqttAddr_4;
    json["mqtt_port"] = mqttPort;
    json["mqtt_tele_period"] = mqttTelePeriod;
    json["mqtt_user"] = mqttUser;
    json["mqtt_pw"] = mqttPW;
    json["mqtt_topic_sub"] = mqttTopicSub;
    json["mqtt_topic_pub"] = mqttTopicPub;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("failed to open config file for writing");
      #endif
      configFile.close();
      LittleFS.end();
      return;
    } else {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        serializeJson(json, Serial);
      #endif
      serializeJson(json, configFile);
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("Settings saved.");
      #endif
      configFile.close();

      if (restart_ESP) {
        //If enabled, turn off onboard LED as reboot indicator
        if (useOnboardLED) {
          digitalWrite(onboardLED_Pin, LOW);
          delay(500);
        }
        ESP.restart();
      }
    }
    LittleFS.end();
  } else {
//could not mount filesystem
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println("failed to mount FS");
    #endif
  }
}

//---------------------------
// Get current config values
//---------------------------
String getCurrentConfig() {
  //This function just reads and returns the raw contents of the config file.
  //No app vars are changed.  Used for web app config file dump page (instead of global string).
  String fileContents = "";
  if (LittleFS.begin(false)) {  //do not format on failure
    if (LittleFS.exists("/config.json")) {
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();
        JsonDocument json;
        auto deserializeError = deserializeJson(json, buf.get());
        if (!deserializeError) {        
          if (json.containsKey("mqtt_pw")) {
            json["mqtt_pw"] = "********";
          }
          if (json.containsKey("no_wifi_mode")) {
            bool apMode = (json["no_wifi_mode"] == 1);
            if (!apMode) {
              // Standard Wi-Fi mode: AP settings are inactive
              json["manual_ap_name"] = "---";
              json["manual_ap_pwd"] = "---";
            } else {
              // AP Mode active: Mask password only if one is configured
              String pwd = json["manual_ap_pwd"] | "";
              if (pwd.length() > 0) {
                json["manual_ap_pwd"] = "********";
              } else {
                json["manual_ap_pwd"] = "(None - Open)";
              }
            }
          }
          serializeJsonPretty(json, fileContents);
        } else {
          fileContents = "ERROR RETURNED: Could not deserialize config file (does not appear to be JSON).";
        }
      } else {
        fileContents = "ERROR RETURNED: 'config.json' could not be opened - See Troubleshooting.";
      }
    } else {
      fileContents = "ERROR RETURNED: 'config.json' file does not exist.  Be sure you have saved the config file at least one time.";
    }
    LittleFS.end();
  } else {
    //LittleFS.begin failed
    fileContents = "ERROR RETURNED: Failed to mount LittleFS - See Troubleshooting.";
  }
  return fileContents;
}

//----------------------------
// Read Discovery Config File
//----------------------------
void loadDiscoveryConfig() {
  if (LittleFS.begin(false)) {
    if (LittleFS.exists("/discovery.json")) {
      File file = LittleFS.open("/discovery.json", "r");
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        currentDiscovery.exists = true;
        currentDiscovery.deviceName = doc["devname"].as<String>();
        currentDiscovery.controls = doc["controls"];
        currentDiscovery.sensors = doc["sensors"];
        currentDiscovery.config = doc["config"];
        currentDiscovery.diagnostics = doc["diag"];
      }
      file.close();
      LittleFS.end();
    } else {
      // Defaults if file doesn't exist
      String devName = deviceName;  // Use global variable
      //Replace any hyphens with spaces (hyphens not allowed in Discovery device name)
      devName.replace("-", " ");
      currentDiscovery.exists = false;
      currentDiscovery.deviceName = devName; 
      currentDiscovery.controls = true;
      currentDiscovery.sensors = true;
      currentDiscovery.config = true;
      currentDiscovery.diagnostics = true;
    }
  }
}
//-----------------------------
// Write Discovery Config File
//-----------------------------
bool saveDiscoveryConfig() {
  if (!LittleFS.begin(false)) return false;
  JsonDocument doc;
  doc["devname"] = currentDiscovery.deviceName;
  doc["controls"] = currentDiscovery.controls;
  doc["sensors"] = currentDiscovery.sensors;
  doc["config"] = currentDiscovery.config;
  doc["diag"] = currentDiscovery.diagnostics;

  File file = LittleFS.open("/discovery.json", "w");
  if (!file) return false;
  if (serializeJson(doc, file) == 0) {
    file.close();
    LittleFS.end();
    return false;
  }
  currentDiscovery.exists = true;
  file.close();
  LittleFS.end();
  return true;
}
//------------------------------
// Delete Discovery Config File
//------------------------------
void deleteDiscoveryConfig() {
  if (LittleFS.begin(false)) {
    if (LittleFS.exists("/discovery.json")) {
      LittleFS.remove("/discovery.json");
    }
    LittleFS.end();
  }  
  currentDiscovery.exists = false;
}
//----------------------------------
// Get Current Discovery Config File
//----------------------------------
String getDiscoveryConfig() {
  //This function just reads and returns the raw contents of the config file.
  //No app vars are changed.  Used for web app config file dump page (instead of global string).
  String fileContents = "";
  if (LittleFS.begin(false)) {  //do not format on failure
    if (LittleFS.exists("/discovery.json")) {
      File configFile = LittleFS.open("/discovery.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();  
        JsonDocument json;
        auto deserializeError = deserializeJson(json, buf.get());
        if (!deserializeError) {        
          serializeJsonPretty(json, fileContents);
        } else {
          fileContents = "ERROR RETURNED: Could not deserialize discovery file (does not appear to be JSON).";
        }
      } else {
        fileContents = "ERROR RETURNED: 'discovery.json' could not be opened - See Troubleshooting.";
      }
    } else {
      fileContents = "'discovery.json' file does not exist.  This will be the case if Discovery is not enabled.";
    }
    LittleFS.end();  
  } else {
    //LittleFS.begin failed
    fileContents = "ERROR RETURNED: Failed to mount LittleFS - See Troubleshooting.";
  }
  return fileContents;
}

// ==============================
//  Define Effects 
// ==============================
//  Increase array size above if adding new
//  Effect name must not exceed 15 characters and must be a String
void defineEffects() {
  Effects[0] = "Out-In";
  Effects[1] = "In-Out";
  Effects[2] = "Full-Strip";
  Effects[3] = "Full-Strip-Inv";
  Effects[4] = "Solid";
}

//===============================
// Web pages and handlers
//===============================
// ---------------------------
//  Setup web handlers
// ---------------------------
void setupWebHandlers() {
  server.on("/", HTTP_GET, handleRoot);
  //Onboarding
  server.on("/onboard", handleOnboard);
  server.on("/onboard-status", HTTP_GET, handleOnboardStatus);
  //Main Page
  server.on("/mainjson", webMainPageJson);
  server.on("/mainapply", handleMainPage);
  //Hardware Settings
  server.on("/system", webSystemPage);
  server.on("/systemjson", webSystemPageJson);
  server.on("/systemapply", handleSystem);
  server.on("/calibrate", webCalibratePage);
  server.on("/data", getCalibrationData);
  //Integrations (MQTT and Discovery)
  server.on("/integrations", webIntegratePage);
  server.on("/integratejson", webIntegratePageJson);
  server.on("/integrateapply", handleIntegrations);
  server.on("/savediscovery", handleDiscoverySave);
  //Controller Functions
  server.on("/restart", webRestartPage);
  server.on("/reset",  webResetPage);
  server.on("/firmwareupdate", webFirmwareUpdate);
  server.on("/update", HTTP_POST, []() {
    handleWebUpdateEnd();
  }, []() {
    handleWebUpdate();
  });  
  server.on("/otaupdate", handleOTAUpdate); //OTAUpdate via Arduino IDE
  server.on("/configdump", webConfigDump);
  server.on("/info", handleInfoDump);
  server.on("/infojson", infoDumpJson);
  server.on("/refresh", forceMqttRefresh);
  //Special Handlers for Android/Window auto-portal launch
  server.on("/generate_204", handleRoot);        // Android Probe
  server.on("/gen_204", handleRoot);             // Android Probe
  server.on("/connecttest.txt", handleRoot);     // Windows Probe
  server.on("/redirect", handleRoot);            // Microsoft Probe  
  //Misc
  server.on("/api", handleAPI);
  server.onNotFound(handleNotFound);
  server.begin();
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("Setup complete - starting main loop");
  #endif
}

// Main Settings page
// Root / Main Settings or Onboarding page handler
void handleRoot() {
  String page;
  if (onboarding) {
    //Onboarding page
    page.reserve(strlen(onboard) + 300);
    page = onboard;
    page.replace("*VAR_APP_NAME*", APPNAME);
    page.replace("*VAR_CURRENT_VER*", VERSION);
    page.replace("*SSID*", wifiSSID);
    page.replace("*WIFIPW*", wifiPW);
    page.replace("*DEVICENAME*", deviceName);
    server.send(200, "text/html", page);
  } else {
    //Main Page
    mainTestFlag = false;
    page.reserve(strlen(mainpage) + 2000);
    page = mainpage;
    page.replace("*VAR_APP_NAME*", APPNAME);
    page.replace("*VAR_CURRENT_VER*", VERSION);
    page.replace("*DEVICENAME*", deviceName);
    if (noWiFiMode) {
      page.replace("*SSID*", String("AP MODE (" + manualAPName + ")"));
    } else {
      page.replace("*SSID*", wifiSSID);
    }
    page.replace("*MACADDR*", strMacAddr);
    page.replace("*IPADDR*", baseIP);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");  //sends headers
    server.sendContent(page);
    server.sendContent(html_footer);
    server.sendContent("");  
  }
}
//---------------------
// Onboarding
//---------------------
void handleOnboard() {
  wifiSSID = server.arg("ssid");
  wifiPW = server.arg("wifipw");           
  deviceName = server.arg("devicename");

  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.print("SSID: "); Serial.println(wifiSSID);
    Serial.println("Buffering onboarding layout payload...");
  #endif

  // Setup non-blocking execution markers
  onboardConnecting = true;
  triggerWiFiBegin = true; // Signals loop to fire up radio on next cycle pass
  onboardStartMilli = millis();
  wifiConnected = false;
  onboardRebootReady = false;
  onboardSuccessMilli = 0;

  // Substitute tags inside dynamic card template
  String page = onboardpost;
  page.replace("VAR_APP_NAME", APPNAME);
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*DEVICE_NAME*", deviceName);
  page.replace("*SSID*", wifiSSID);

  server.send(200, "text/html", page);
}

void handleOnboardStatus() {
  String json = "{";
  
  if (WiFi.status() == WL_CONNECTED) {
    json += "\"status\":\"success\",";
    json += "\"ssid\":\"" + wifiSSID + "\",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
    
    // Track when client browser successfully flags connection success profile
    if (!onboardRebootReady) {
      onboardRebootReady = true;
      onboardRebootMilli = millis();
    }
  } 
  else if (!onboardConnecting && !wifiConnected) {
    // Timeout cycle parsed connection verification as dead failure
    json += "\"status\":\"failed\",";
    json += "\"ip\":\"0.0.0.0\"";
  } 
  else {
    // Radio stack actively validating connection handshakes in background
    json += "\"status\":\"connecting\",";
    json += "\"ip\":\"0.0.0.0\"";
  }
  
  json += "}";
  server.send(200, "application/json", json);
}

void runOnboardingProcesses() {
// Delayed Radio Action: Runs on the first loop tick after webpage delivery to protect buffers
  if (triggerWiFiBegin) {
    triggerWiFiBegin = false;
    WiFi.begin(wifiSSID.c_str(), wifiPW.c_str());
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println(F("Web template dispatched safely. Hardware radio dialing out..."));
    #endif
    return; 
  }

  // Monitor ongoing network connections
  if (onboardConnecting) {
    if (WiFi.status() == WL_CONNECTED) {
      onboardConnecting = false;
      wifiConnected = true;
      onboardSuccessMilli = millis();
      
      baseIP = WiFi.localIP().toString();
      strMacAddr = WiFi.macAddress();
      
      // Fire up mDNS now that credentials are valid and local services are up
      if (MDNS.begin(wifiHostName.c_str())) {
        MDNS.addService("http", "tcp", 80);
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.print(F("mDNS broadcast established at: http://"));
          Serial.print(wifiHostName);
          Serial.println(F(".local"));
        #endif
      }
    } 
    // Allow up to 60000ms (60 seconds) to allow slower routers to allocate DHCP leases
    else if (millis() - onboardStartMilli > 60000) { 
      onboardConnecting = false;
      wifiConnected = false;
      
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println(F("Onboarding connection handshake timed out. Resetting radio core interface..."));
      #endif

      setupSoftAP(); 
    }
  }
  // Coordinate deferred reboot cycle safely after success
  if (wifiConnected && !onboardConnecting) {
    if ((onboardRebootReady && (millis() - onboardRebootMilli > 5000)) || 
        (millis() - onboardSuccessMilli > 15000)) {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.println(F("Committing configuration file and resetting system core..."));
        #endif
        writeConfigFile(true); 
    }
  }
}
//-------------------
// Main Page
//-------------------
// Main Page Data
void webMainPageJson() {
  String jsonString;
  JsonDocument doc;
  doc.clear();
  doc["which_data"] = String(whichData);  //Current data request
  doc["page_mode"] = mainPageMode;
  doc["page_color_msg"] = mainPageColorMsg;
  doc["page_dist_msg"] = mainPageDistMsg;
  doc["uom_distance"] = uomDistance;
  doc["sensor_override"] = (overrideSensors ? 1 : 0);
  doc["use_side_sensor"] = (useSideSensor ? 1 : 0);
  doc["ap_mode"] = (noWiFiMode ? 1 : 0);
  //effect drop down
  JsonArray effectArray = doc.createNestedArray("effects");
  for (int i = 0; i < numberOfEffects; i++) {
    JsonObject effectObj = effectArray.createNestedObject();
    effectObj["index"] = i;
    effectObj["effectname"] = Effects[i];
  }

  if (mainPageMode == "colordfltbtn") {
    doc["test_flag_color"] = 0;
    doc["color_wake"] = crgbToHex(ledColorWake_dflt);
    doc["color_active"] = crgbToHex(ledColorActive_dflt);
    doc["color_parked"] = crgbToHex(ledColorParked_dflt);
    doc["color_backup"] = crgbToHex(ledColorBackup_dflt);
    doc["color_standby"] = crgbToHex(ledColorStandby_dflt);
    doc["brightness"] = defaultBrightness;
    doc["brightness_standby"] = sleepBrightness_dflt;
    doc["led_effect"] = ledEffect_dflt;
  } else if (mainPageMode == "distdfltbtn") {
    doc["test_flag_dist"] = 0;
    //Convert mm to inches (as 1-decimal float) if UOM = 0 (inches)
    doc["dist_wake"] = ((uomDistance == 1) ? (float)wakeDistance_dflt : mmToInches(wakeDistance_dflt));
    doc["dist_active"] = ((uomDistance == 1) ? (float)startDistance_dflt : mmToInches(startDistance_dflt));
    doc["dist_parked"] = ((uomDistance == 1) ? (float)parkDistance_dflt : mmToInches(parkDistance_dflt));
    doc["dist_backup"] = ((uomDistance == 1) ? (float)backupDistance_dflt : mmToInches(backupDistance_dflt));

    if (useSideSensor) {
      doc["dist_left"] = ((uomDistance == 1) ? (float)leftDistance_dflt : mmToInches(leftDistance_dflt));
      doc["dist_right"] = ((uomDistance == 1) ? (float)rightDistance_dflt : mmToInches(rightDistance_dflt));
    } else {
      doc["dist_left"] = 0;
      doc["dist_right"] = 0;
    }
  } else {  
    //all fields from active
    doc["test_flag_color"] = 0;
    doc["color_wake"] = crgbToHex(ledColorWake);
    doc["color_active"] = crgbToHex(ledColorActive);
    doc["color_parked"] = crgbToHex(ledColorParked);
    doc["color_backup"] = crgbToHex(ledColorBackup);
    doc["color_standby"] = crgbToHex(ledColorStandby);
    doc["led_effect"] = ledEffect_m1;
    doc["brightness"] = activeBrightness;
    doc["brightness_standby"] = sleepBrightness;
    doc["test_flag_dist"] = 0;
    //Convert mm to inches (as 1-decimal float) if UOM = 0 (inches)
    doc["dist_wake"] = ((uomDistance == 1) ? (float)wakeDistance : mmToInches(wakeDistance));
    doc["dist_active"] = ((uomDistance == 1) ? (float)startDistance : mmToInches(startDistance));
    doc["dist_parked"] = ((uomDistance == 1) ? (float)parkDistance : mmToInches(parkDistance));
    doc["dist_backup"] = ((uomDistance == 1) ? (float)backupDistance : mmToInches(backupDistance));
    if (useSideSensor) {
      doc["dist_left"] = ((uomDistance == 1) ? (float)leftDistance : mmToInches(leftDistance));
      doc["dist_right"] = ((uomDistance == 1) ? (float)rightDistance : mmToInches(rightDistance));
      doc["side_sensor_pos"] = sideSensorPos;
    } else {
      doc["dist_left"] = 0;
      doc["dist_right"] = 0;
      doc["side_sensor_pos"] = 0;
    }
  }
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
  whichData = "all";  //reset for next request (all is default)
  mainPageMode = "";
  mainPageColorMsg = "";
  mainPageDistMsg = "";
}
// Main page handler - top color section
void handleMainPage() {
  if (server.hasArg("plain")) {
    String payload = server.arg("plain");
    JsonDocument doc;
    DeserializationError dsError = deserializeJson(doc, payload);
    
    if (!dsError) {
      String mode = doc["page_mode"].as<String>();
      JsonDocument responseDoc;
        responseDoc["page_mode"] = mode;
        responseDoc["test_flag_color"] = 0;
        responseDoc["test_flag_dist"] = 0;
        responseDoc["use_side_sensor"] = (useSideSensor ? 1 : 0);
        responseDoc["side_sensor_pos"] = sideSensorPos;
        responseDoc["uom_distance"] = uomDistance;
        responseDoc["sensor_override"] = (overrideSensors ? 1 : 0);
      //--- Default buttons ---
      if (mode == "colordfltbtn") {
        //Build and return default values to web page
        responseDoc["which_data"] = "color";
        responseDoc["color_wake"] = crgbToHex(ledColorWake_dflt);
        responseDoc["color_active"] = crgbToHex(ledColorActive_dflt);
        responseDoc["color_parked"] = crgbToHex(ledColorParked_dflt);
        responseDoc["color_backup"] = crgbToHex(ledColorBackup_dflt);
        responseDoc["color_standby"] = crgbToHex(ledColorStandby_dflt);
        responseDoc["led_effect"] = ledEffect_dflt;
        responseDoc["brightness"] = defaultBrightness;
        responseDoc["brightness_standby"] = sleepBrightness_dflt;
        responseDoc["page_color_msg"] = "Defaults loaded, but not applied.";
        //populate effect dropdown
        JsonArray effectArray = responseDoc.createNestedArray("effects");
        for (int i = 0; i < numberOfEffects; i++) {
          JsonObject effectObj = effectArray.createNestedObject();
          effectObj["index"] = i;
          effectObj["effectname"] = Effects[i];
        }
        String responseString;
        serializeJson(responseDoc, responseString);
        server.send(200, "application/json", responseString);
        return;
      }
      if (mode == "distdfltbtn") {
        responseDoc["which_data"] = "dist";
        responseDoc["dist_wake"] = ((uomDistance == 1) ? (float)wakeDistance_dflt : mmToInches(wakeDistance_dflt));
        responseDoc["dist_active"] = ((uomDistance == 1) ? (float)startDistance_dflt : mmToInches(startDistance_dflt));
        responseDoc["dist_parked"] = ((uomDistance == 1) ? (float)parkDistance_dflt : mmToInches(parkDistance_dflt));
        responseDoc["dist_backup"] = ((uomDistance == 1) ? (float)backupDistance_dflt : mmToInches(backupDistance_dflt));
        if (useSideSensor) {
          responseDoc["dist_left"] = ((uomDistance == 1) ? (float)leftDistance_dflt : mmToInches(leftDistance_dflt));
          responseDoc["dist_right"] = ((uomDistance == 1) ? (float)rightDistance_dflt : mmToInches(rightDistance_dflt));
        } else {
          responseDoc["dist_left"] = 0;
          responseDoc["dist_right"] = 0;
        }
        responseDoc["page_dist_msg"] = "Defaults loaded, but not applied.";
        String responseString;
        serializeJson(responseDoc, responseString);
        server.send(200, "application/json", responseString);
        return;
      }
      //Apply Buttons
      if (mode == "colorapplybtn") {
        //Update active vars
        ledColorWake = hexToCRGB(doc["color_wake"].as<String>());
        ledColorActive = hexToCRGB(doc["color_active"].as<String>());
        ledColorParked = hexToCRGB(doc["color_parked"].as<String>());
        ledColorBackup = hexToCRGB(doc["color_backup"].as<String>());
        ledColorStandby = hexToCRGB(doc["color_standby"].as<String>());
        ledEffect_m1 = doc["led_effect"].as<String>();
        activeBrightness = doc["brightness"];
        sleepBrightness = doc["brightness_standby"];
        intervalDistance = calculateInterval();
        //Update MQTT Topics
        if (mqttEnabled) {
          forceMqttRefresh();
        }
        //Show confirmation here - just flash green
        FastLED.setBrightness(activeBrightness);
        allLEDsOff(false);  //needed to apply brightness
          fill_solid(LEDs, numLEDs, CRGB::Green); 
          FastLED.show();
          delay(750);
          allLEDsOff(true);

        responseDoc["which_data"] = "color";
        responseDoc["color_wake"] = crgbToHex(ledColorWake);
        responseDoc["color_active"] = crgbToHex(ledColorActive);
        responseDoc["color_parked"] = crgbToHex(ledColorParked);
        responseDoc["color_backup"] = crgbToHex(ledColorBackup);
        responseDoc["color_standby"] = crgbToHex(ledColorStandby);
        responseDoc["led_effect"] = ledEffect_m1;
        responseDoc["brightness"] = activeBrightness;
        responseDoc["brightness_standby"] = sleepBrightness;
        responseDoc["page_color_msg"] = "Current settings are now ACTIVE.";
        //populate effect dropdown
        JsonArray effectArray = responseDoc.createNestedArray("effects");
        for (int i = 0; i < numberOfEffects; i++) {
          JsonObject effectObj = effectArray.createNestedObject();
          effectObj["index"] = i;
          effectObj["effectname"] = Effects[i];
        }
        String responseString;
        serializeJson(responseDoc, responseString);
        server.send(200, "application/json", responseString);
        return;
      }
      if (mode == "distapplybtn") {
        if (uomDistance == 0) {
          float rawDist = doc["dist_wake"].as<float>();
          wakeDistance = inchesToMm(rawDist);
          rawDist = doc["dist_active"].as<float>();
          startDistance = inchesToMm(rawDist);
          rawDist = doc["dist_parked"].as<float>();
          parkDistance = inchesToMm(rawDist);
          rawDist = doc["dist_backup"].as<float>();
          backupDistance = inchesToMm(rawDist);
          rawDist = doc["dist_left"].as<float>();
          leftDistance = ((useSideSensor) ? inchesToMm(rawDist) : 0);
          rawDist = doc["dist_right"].as<float>();
          rightDistance = ((useSideSensor) ? inchesToMm(rawDist) : 0);
        } else {
          wakeDistance = doc["dist_wake"].as<int>();
          startDistance = doc["dist_active"].as<int>();
          parkDistance = doc["dist_parked"].as<int>();
          backupDistance = doc["dist_backup"].as<int>();
          leftDistance = ((useSideSensor) ? doc["dist_left"].as<int>() : 0 );
          rightDistance = ((useSideSensor) ? doc["dist_right"].as<int>() : 0);
        }
        intervalDistance = calculateInterval();
        //Update MQTT Topics
        if (mqttEnabled) {
          forceMqttRefresh();
        }
        //Show confirmation here - just flash green
        FastLED.setBrightness(activeBrightness);
        allLEDsOff(false);  
          fill_solid(LEDs, numLEDs, CRGB::Green); 
          FastLED.show();
          delay(750);
          allLEDsOff(true);

        responseDoc["which_data"] = "dist";
        responseDoc["dist_wake"] = ((uomDistance == 1) ? (float)wakeDistance : mmToInches(wakeDistance));
        responseDoc["dist_active"] = ((uomDistance == 1) ? (float)startDistance : mmToInches(startDistance));
        responseDoc["dist_parked"] = ((uomDistance == 1) ? (float)parkDistance : mmToInches(parkDistance));
        responseDoc["dist_backup"] = ((uomDistance == 1) ? (float)backupDistance : mmToInches(backupDistance));
        if (useSideSensor) {
          responseDoc["dist_left"] = ((uomDistance == 1) ? (float)leftDistance : mmToInches(leftDistance));
          responseDoc["dist_right"] = ((uomDistance == 1) ? (float)rightDistance : mmToInches(rightDistance));
        } else {
          responseDoc["dist_left"] = 0;
          responseDoc["dist_right"] = 0;
        }
        responseDoc["page_dist_msg"] = "Current settings are now ACTIVE.";
        String responseString;
        serializeJson(responseDoc, responseString);
        server.send(200, "application/json", responseString);        
        return;
      }
      //Save & Reboot Buttons
      if ((mode == "colorsavebtn") || (mode == "distsavebtn")) {
        String page;
        mainTestFlag = false;
        page.reserve(strlen(mainsave) + 300);
        page = mainsave;
        if (mode == "colorsavebtn") {
          ledColorWake_dflt = hexToCRGB(doc["color_wake"].as<String>());
          ledColorActive_dflt = hexToCRGB(doc["color_active"].as<String>());
          ledColorParked_dflt = hexToCRGB(doc["color_parked"].as<String>());
          ledColorBackup_dflt = hexToCRGB(doc["color_backup"].as<String>());
          ledColorStandby_dflt = hexToCRGB(doc["color_standby"].as<String>());
          ledEffect_dflt = doc["led_effect"].as<String>();
          defaultBrightness = doc["brightness"];
          sleepBrightness_dflt = doc["brightness_standby"];
        } else {
          if (uomDistance == 0) {
            float rawDist = doc["dist_wake"].as<float>();
            wakeDistance_dflt = inchesToMm(rawDist);
            rawDist = doc["dist_active"].as<float>();
            startDistance_dflt = inchesToMm(rawDist);
            rawDist = doc["dist_parked"].as<float>();
            parkDistance_dflt = inchesToMm(rawDist);
            rawDist = doc["dist_backup"].as<float>();
            backupDistance_dflt = inchesToMm(rawDist);
            rawDist = doc["dist_left"].as<float>();
            leftDistance_dflt = ((useSideSensor) ? inchesToMm(rawDist) : 0);
            rawDist = doc["dist_right"].as<float>();
            rightDistance_dflt = ((useSideSensor) ? inchesToMm(rawDist) : 0);
          } else {
            wakeDistance_dflt = doc["dist_wake"];
            startDistance_dflt = doc["dist_active"];
            parkDistance_dflt = doc["dist_parked"];
            backupDistance_dflt = doc["dist_backup"];
            leftDistance_dflt = ((useSideSensor) ? doc["dist_left"] : 0 );
            rightDistance_dflt = ((useSideSensor) ? doc["dist_right"] : 0);
          }
        }  
        page.replace("*DEVICENAME*", deviceName); 
        page.replace("*VAR_APP_NAME*", APPNAME);
        page.replace("*VAR_CURRENT_VER*", VERSION);
        page.replace("*IPADDR*", baseIP);
        server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        server.send(200, "text/html", "");
        server.sendContent(page);
        server.sendContent(html_footer);
        server.sendContent(""); 
        delay(1000);
        writeConfigFile(true);
        return;
      }
      return;
    }
    server.send(500, "text/plain", "Could not parse data");
  }
}

/*-------------------------
   Hardware Settings Page
  ------------------------- */
void webSystemPage() {
  mainTestFlag = false;
  String page;
  page.reserve(strlen(systempage) + 300);
  page = systempage;
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*VAR_CURRENT_VER*", VERSION);
  page.replace("*IPADDR*", baseIP);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent(""); 
}

void webSystemPageJson() {
  String jsonString;
  JsonDocument doc;
    doc["onboard_led_pin"] = onboardLED_Pin;
    doc["use_onboard_led"] = ((onboardLED_Pin > 0) ? 1 : 0);
    doc["use_boot_leds"] = ((useBootLightsLED) ? 1 : 0);
    doc["led_count"] = numLEDs;
    doc["right_led_wiring"] = ((rightLEDWiring) ? 1 : 0);
    doc["led_data_pin"] = ledData_Pin;
    doc["amps"] = roundf(milliamps / 100.0f) / 10.0f;  // Converts milliamps to amps with 1 decimal
    doc["uom_distance"] = uomDistance;
    doc["tfmini_rx_pin"] = tfminiRX_Pin;
    doc["tfmini_tx_pin"] = tfminiTX_Pin;
    doc["no_car_debounce"] = nocarDetectedCounterMax;
    doc["led_park_time"] = maxOperationTimePark;
    doc["led_exit_time"] = maxOperationTimeExit;
    doc["use_side_sensor"] = ((useSideSensor) ? 1 : 0);
    doc["tof_dat_pin"] = tofDat_Pin; 
    doc["tof_clk_pin"] = tofClk_Pin; 
    doc["side_sensor_pos"] = ((useSideSensor) ? sideSensorPos : 0);
    doc["no_wifi_mode"] = ((noWiFiMode) ? 1 : 0);
    doc["manual_ap_name"] = manualAPName;
    doc["manual_ap_pwd"] = manualAPPwd;
    doc["mqtt_enabled"] = ((mqttEnabled) ? 1 : 0);
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handleSystem() {
  String page;
  page.reserve(strlen(postsystem) + 300);
  page = postsystem;
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    useBootLightsLED = server.arg("bootledsval").toInt();
    useOnboardLED = server.arg("onboardledval").toInt();
    useSideSensor = server.arg("usesidesensorval").toInt();
    noWiFiMode = (server.arg("nowifimodeval").toInt() == 1); 
    onboardLED_Pin = ((useOnboardLED) ? server.arg("onboardpin").toInt() : 0);
    numLEDs = server.arg("numleds").toInt();
    ledData_Pin = server.arg("ledpin").toInt();
    milliamps = (int)roundf(server.arg("amps").toFloat() * 1000.0f);  //Convert amps back to milliamps
    rightLEDWiring = (server.arg("ledwire") == "1");
    uomDistance = server.arg("uom").toInt();          //0=inches, 1=millimeters
    tfminiRX_Pin = server.arg("frontrxpin").toInt();
    tfminiTX_Pin = server.arg("fronttxpin").toInt();
    nocarDetectedCounterMax = server.arg("debounce").toInt();
    maxOperationTimePark = server.arg("parktime").toInt();
    maxOperationTimeExit = server.arg("exittime").toInt();
    if (useSideSensor) {
      tofDat_Pin = server.arg("tofdatpin").toInt();
      tofClk_Pin = server.arg("tofclkpin").toInt();
    }
    sideSensorPos = useSideSensor ? server.arg("sidepos").toInt() : 0;      //0=unused, 1=right, 2=left

    if (noWiFiMode) {
      manualAPName = server.arg("manualapname");
      manualAPName.trim();
      if (manualAPName.length() == 0) {
        manualAPName = deviceName + "_Hotspot";
      }
      manualAPPwd = server.arg("manualappwd");
    } else {
      // Retain/update values if submitted while disabled
      if (server.hasArg("manualapname")) manualAPName = server.arg("manualapname");
      if (server.hasArg("manualappwd")) manualAPPwd = server.arg("manualappwd");
    }
    //Return result page
    page.replace("*DEVICENAME*", deviceName);
    page.replace("*VAR_APP_NAME*", APPNAME);
    page.replace("*VAR_CURRENT_VER*", VERSION);
    page.replace("*IPADDR*", baseIP);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent(page);
    server.sendContent(html_footer);
    server.sendContent(""); 
    delay(1000);
    writeConfigFile(true);  //write configuration file and reboots ESP32
  }
}

/* -------------------
    Calibration Page
   ------------------- */
void webCalibratePage() {
  mainTestFlag = true;
  String page;
  page.reserve(strlen(calibratepage) + 300);
  page = calibratepage;
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*VAR_CURRENT_VER*", VERSION);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent(""); 
}

//--- Function for Calibration Page ---
void getCalibrationData() {
  int16_t frontDistRaw = 0;
  int16_t frontDistMM = 0;
  int16_t frontDistIN = 0;
  int16_t sideDistMM = 0;
  int16_t sideDistIN = 0;
  JsonDocument doc;
  if (tfMiniEnabled) {
    if (tfmini.getData(frontDistRaw)) {
      frontDistMM = frontDistRaw * 10;
      float frontDistInTrue = (frontDistMM / 25.4);
      frontDistIN = round(frontDistInTrue);
    } else {
      frontDistMM = 9999;
      frontDistIN = 999;
    }
    doc["frontmm"] = frontDistMM;
    doc["frontin"] = frontDistIN;
  } else {
    doc["frontmm"] = "N/A";
    doc["frontin"] = "N/A";
  }
  if (useSideSensor) {
    sideDistMM = side_sensor.readRangeSingleMillimeters();
    if ((sideDistMM > 0) && (sideDistMM < 1300)) {
      float sideDistInTrue = (sideDistMM / 25.4);
      sideDistIN = round(sideDistInTrue);
    } else {
      sideDistMM = 9999;
      sideDistIN = 999;
    }
    doc["sidemm"] = sideDistMM;
    doc["sidein"] = sideDistIN;
  } else {
    doc["sidemm"] = "N/A";
    doc["sidein"] = "N/A";
  }
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

/*--------------------
   Integrations Page 
  -------------------- */
void webIntegratePage() {
  mainTestFlag = false;
  String page;
  page.reserve(strlen(integratepage) + 300); 
  page = integratepage;
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*VAR_CURRENT_VER*", VERSION);
  page.replace("*IPADDR*", baseIP);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent(""); 
}

void webIntegratePageJson() {
  loadDiscoveryConfig();
  String jsonString;
  JsonDocument doc;
    doc["mqtt_addr_1"] = mqttAddr_1;
    doc["mqtt_addr_2"] = mqttAddr_2;
    doc["mqtt_addr_3"] = mqttAddr_3;
    doc["mqtt_addr_4"] = mqttAddr_4;
    doc["mqtt_enabled"] = ((mqttEnabled) ? 1 : 0);
    doc["mqtt_port"] = mqttPort;
    doc["mqtt_tele_period"] = mqttTelePeriod;
    doc["mqtt_user"] = mqttUser;
    doc["mqtt_pw"] = mqttPW;
    doc["mqtt_topic_sub"] = mqttTopicSub;
    doc["mqtt_topic_pub"] = mqttTopicPub;
    doc["no_wifi_mode"] = ((noWiFiMode) ? 1 : 0);
    if (mqttEnabled) {
        if (mqttIsConnected()) {
          doc["mqtt_connected"] = 1;
          mqttConnected = true;
        } else {
          doc["mqtt_connected"] = 0;
          mqttConnected = false;
        }
    } else {
      doc["mqtt_connected"] = 0;
      mqttConnected = false;
    }
  // Add Discovery Data
  doc["disc_exists"] = currentDiscovery.exists;
  doc["disc_devname"] = currentDiscovery.deviceName;
  doc["disc_controls"] = currentDiscovery.controls;
  doc["disc_sensors"] = currentDiscovery.sensors;
  doc["disc_config"] = currentDiscovery.config;
  doc["disc_diag"] = currentDiscovery.diagnostics;

  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handleIntegrations() {
  String page;
  page.reserve(strlen(postIntegrations) + 300);
  page = postIntegrations;
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    bool mqttSwitch = server.arg("enablemqttval").toInt();
    if (mqttSwitch && !noWiFiMode) {
      mqttAddr_1 = server.arg("mqttaddr1").toInt();
      mqttAddr_2 = server.arg("mqttaddr2").toInt();
      mqttAddr_3 = server.arg("mqttaddr3").toInt();
      mqttAddr_4 = server.arg("mqttaddr4").toInt();
      mqttPort = server.arg("mqttport").toInt();
      mqttTelePeriod = server.arg("mqttperiod").toInt();
      mqttUser = server.arg("mqttuser");
      mqttPW = server.arg("mqttpw");
      mqttTopicSub = server.arg("mqtttopicsub");
      mqttTopicPub = server.arg("mqtttopicpub");
    } else {
      //Only reset IP address.  Leave other fields "as-is"
      mqttAddr_1 = 0;
      mqttAddr_2 = 0;
      mqttAddr_3 = 0;
      mqttAddr_4 = 0;
      mqttEnabled = false;  //set immediately to prevent any updates
      mqttConnected = false; 
    }
    //return web page
    page.replace("*VAR_APP_NAME*", APPNAME);
    page.replace("*DEVICENAME*", deviceName);
    page.replace("*VAR_CURRENT_VER*", VERSION);
    page.replace("*IPADDR*", baseIP);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent(page);
    server.sendContent(html_footer);
    server.sendContent("");
    delay(1000);
    writeConfigFile(true);
  }
}
//-------------------
// Discovery Publish
//-------------------
void handleDiscoverySave() {
  String action = server.arg("discaction");
  if (action == "delete") {
    publishDiscovery(false);
    deleteDiscoveryConfig();
    //redirect to delete page
    String page;
    page.reserve(strlen(postDiscoveryRemove) + 300);
    page = postDiscoveryRemove;
    page.replace("*VAR_APP_NAME*", APPNAME);
    page.replace("*VAR_CURRENT_VER*", VERSION);
    page.replace("*DEVICENAME*", deviceName);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent(page);
    server.sendContent(html_footer);
    server.sendContent("");
  } else if (action == "save") {
    currentDiscovery.deviceName = server.arg("devicename");
    currentDiscovery.controls = server.hasArg("controls");
    currentDiscovery.sensors = server.hasArg("sensors");
    currentDiscovery.config = server.hasArg("config");
    currentDiscovery.diagnostics = server.hasArg("diagnostics");
    if (saveDiscoveryConfig()) {
      delay(250);
      publishDiscovery(true);
      String page;
      page.reserve(strlen(postDiscoveryAdd) + 300);
      page = postDiscoveryAdd;
      page.replace("*VAR_APP_NAME*", APPNAME);
      page.replace("*VAR_CURRENT_VER*", VERSION);
      page.replace("*DEVICENAME*", deviceName);
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.send(200, "text/html", "");
      server.sendContent(page);
      server.sendContent(html_footer);
      server.sendContent("");    
    } else {
      server.send(500, "text/plain", "Error saving config");
    }
  }
}

void publishDiscovery(bool enable) {
  if (!mqttConnected) return;
  // Initialize filesystem
  if (!LittleFS.begin(false)) return;

  JsonDocument cfg; 
  File file = LittleFS.open("/discovery.json", "r");
  if (file) {
    deserializeJson(cfg, file);
    file.close();
  }
  LittleFS.end();

  // Use "devname" from the selection file; fallback to global deviceName variable
  String discoveryName = cfg["devname"] | deviceName;
  if (discoveryName.length() == 0) discoveryName = deviceName;

  struct DiscoveryEntity {
    const char* group;        // Matches config key ("led", "sensor1", "sensor2", "diag")
    const char* component;  
    const char* suffix;       // Must match the key used in updateMQTT()
    const char* cmd_suffix;   // Must match the key used in processCommand()
    const char* name;  
    const char* category;     
  };
  const DiscoveryEntity list[] = {
    // ------------------------------------
    // CORE LIGHTING CONTROL (group: empty)
    // ------------------------------------
    {"controls", "light", "light", "light", "LED Strip", ""}, 
    {"controls", "switch", "sensoroverride", "sensoroverride", "Sensor Override", ""},

    // ----------------------------------------
    // CONFIGURATION CATEGORY (group: "config")
    // ----------------------------------------
    {"config", "number", "brightnesssleep", "brightnesssleep", "Brightness Standby", "config"},
    {"config", "text", "colorwake", "colorwake", "Color1 Wake", "config"},
    {"config", "text", "coloractive", "coloractive", "Color2 Active", "config"},
    {"config", "text", "colorparked", "colorparked", "Color3 Parked", "config"},
    {"config", "text", "colorbackup", "colorbackup", "Color4 Backup", "config"},
    {"config", "text", "colorstandby", "colorstandby", "Color5 Standby", "config"},
    {"config", "number", "distwake", "distwake", "Dist1 Wake", "config"},
    {"config", "number", "diststart", "diststart", "Dist2 Active", "config"},
    {"config", "number", "distpark", "distpark", "Dist3 Park", "config"},
    {"config", "number", "distbackup", "distbackup", "Dist4 Backup", "config"},
    {"config", "select", "effect", "effect", "LED Effect", "config"},
    {"config", "number", "sidedistleft", "sidedistleft", "Side Dist Left", "config"},
    {"config", "number", "sidedistright", "sidedistright", "Side Dist Right", "config"},
    // -----------------------------------
    // SENSORS CATEGORY (group: "sensors")
    // -----------------------------------
    {"sensors", "binary_sensor", "carpresence", "carpresence", "Car Presence", ""},
    {"sensors", "sensor", "parkdistance", "parkdistance", "Park Distance", ""},
    {"sensors", "sensor", "sidedistance", "sidedistance", "Side Distance", ""},
    {"sensors", "sensor", "zone", "zone", "Zone", ""},
    // ------------------------------------
    // DIAGNOSTICS CATEGORY (group: "diag")
    // ------------------------------------
    {"diag", "sensor", "ipaddress", "ipaddress", "IP Address", "diagnostic"},
    {"diag", "sensor", "macaddress", "macaddress", "MAC Address", "diagnostic"},
    {"diag", "button", "restart", "restart", "Controller Restart", "diagnostic"}
  };
  String cleanMac = strMacAddr;
  cleanMac.replace(":", "");
  // -------------------------------
  // CORE DISCOVERY PUBLISHING LOOP
  // -------------------------------
  for (const auto& ent : list) {
    String comp = String(ent.component);
    String sfx = String(ent.suffix);
    String cmdSfx = String(ent.cmd_suffix);
    String grp = String(ent.group);
    String cat = String(ent.category);

    // uniqueId format: [suffix]_[MAC]
    String uniqueId = sfx + "_" + cleanMac;
    String discoveryTopic = "homeassistant/" + comp + "/" + uniqueId + "/config";

  // Check if the overall group is toggled true in discovery.json
    bool groupActive = cfg[grp] | false;
    // Guard Check: Skip side sensor entirely if the hardware configuration doesn't support them
    if (sfx.indexOf("sidedist") != -1) {
      if (!useSideSensor) {
        // Publish an empty payload to force Home Assistant to clear old Sensor 2 entities if they exist
        client.publish(discoveryTopic.c_str(), "", true);
        continue;
      }
    }
    // Handle structural unpublishing or group disablings cleanly
    if (!enable || !groupActive) {
      client.publish(discoveryTopic.c_str(), "", true);
      continue; 
    }
    // --- Generate Discovery Payload ---
    JsonDocument doc; 
    doc["uniq_id"] = uniqueId;
    doc["name"] = ent.name;
    doc["object_id"] = sfx;            // Locks programmatic entity identity
    doc["has_entity_name"] = true;     // Clean masking of redundant device names

    if (cat.length() > 0) {
      doc["ent_cat"] = cat;
    }

    // Setup Availability and LWT logic
    doc["avty_t"] = "stat/" + mqttTopicPub + "/status";
    doc["pl_avail"] = "online";
    doc["pl_not_avail"] = "offline";

    // Common Device Information Grouping
    JsonObject dev = doc["dev"].to<JsonObject>();
    dev["ids"][0] = cleanMac; 
    dev["name"] = discoveryName;
    dev["mdl"] = "Parking Assistant"; 
    dev["mf"] = "ResinChem Tech";
    dev["cu"] = "http://" + baseIP;
    dev["sw"] = VERSION;

    // Common Origin metadata
    JsonObject origin = doc["origin"].to<JsonObject>();
    origin["name"] = "Parking Assistant";
    origin["url"] = "https://github.com/Resinchem/ESP-Parking-Assistant";


    // --- Component Specific Logic Processing ---
    if (comp == "light") {
      doc["schema"] = "json";
      doc["stat_t"] = "stat/" + mqttTopicPub + "/light";
      doc["cmd_t"] = "cmnd/" + mqttTopicSub + "/light";
      
      doc["brightness"] = true;
      doc["effect"] = false;
      
      // CRITICAL FOR COLOR WHEEL: Declare tracking modes for the JSON Schema
      doc["color_mode"] = true;
      JsonArray clrModes = doc["supported_color_modes"].to<JsonArray>();
      clrModes.add("rgb");

      doc["icon"] = "mdi:led-strip-variant";
    } 
    else if (comp == "text") {
      doc["stat_t"] = "stat/" + mqttTopicPub + "/" + sfx;
      doc["cmd_t"] = "cmnd/" + mqttTopicSub + "/" + cmdSfx;
      doc["icon"] = "mdi:palette"; 
    }
    else if (comp == "select") {
      doc["stat_t"] = "stat/" + mqttTopicPub + "/" + sfx;
      doc["cmd_t"] = "cmnd/" + mqttTopicSub + "/" + cmdSfx;

      JsonArray options = doc["ops"].to<JsonArray>();
      for (int i = 0; i < numberOfEffects; i++) {
        if (Effects[i].length() > 0) {
          options.add(Effects[i]);
        }
      }
    } 
    else if (comp == "number") {
      doc["stat_t"] = "stat/" + mqttTopicPub + "/" + sfx;
      doc["cmd_t"] = "cmnd/" + mqttTopicSub + "/" + cmdSfx;

      if (sfx == "brightnesssleep") {
        doc["min"] = 0;
        doc["max"] = 255;
        doc["step"] = 1;
        doc["mode"] = "slider";
      } else if (sfx.startsWith("sidedist")) {
        doc["mode"] = "box";
        doc["icon"] = "mdi:compare-horizontal";
        if (uomDistance == 1) {    
          //mm
          doc["min"] = 50;
          doc["max"] = 1220;
          doc["step"] = 1;
          doc["unit_of_meas"] = "mm";
        } else {
          //in
          doc["min"] = 2;
          doc["max"] = 48;
          doc["step"] = 0.1;
          doc["unit_of_meas"] = "in";
        }
      } else if (sfx.startsWith("dist")) {
        doc["mode"] = "box";
        doc["icon"] = "mdi:compare-vertical";
        if (uomDistance == 1) {    
          //mm
          doc["min"] = 305;
          doc["max"] = 4980;
          doc["step"] = 1;
          doc["unit_of_meas"] = "mm";
        } else {
          //in
          doc["min"] = 12;
          doc["max"] = 192;
          doc["step"] = 0.1;
          doc["unit_of_meas"] = "in";
        }
      }
    } 
    else if (comp == "sensor") {
      doc["stat_t"] = "stat/" + mqttTopicPub + "/" + sfx;
      if (sfx == "parkdistance" || sfx == "sidedistance") {
        doc["dev_cla"] = "distance";
        doc["unit_of_meas"] = (uomDistance == 1) ? "mm" : "in";
        if (sfx == "parkdistance") {
          doc["icon"] = "mdi:arrow-expand-vertical";
        } else {
          doc["icon"] = "mdi:arrow-expand-horizontal";
        }
      } else if (sfx == "zone") {
        doc["icon"] = "mdi:map-marker-radius";
      }
    }
    else if (grp == "diag") {
      if (comp == "button") {
        doc["cmd_t"] = "cmnd/" + mqttTopicSub + "/restart";
        doc["pl_prs"] = "PRESS";
        doc["dev_cla"] = "restart";
      } else {
        doc["stat_t"] = "stat/" + mqttTopicPub + "/" + sfx;
      }
    } 
    else {
      doc["stat_t"] = "stat/" + mqttTopicPub + "/" + sfx;
      if (comp != "binary_sensor") {
        doc["cmd_t"] = "cmnd/" + mqttTopicSub + "/" + cmdSfx;
      } else {
        if (sfx == "carpresence") {
          doc["dev_cla"] = "presence";
          doc["icon"] = "mdi:car";  
          doc["pl_on"] = "ON";
          doc["pl_off"] = "OFF";
        }
      }
      if (comp == "switch") {
        doc["pl_on"] = "ON";
        doc["pl_off"] = "OFF";
        if (sfx == "sensoroverride") doc["icon"] = "mdi:car-off";
      }
    }
    // Serialize object to buffer string and transmit over MQTT
    String buffer;
    serializeJson(doc, buffer);
    client.publish(discoveryTopic.c_str(), buffer.c_str(), true);
    delay(25); 
  }
  delay(500);
  publishLightJsonState();
}
void publishVersionUpdate() {
// Updates version and IP address ONLY.  Called via setup_mqtt() during boot.  
// Only proceed if connected, discovery is active, and Diagnostics group is enabled
  if (!mqttConnected || !currentDiscovery.exists || !currentDiscovery.diagnostics) return;

  String cleanMac = strMacAddr;
  cleanMac.replace(":", "");

  String discoveryName = currentDiscovery.deviceName.length() > 0 ? currentDiscovery.deviceName : deviceName;
  String uniqueId = "ipaddress_" + cleanMac;
  String discoveryTopic = "homeassistant/sensor/" + uniqueId + "/config";

  JsonDocument doc;
  doc["uniq_id"] = uniqueId;
  doc["name"] = "IP Address";
  doc["object_id"] = "ipaddress";
  doc["has_entity_name"] = true;
  doc["ent_cat"] = "diagnostic";
  doc["stat_t"] = "stat/" + mqttTopicPub + "/ipaddress";

  doc["avty_t"] = "stat/" + mqttTopicPub + "/status";
  doc["pl_avail"] = "online";
  doc["pl_not_avail"] = "offline";

  // Device block updates Version and Web Link on HA Device Card
  JsonObject dev = doc["dev"].to<JsonObject>();
  dev["ids"][0] = cleanMac; 
  dev["name"] = discoveryName;
  dev["mdl"] = "Parking Assistant"; 
  dev["mf"] = "Resinchem Tech";
  dev["cu"] = "http://" + baseIP;
  dev["sw"] = VERSION;

  char buffer[1024];
  serializeJson(doc, buffer);
  client.publish(discoveryTopic.c_str(), buffer, true);
}

/* =======================
    CONTROLLER FUNCTIONS
   ======================= */
// --- Reboot Controller ---
void webRestartPage() {
  String page;
  page.reserve(strlen(restart) + 300);
  page = restart;
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*IPADDR*", baseIP);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent("");
  //If enabled, turn off onboard LED as reboot indicator
  if (useOnboardLED) {
    digitalWrite(onboardLED_Pin, LOW);
  }
  delay(1000);
  ESP.restart();  
}
 // --- Firmware Update ----
void webFirmwareUpdate() {
  mainTestFlag = false;
  String page;
  page.reserve(strlen(updateFirmware) + 500);
  page = updateFirmware;
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*VAR_CURRENT_VER*", VERSION);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent("");
}

void handleWebUpdate() {
  size_t fsize = UPDATE_SIZE_UNKNOWN;
  if (server.hasArg("size")) {
    size_t parsedSize = server.arg("size").toInt();
    if (parsedSize > 0) {
      fsize = parsedSize;
    }
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Update.begin(fsize, U_FLASH);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Update.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    Update.end(true);
  }
}

void handleWebUpdateEnd() {
  server.sendHeader("Connection", "close");
  if (Update.hasError()) {
    server.send(200, "text/plain", "FAIL");
  } else {
    // Send plain text "OK" so the background AJAX listener can intercept it cleanly
    server.send(200, "text/plain", "OK");
    //If enabled, turn off onboard LED as reboot indicator
    if (useOnboardLED) {
      digitalWrite(onboardLED_Pin, LOW);
    }
    delay(1000); // Give the ESP32 chip a moment to flush the response bytes out before shutting down
    ESP.restart();
  }
}
// --- Config Dump ---
void webConfigDump() {
  mainTestFlag = false;
  String page;
  page.reserve(strlen(configpage) + 300);
  page = configpage;
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*IPADDR*", baseIP);
  page.replace("*CONFIGJSON*", getCurrentConfig());
  page.replace("*DISCJSON*", getDiscoveryConfig());
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  //No footer on this page
  server.sendContent("");
}
/* --------------------------------
    Arudino OTA Page and Functions
   -------------------------------- */
void handleOTAUpdate() {
  mainTestFlag = false;
  String page;
  page.reserve(strlen(otapage) + 300);
  page = otapage;
  page.replace("*VAR_APP_NAME*", APPNAME);
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*VAR_CURRENT_VER*", VERSION);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent("");
  //Start OTA Window
  ota_flag = true;
  ota_time = ota_time_window;
  ota_time_elapsed = 0;
}
void handleOTAUpdateMqtt() {
  //enters OTA mode without web page
  //Start OTA Window
  ota_flag = true;
  ota_time = ota_time_window;
  ota_time_elapsed = 0;
}

// --- System Info ---
void handleInfoDump() {
  mainTestFlag = false;
  String page;
  page.reserve(strlen(infodump) + 1000);
  page = infodump;

  page.replace("*DEVICENAME*", deviceName);
  page.replace("*MACADDR*", strMacAddr);
  page.replace("*IPADDR*", baseIP);
  if (noWiFiMode) {
    page.replace("*SSID*", String(manualAPName) + " (AP Mode)");
  } else {
    page.replace("*SSID*", wifiSSID);
  }
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent(""); 
}

void infoDumpJson() {
  String jsonString;
  JsonDocument doc;
  JsonObject sysSettings = doc.createNestedObject("system_settings");
  sysSettings["LED DATA Pin"] = String(ledData_Pin);
  sysSettings["TFMINI RX Pin"] = String(tfminiRX_Pin);
  sysSettings["TFMINI TX Pin"] = String(tfminiTX_Pin);
  sysSettings["ToF Data Pin"]  = (((tofDat_Pin > 0) && (useSideSensor)) ? String(tofDat_Pin) : "Disabled");
  sysSettings["ToF Clock Pin"] = (((tofClk_Pin > 0) && (useSideSensor)) ? String(tofClk_Pin) :"Disabled");
  sysSettings["Onboard LED Pin"]  = ((onboardLED_Pin == 0) ? "Disabled" : String(onboardLED_Pin));
  sysSettings["Number of LEDS"] = String(numLEDs);
  sysSettings["Max Milliamps"]  = String(milliamps);
  sysSettings["LED Wiring"] = ((rightLEDWiring) ? "Right Side" : "Left Side");
  if ((useSideSensor) && (sideSensorPos > 0)) {
    sysSettings["Side Sensor"] = ((sideSensorPos == 1) ? "Right Side" : "Left Side");
  } else {
    sysSettings["Side Sensor"] = "Disabled";
  }
  sysSettings["Unit of Measure"] = ((uomDistance) ? "millimeters" : "inches");
  sysSettings["MQTT"] = ((mqttEnabled) ? "Enabled" : "Disabled");
  sysSettings["Discovery"] = (((currentDiscovery.exists) && (mqttEnabled)) ? "Enabled" : "Disabled");

  JsonArray configVars = doc.createNestedArray("config_vars");
  
  JsonObject row1 = configVars.createNestedObject();
  row1["name"] = "Active Brightness";
  row1["default"] = String(defaultBrightness);
  row1["active"] = String(activeBrightness);

  JsonObject row2 = configVars.createNestedObject();
  row2["name"] = "Standby Brightness";
  row2["default"] = String(sleepBrightness_dflt);
  row2["active"] = String(sleepBrightness);

  JsonObject row3 = configVars.createNestedObject();
  row3["name"] = "Wake Color";
  row3["default"] = crgbToHex(ledColorWake_dflt);
  row3["active"] = crgbToHex(ledColorWake);

  JsonObject row4 = configVars.createNestedObject();
  row4["name"] = "Active Color";
  row4["default"] = crgbToHex(ledColorActive_dflt);
  row4["active"] = crgbToHex(ledColorActive);

  JsonObject row5 = configVars.createNestedObject();
  row5["name"] = "Parked Color";
  row5["default"] = crgbToHex(ledColorParked_dflt);
  row5["active"] = crgbToHex(ledColorParked);

  JsonObject row6 = configVars.createNestedObject();
  row6["name"] = "Backup Color";
  row6["default"] = crgbToHex(ledColorBackup_dflt);
  row6["active"] = crgbToHex(ledColorBackup);

  JsonObject row7 = configVars.createNestedObject();
  row7["name"] = "Standby Color";
  row7["default"] = crgbToHex(ledColorStandby_dflt);
  row7["active"] = crgbToHex(ledColorStandby);

  JsonObject row8 = configVars.createNestedObject();
  row8["name"] = "LED Effect";
  row8["default"] = ledEffect_dflt;
  row8["active"] = ledEffect_m1;

  JsonObject row9 = configVars.createNestedObject();
  row9["name"] = "Wake Zone Dist";
  row9["default"] = ((uomDistance == 1) ? (String(wakeDistance_dflt) + " mm") : (String(mmToInches(wakeDistance_dflt), 1) + " in"));
  row9["active"] = ((uomDistance == 1) ? (String(wakeDistance) + " mm") : (String(mmToInches(wakeDistance), 1) + " in"));

  JsonObject row10 = configVars.createNestedObject();
  row10["name"] = "Active Zone Dist";
  row10["default"] = ((uomDistance == 1) ? (String(startDistance_dflt) + " mm") : (String(mmToInches(startDistance_dflt), 1) + " in"));
  row10["active"] = ((uomDistance == 1) ? (String(startDistance) + " mm") : (String(mmToInches(startDistance), 1) + " in"));

  JsonObject row11 = configVars.createNestedObject();
  row11["name"] = "Park Zone Dist";
  row11["default"] = ((uomDistance == 1) ? (String(parkDistance_dflt) + " mm") : (String(mmToInches(parkDistance_dflt), 1) + " in"));
  row11["active"] = ((uomDistance == 1) ? (String(parkDistance) + " mm") : (String(mmToInches(parkDistance), 1) + " in"));

  JsonObject row12 = configVars.createNestedObject();
  row12["name"] = "Backup Zone Dist";
  row12["default"] = ((uomDistance == 1) ? (String(backupDistance_dflt) + " mm") : (String(mmToInches(backupDistance_dflt), 1) + " in"));
  row12["active"] = ((uomDistance == 1) ? (String(backupDistance) + " mm") : (String(mmToInches(backupDistance), 1) + " in"));

  JsonObject row13 = configVars.createNestedObject();
  row13["name"] = "Side Left Dist";
  if (useSideSensor) {
    row13["default"] = ((uomDistance == 1) ? (String(leftDistance_dflt) + " mm") : (String(mmToInches(leftDistance_dflt), 1)+ "in"));
    row13["active"] = ((uomDistance == 1) ? (String(leftDistance) + " mm") : (String(mmToInches(leftDistance), 1)+ "in"));  
  } else {
    row13["default"] = "Disabled";
    row13["active"] = "Disabled";
  }

  JsonObject row14 = configVars.createNestedObject();
  row14["name"] = "Side Right Dist";
  if (useSideSensor) {
    row14["default"] = ((uomDistance == 1) ? (String(rightDistance_dflt) + " mm") : (String(mmToInches(rightDistance_dflt), 1)+ "in"));;
    row14["active"] = ((uomDistance == 1) ? (String(rightDistance) + " mm") : (String(mmToInches(rightDistance), 1)+ "in")); 
  } else {
    row14["default"] = "Disabled";
    row14["active"] = "Disabled";
  }

  JsonObject row15 = configVars.createNestedObject();
  row15["name"] = "No Car Debounce";
  row15["default"] = String(nocarDetectedCounterMax) + " cycles";
  row15["active"] = "---";

  JsonObject row16 = configVars.createNestedObject();
  row16["name"] = "Active Park Time";
  row16["default"] = String(maxOperationTimePark) + " sec.";
  row16["active"] = "---";

  JsonObject row17 = configVars.createNestedObject();
  row17["name"] = "Active Exit Time";
  row17["default"] = String(maxOperationTimeExit) + " sec.";
  row17["active"] = "---";

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// --- FULL Controller Reset ---
void webResetPage() {
  String page;
  page.reserve(strlen(reset) + 300);
  page = reset;
  page.replace("*DEVICENAME*", deviceName);
  page.replace("*VAR_APP_NAME*", APPNAME);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(page);
  server.sendContent(html_footer);
  server.sendContent("");
  delay(1000);
  //Format LittleFS, wipe WiFi and reboot
  LittleFS.begin();
  LittleFS.format();
  LittleFS.end();
  WiFi.disconnect(false, true);
  ESP.restart();
}
/* ------------------------
    Miscellenous Handlers
   ------------------------ */
// Not found or invalid page handler
void handleNotFound() {
  if (noWiFiMode || onboarding) {
    server.sendHeader("Location", String("http://") + baseIP + "/", true);
    server.send(302, "text/plain", "");
    return;
  }
  String message = "File Not Found or invalid command.\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(404, "text/plain", message);
}

// ===================================
//  SETUP MQTT AND CALLBACKS
// ===================================
bool setup_mqtt() {
  byte mcount = 0;
  espClient.setTimeout(500);
  IPAddress myserver = IPAddress(mqttAddr_1, mqttAddr_2, mqttAddr_3, mqttAddr_4);
  String statusTopic = "stat/" + mqttTopicPub + "/status";
  client.setServer(myserver, mqttPort);
  client.setBufferSize(1024); 
  client.setCallback(callback);
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.print("Connecting to MQTT broker.");
  #endif
  if (useBootLightsLED) {
    fill_solid(LEDs, numLEDs, CRGB::Black);
    FastLED.show();
  }
  while (!client.connected( )) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.print(".");
    #endif
    if (useBootLightsLED) {
        LEDs[mcount] = CRGB::Lime;
        FastLED.show();
    }
    client.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPW.c_str(), statusTopic.c_str(), 1, true, "offline");

    if (mcount >= 10) {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println();
        Serial.println("Could not connect to MQTT broker. MQTT disabled.");
      #endif
      // Could not connect to MQTT broker
      if (useBootLightsLED) {
        //Flash yellow
        for (int i = 0; i < 3; i++) {
          fill_solid(LEDs, numLEDs, CRGB::Yellow);
          FastLED.show();
          delay(250);
          fill_solid(LEDs, numLEDs, CRGB::Black);
          delay(250);
        }
      }
      return false;
    }
    delay(250);
    yield();
    mcount++;
  }
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println();
    Serial.println("Successfully connected to MQTT broker.");
  #endif
  //Publish LWT, current IP and MAC addresses
  String curVer = VERSION;
  client.publish((statusTopic).c_str(), "online", true);  //LWT
  client.publish(("stat/" + mqttTopicPub + "/ipaddr").c_str(), baseIP.c_str(), true);
  client.publish(("stat/" + mqttTopicPub + "/macaddr").c_str(), strMacAddr.c_str(), true);
  client.publish(("stat/" + mqttTopicPub + "/version").c_str(), curVer.c_str(), true);
  client.subscribe(("cmnd/" + mqttTopicSub + "/#").c_str());
  mqttConnected = true;
  //Load and sync discovery - needed for version or IP address changes
  loadDiscoveryConfig();
    if (currentDiscovery.exists) {
      publishVersionUpdate();
    }
  return true;
}

void reconnect() {
  //Attempt to reconnect to MQTT broker. 
  if (!client.connected()) {
    String statusTopic = "stat/" + mqttTopicPub + "/status";
    // Attempt one connection
    if (client.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPW.c_str(), statusTopic.c_str(), 1, true, "offline")) {
      client.publish((statusTopic).c_str(), "online", true);  //LWT
      client.subscribe(("cmnd/" + mqttTopicSub + "/#").c_str());
      mqttConnected = true;
    } else {
      // Set the flag and let the main loop continue.
      mqttConnected = false;
    }
  }
}

bool reconnect_soft() {
  //Attempt quick MQTT reconnect.  If fails, return false instead of forcing ESP Reboot.
  //This should be used instead of full "reconnect" function to just check or re-establish 
  //a connection to a good, known broker.
  if (!client.connected()) {
    unsigned long now = millis();
    String statusTopic = "stat/" + mqttTopicPub + "/status";
    // Only try to connect every 5 seconds to avoid blocking the loop
    if (now - lastMqttRetryAttempt > MQTT_RETRY_DELAY) {
      lastMqttRetryAttempt = now;
      // Try to connect once
      if (client.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPW.c_str(), statusTopic.c_str(), 1, true, "offline")) {
        client.publish((statusTopic).c_str(), "online", true);  //LWT
        client.subscribe(("cmnd/" + mqttTopicSub + "/#").c_str());
        mqttConnected = true;
        return true;
      } else {
        // Failed this attempt, but we return false so the loop() continues
        mqttConnected = false;
        return false;
      }
    }
    return false; // Not time to retry yet
  }
  mqttConnected = true;
  return true;  
}

bool mqttIsConnected() {
  bool retVal = false;
  if (client.connected()) {
    retVal = true;
  } else {
    //attempt a soft reconnect
    retVal = reconnect_soft();
  }
  return retVal;
}

//=========================
// MQTT and API Processing
//=========================

void callback(char* topic, byte* payload, unsigned int length) {
  if (isBooting || mainTestFlag) return;
  if (!mqttIsConnected()) return;
  char* subTopic = strrchr(topic, '/');
  if (subTopic == NULL) return;
  subTopic++;

  char buf[length + 1];
  memcpy(buf, payload, length);
  buf[length] = '\0';
  String val = String(buf);

  // Check if the payload is a unified JSON object (from Home Assistant Light Card)
  if (val.startsWith("{")) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, val);
    if (!error) {
      isMqttJsonProcessing = true;
      
      if (doc.containsKey("state")) {
        String stateStr = doc["state"].as<String>();
        if (stateStr.equalsIgnoreCase("ON")) {
          overrideSensors = true;
          processCommand("ledstate", "ON");
        } else if (stateStr.equalsIgnoreCase("OFF")) {
          // Turn light OFF but leave the master override state unaltered
          processCommand("ledstate", "OFF"); 
        }
      }
      if (doc.containsKey("brightness")) {
        overrideSensors = true;
        processCommand("brightness", doc["brightness"].as<String>());
      }
      if (doc.containsKey("color")) {
        overrideSensors = true;
        JsonObject colorObj = doc["color"];
        if (colorObj.containsKey("r") && colorObj.containsKey("g") && colorObj.containsKey("b")) {
          byte r = colorObj["r"];
          byte g = colorObj["g"];
          byte b = colorObj["b"];
          char hexColor[8];
          sprintf(hexColor, "#%02X%02X%02X", r, g, b);
          processCommand("ledcolor", String(hexColor));
        }
      }
      // Send the unified status back to HA to confirm alignment
      isMqttJsonProcessing = false;
      executeMqttUpdate("sensoroverride");
      updateMQTT("ledall");
    }
    return;
  }
  //Reboot Controller.  
  if (strcmp(subTopic, "restart") == 0) {
    ESP.restart();
    return;  
  }   

  //Force a state refresh (publish) of all MQTT topics
  if ((strcmp(subTopic, "refresh") == 0) || (strcmp(subTopic, "refreshall") == 0)) {
    //payload ignored. Any payload can be passed
    forceMqttRefresh();
    return;
  }
  //Save current settings as new boot defaults and reboot
  if (strcmp(subTopic, "saveconfig") == 0) {
    delay(500); 
    writeConfigFile(true); // This saves and triggers ESP.restart()
    return;
  } 
  //enter Arduino OTA
  if (strcmp(subTopic, "otaupdate") == 0) {
    handleOTAUpdateMqtt();
    return;
  }
  processCommand(String(subTopic), val);  
}

void updateMQTT(String whichField) {
  if (isBooting) return;  //Shield during setup()
  if (isMqttJsonProcessing) return;  //it processing JSON payload, skip individual updates

  if (whichField == "ledall") {
    mqttUpdateMask |= MSK_LEDSTATE;
    mqttUpdateMask |= MSK_LEDCOLOR;
    mqttUpdateMask |= MSK_LEDBRIGHTNESS;
    mqttUpdateMask |= MSK_LIGHT_JSON;
  } else if (whichField == "sensors") {
    mqttUpdateMask |= MSK_CAR_PRESENCE;
    mqttUpdateMask |= MSK_SIDE_DISTANCE;
    mqttUpdateMask |= MSK_PARK_DISTANCE;
    mqttUpdateMask |= MSK_ACTIVE_ZONE;
  }
  else if (whichField == "carpresence")     mqttUpdateMask |= MSK_CAR_PRESENCE;
  else if (whichField == "sidedistance")    mqttUpdateMask |= MSK_SIDE_DISTANCE;
  else if (whichField == "parkdistance")    mqttUpdateMask |= MSK_PARK_DISTANCE;
  else if (whichField == "zone")            mqttUpdateMask |= MSK_ACTIVE_ZONE;
  else if (whichField == "ledstate")        mqttUpdateMask |= MSK_LEDSTATE;
  else if (whichField == "ledcolor")        mqttUpdateMask |= MSK_LEDCOLOR;
  else if (whichField == "ledbrightness")   mqttUpdateMask |= MSK_LEDBRIGHTNESS;
  else if (whichField == "brightnesssleep") mqttUpdateMask |= MSK_SLEEP_BRIGHT;
  else if (whichField == "colorstandby")    mqttUpdateMask |= MSK_COLOR_STANDBY;
  else if (whichField == "colorwake")       mqttUpdateMask |= MSK_COLOR_WAKE;
  else if (whichField == "coloractive")     mqttUpdateMask |= MSK_COLOR_ACTIVE;
  else if (whichField == "colorparked")     mqttUpdateMask |= MSK_COLOR_PARKED;
  else if (whichField == "colorbackup")     mqttUpdateMask |= MSK_COLOR_BACKUP;
  else if (whichField == "colormanual")     mqttUpdateMask |= MSK_COLOR_MANUAL;  
  else if (whichField == "distwake")        mqttUpdateMask |= MSK_DIST_WAKE;
  else if (whichField == "diststart")       mqttUpdateMask |= MSK_DIST_START;
  else if (whichField == "distpark")        mqttUpdateMask |= MSK_DIST_PARK;
  else if (whichField == "distbackup")      mqttUpdateMask |= MSK_DIST_BACKUP;
  else if (whichField == "sidedistleft")    mqttUpdateMask |= MSK_SIDE_DIST_LEFT;
  else if (whichField == "sidedistright")   mqttUpdateMask |= MSK_SIDE_DIST_RIGHT;
  else if (whichField == "sensoroverride")  mqttUpdateMask |= MSK_SENSOR_OVERRIDE;
  else if (whichField == "effect")          mqttUpdateMask |= MSK_LED_EFFECT;
  else if (whichField == "light")           mqttUpdateMask |= MSK_LIGHT_JSON;
}

void handleQueuedMqttUpdates() {
  //Processes queue, with a delay between calls to prvent flooding wifi buffer 
  if (mqttUpdateMask == 0 || !mqttConnected || !client.connected()) return;

  //Priority 1 - sensor states
  if (mqttUpdateMask & MSK_PARK_DISTANCE)       { executeMqttUpdate("parkdistance"); mqttUpdateMask &= ~MSK_PARK_DISTANCE; }
  else if (mqttUpdateMask & MSK_SIDE_DISTANCE)  { executeMqttUpdate("sidedistance"); mqttUpdateMask &= ~MSK_SIDE_DISTANCE; }
  else if (mqttUpdateMask & MSK_ACTIVE_ZONE)    { executeMqttUpdate("zone"); mqttUpdateMask &= ~MSK_ACTIVE_ZONE; }
  else if (mqttUpdateMask & MSK_CAR_PRESENCE)   { executeMqttUpdate("carpresence"); mqttUpdateMask &= ~MSK_CAR_PRESENCE; }
  //Priority 2 - LED State
  else if (mqttUpdateMask & MSK_LEDSTATE)       { executeMqttUpdate("ledstate"); mqttUpdateMask &= ~MSK_LEDSTATE; }
  else if (mqttUpdateMask & MSK_LEDCOLOR)       { executeMqttUpdate("ledcolor"); mqttUpdateMask &= ~MSK_LEDCOLOR; }
  else if (mqttUpdateMask & MSK_LEDBRIGHTNESS)  { executeMqttUpdate("ledbrightness"); mqttUpdateMask &= ~MSK_LEDBRIGHTNESS; }
  else if (mqttUpdateMask & MSK_LIGHT_JSON)     { executeMqttUpdate("light"); mqttUpdateMask &= ~MSK_LIGHT_JSON; } 
  //Priority 3 - Settings (config)
  else if (mqttUpdateMask & MSK_SLEEP_BRIGHT)   { executeMqttUpdate("brightnesssleep"); mqttUpdateMask &= ~MSK_SLEEP_BRIGHT; }
  else if (mqttUpdateMask & MSK_COLOR_STANDBY)  { executeMqttUpdate("colorstandby"); mqttUpdateMask &= ~MSK_COLOR_STANDBY; }
  else if (mqttUpdateMask & MSK_COLOR_WAKE)     { executeMqttUpdate("colorwake"); mqttUpdateMask &= ~MSK_COLOR_WAKE; }
  else if (mqttUpdateMask & MSK_COLOR_ACTIVE)   { executeMqttUpdate("coloractive"); mqttUpdateMask &= ~MSK_COLOR_ACTIVE; }
  else if (mqttUpdateMask & MSK_COLOR_PARKED)   { executeMqttUpdate("colorparked"); mqttUpdateMask &= ~MSK_COLOR_PARKED; }
  else if (mqttUpdateMask & MSK_COLOR_BACKUP)   { executeMqttUpdate("colorbackup"); mqttUpdateMask &= ~MSK_COLOR_BACKUP; }
  else if (mqttUpdateMask & MSK_COLOR_MANUAL)   { executeMqttUpdate("colormanual"); mqttUpdateMask &= ~MSK_COLOR_MANUAL; }
  else if (mqttUpdateMask & MSK_DIST_WAKE)      { executeMqttUpdate("distwake"); mqttUpdateMask &= ~MSK_DIST_WAKE; }
  else if (mqttUpdateMask & MSK_DIST_START)     { executeMqttUpdate("diststart"); mqttUpdateMask &= ~MSK_DIST_START; }
  else if (mqttUpdateMask & MSK_DIST_PARK)      { executeMqttUpdate("distpark"); mqttUpdateMask &= ~MSK_DIST_PARK; }
  else if (mqttUpdateMask & MSK_DIST_BACKUP)    { executeMqttUpdate("distbackup"); mqttUpdateMask &= ~MSK_DIST_BACKUP; }
  else if (mqttUpdateMask & MSK_SIDE_DIST_LEFT) { executeMqttUpdate("sidedistleft"); mqttUpdateMask &= ~MSK_SIDE_DIST_LEFT; }
  else if (mqttUpdateMask & MSK_SIDE_DIST_RIGHT) { executeMqttUpdate("sidedistright"); mqttUpdateMask &= ~MSK_SIDE_DIST_RIGHT; }
  else if (mqttUpdateMask & MSK_SENSOR_OVERRIDE) { executeMqttUpdate("sensoroverride"); mqttUpdateMask &= ~MSK_SENSOR_OVERRIDE; }
  else if (mqttUpdateMask & MSK_LED_EFFECT)     { executeMqttUpdate("effect"); mqttUpdateMask &= ~MSK_LED_EFFECT; }
}

void executeMqttUpdate(String whichField) {
  //Actually published message
  if (!mqttConnected|| !client.connected()) return;
  if (mqttIsConnected()) {
    char topicBuf[128];   // Buffer for the full topic string
    char payloadBuf[32]; // Buffer for the converted numbers/bools
    // -- SENSOR STATE(S) --
    if (whichField == "carpresence") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/carpresence", mqttTopicPub.c_str());
      client.publish(topicBuf, ((carDetected) ? "ON" : "OFF"), true);
      return;
    }
    //Convert all distance to inches if uomDistance == 0
    if (whichField == "parkdistance") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/parkdistance", mqttTopicPub.c_str());
      if (!uomDistance) {
        float tmpDist = mmToInches(tf_dist);
        snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
      } else {
        snprintf(payloadBuf, sizeof(payloadBuf), "%d", tf_dist);
      }
      client.publish(topicBuf, payloadBuf, true);
      return;
    }
    if (whichField == "sidedistance") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/sidedistance", mqttTopicPub.c_str());
      if (useSideSensor) {
        if (!uomDistance) {
          float tmpDist = mmToInches(vl_side_dist);
          snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
        } else {
          snprintf(payloadBuf, sizeof(payloadBuf), "%d", tf_dist);
        }
        client.publish(topicBuf, payloadBuf, true);
      }
       else {
        client.publish(topicBuf, "0", true);
       }
      return;
    }
    if (whichField == "zone") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/zone", mqttTopicPub.c_str());
      client.publish(topicBuf, mqttZone.c_str(), true);
      return;
    }
    // -- LEDs --
    if (whichField == "ledstate") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/ledstate", mqttTopicPub.c_str());
      client.publish(topicBuf, ((ledState) ? "ON" : "OFF"), true);
      return;
    }
    if (whichField == "ledcolor") {
      //Colors published as R,G,B to MQTT for proper use via Home Assistant
      byte r, g, b;
      crgbToRgb(activeLEDColor, r, g, b);
      String mqttColor = String(r) + "," + String(g) + "," + String(b);
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/ledcolor", mqttTopicPub.c_str());
      client.publish(topicBuf, mqttColor.c_str() , true);
      return;
    }    
    if (whichField == "ledbrightness") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/ledbrightness", mqttTopicPub.c_str());
      snprintf(payloadBuf, sizeof(payloadBuf), "%d", activeBrightness);  
      client.publish(topicBuf, payloadBuf, true);
      return;
    }
    if (whichField == "light") {
      publishLightJsonState();
      return;
    }
    // -- Sensor Override --
    if (whichField == "sensoroverride") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/sensoroverride", mqttTopicPub.c_str());
      client.publish(topicBuf, ((overrideSensors) ? "ON" : "OFF"), true);
      return;
    }
    // -- Color and Brightness
    if (whichField == "brightnesssleep") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/brightnesssleep", mqttTopicPub.c_str());
      snprintf(payloadBuf, sizeof(payloadBuf), "%d", sleepBrightness);  
      client.publish(topicBuf, payloadBuf, true);
      return;
    }
    if (whichField == "colorstandby") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/colorstandby", mqttTopicPub.c_str());
      String hexColor = crgbToHex(ledColorStandby);
      client.publish(topicBuf, hexColor.c_str(), true);
      return;
    }
    if (whichField == "colorwake") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/colorwake", mqttTopicPub.c_str());
      String hexColor = crgbToHex(ledColorWake);
      client.publish(topicBuf, hexColor.c_str(), true);
      return;
    }
    if (whichField == "coloractive") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/coloractive", mqttTopicPub.c_str());
      String hexColor = crgbToHex(ledColorActive);
      client.publish(topicBuf, hexColor.c_str(), true);
      return;
    }
    if (whichField == "colorparked") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/colorparked", mqttTopicPub.c_str());
      String hexColor = crgbToHex(ledColorParked);
      client.publish(topicBuf, hexColor.c_str(), true);
      return;
    }
    if (whichField == "colorbackup") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/colorbackup", mqttTopicPub.c_str());
      String hexColor = crgbToHex(ledColorBackup);
      client.publish(topicBuf, hexColor.c_str(), true);
      return;
    }
    if (whichField == "colorstandby") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/colorstandby", mqttTopicPub.c_str());
      String hexColor = crgbToHex(ledColorStandby);
      client.publish(topicBuf, hexColor.c_str(), true);
      return;
    }
    if (whichField == "colormanual") {
      byte r, g, b;
      crgbToRgb(ledColorManual, r, g, b);
      String mqttColor = String(r) + "," + String(g) + "," + String(b);
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/colormanual", mqttTopicPub.c_str());
      client.publish(topicBuf, mqttColor.c_str() , true);
      return;      
    }
    if (whichField == "effect") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/effect", mqttTopicPub.c_str());
      client.publish(topicBuf, ledEffect_m1.c_str(), true);
      return;
    }
    // -- Zone Distances --
    if (whichField == "distwake") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/distwake", mqttTopicPub.c_str());
      if (!uomDistance) {
        float tmpDist = mmToInches(wakeDistance);
        snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
      } else {
        snprintf(payloadBuf, sizeof(payloadBuf), "%d", wakeDistance);
      }
      client.publish(topicBuf, payloadBuf, true);
      return;
    }
    if (whichField == "diststart") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/diststart", mqttTopicPub.c_str());
      if (!uomDistance) {
        float tmpDist = mmToInches(startDistance);
        snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
      } else {
        snprintf(payloadBuf, sizeof(payloadBuf), "%d", startDistance);
      }
      client.publish(topicBuf, payloadBuf, true);
      return;
    }
    if (whichField == "distpark") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/distpark", mqttTopicPub.c_str());
      if (!uomDistance) {
        float tmpDist = mmToInches(parkDistance);
        snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
      } else {
        snprintf(payloadBuf, sizeof(payloadBuf), "%d", parkDistance);
      }
      client.publish(topicBuf, payloadBuf, true);
      return;
    }
    if (whichField == "distbackup") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/distbackup", mqttTopicPub.c_str());
      if (!uomDistance) {
        float tmpDist = mmToInches(backupDistance);
        snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
      } else {
        snprintf(payloadBuf, sizeof(payloadBuf), "%d", backupDistance);
      }
      client.publish(topicBuf, payloadBuf, true);
      return;
    }
    if (whichField == "sidedistleft") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/sidedistleft", mqttTopicPub.c_str());
      if (useSideSensor) {
        if (!uomDistance) {
          float tmpDist = mmToInches(leftDistance);
          snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
        } else {
          snprintf(payloadBuf, sizeof(payloadBuf), "%d", leftDistance);
        }
        client.publish(topicBuf, payloadBuf, true);
      }
       else {
        client.publish(topicBuf, "0", true);
       }
      return;
    }    
    if (whichField == "sidedistright") {
      snprintf(topicBuf, sizeof(topicBuf), "stat/%s/sidedistright", mqttTopicPub.c_str());
      if (useSideSensor) {
        if (!uomDistance) {
          float tmpDist = mmToInches(rightDistance);
          snprintf(payloadBuf, sizeof(payloadBuf), "%.1f", tmpDist); 
        } else {
          snprintf(payloadBuf, sizeof(payloadBuf), "%d", rightDistance);
        }
        client.publish(topicBuf, payloadBuf, true);
      }
       else {
        client.publish(topicBuf, "0", true);
       }
      return;
    } 
  }
}

void handleInitMqttSync() {
  switch (syncStep) {
    case 0: executeMqttUpdate("carpresence"); break;
    case 1: executeMqttUpdate("sidedistance"); break;
    case 2: executeMqttUpdate("parkdistance"); break;
    case 3: executeMqttUpdate("zone"); break;
    case 4: executeMqttUpdate("ledstate"); break;
    case 5: executeMqttUpdate("ledcolor"); break;
    case 6: executeMqttUpdate("ledbrightness"); break;
    case 7: executeMqttUpdate("light"); break;
    case 8: executeMqttUpdate("brightnesssleep"); break;
    case 9: executeMqttUpdate("colorstandby"); break;
    case 10: executeMqttUpdate("colorwake"); break;
    case 11: executeMqttUpdate("coloractive"); break;
    case 12: executeMqttUpdate("colorparked"); break;
    case 13: executeMqttUpdate("colorbackup"); break;
    case 14: executeMqttUpdate("colormanual"); break;
    case 15: executeMqttUpdate("effect"); break;
    case 16: executeMqttUpdate("distwake"); break;
    case 17: executeMqttUpdate("diststart"); break;
    case 18: executeMqttUpdate("distpark"); break;
    case 19: executeMqttUpdate("distbackup"); break;    
    case 20: executeMqttUpdate("sensor_override"); break;
    case 21: executeMqttUpdate("sidedistleft"); break;
    case 22: executeMqttUpdate("sidedistright"); break;
    case 23:
      initialSyncRequired = false; // We are DONE!
      syncStep = -1;
      break;
    default:
      break;
  }
  syncStep++;  
}

void handleLiveMqttUpdates() {
  // Guard Check: Skip completely if disconnected, in test mode, or sensors overridden
  if (!client.connected() || mainTestFlag || overrideSensors) return;

  unsigned long curMillis = millis();
  static unsigned long lastLivePublishTime = 0;
  static bool prevAwakeState = false;
  static String prevMqttZone = "";
  static bool prevLedState = false;
  static CRGB prevColor = CRGB::Black;
  // Determine publication rate: 1 sec when awake, telePeriod when asleep
  unsigned long activeInterval = isAwake ? 1000UL : ((unsigned long)mqttTelePeriod * 1000UL);

  // Force an immediate update if the system JUST woke up or went to sleep
  if ((isAwake != prevAwakeState) || (forceMQTTUpdate)) {
    prevAwakeState = isAwake;
    forceMQTTUpdate = false;
    lastLivePublishTime = 0; // Forces timer condition below to be true immediately
  }

  // Check if interval has elapsed
  if (curMillis - lastLivePublishTime >= activeInterval) {
    lastLivePublishTime = curMillis;

    // Publish all live sensor topics in a single quick pass
    executeMqttUpdate("carpresence");
    executeMqttUpdate("parkdistance");
    executeMqttUpdate("zone");
    if (useSideSensor) {
      executeMqttUpdate("sidedistance");
    }
    //Zone update
    bool isTeleTick = (!isAwake); // Force full telemetry refresh on idle ticks
    if (isTeleTick || (mqttZone != prevMqttZone)) {
      prevMqttZone = mqttZone;
      executeMqttUpdate("zone");
    }
    //Light state, color and HA Light JSON Card
    bool lightChanged = (ledState != prevLedState) || (activeLEDColor != prevColor);
    if (isTeleTick || lightChanged) {
      prevLedState = ledState;
      prevColor = activeLEDColor;

      executeMqttUpdate("ledstate");
      executeMqttUpdate("ledcolor");
      executeMqttUpdate("ledbrightness");
      executeMqttUpdate("light"); // Updates HA Light Entity JSON Card (/light)
    }
  }
}

void publishLightJsonState() {
  if (!client.connected()) return;

  JsonDocument doc;
  doc["state"] = ledState ? "ON" : "OFF";
  doc["brightness"] = activeBrightness;
  doc["color_mode"] = "rgb";

  JsonObject colorObj = doc["color"].to<JsonObject>();
  colorObj["r"] = activeLEDColor.r;
  colorObj["g"] = activeLEDColor.g;
  colorObj["b"] = activeLEDColor.b;

  char buffer[256];
  serializeJson(doc, buffer); 
  
  String stateTopic = "stat/" + mqttTopicPub + "/light";
  client.publish(stateTopic.c_str(), buffer, true);
}

void forceMqttRefresh() {
  //Set flags to update MQTT via queue/loop handler
  syncStep = 0;               // Reset the counter to the first case
  initialSyncRequired = true; // Tell the loop to start the sequence
  lastSyncStepTime = millis(); // Initialize the timer    
}

// ============================
//  Misc MQTT and API Handers
// ============================
void handleAPI() {
  bool hasState = false;
  bool prevLEDState = ledState;
  bool prevOverrideState = overrideSensors;
  bool newState = false;
  bool processResult = false;
  if (mainTestFlag) { server.send(404, "text/plain", "System in calibration mode - command rejected"); return; }
  //Single param commands (cannot be combined)
  //Commands that return values
  if (server.argName(0) == "ipaddress") { server.send(200, "text/plain", baseIP); return; }
  if (server.argName(0) == "macaddress") { server.send(200, "text/plain", strMacAddr); return; }
  //Commands that just return OK or Error
  if (server.argName(0) == "ping") { server.send(200, "text/plain", "OK"); return; }
  if (server.argName(0) == "refresh") { forceMqttRefresh(); server.send(200, "text/plain", "OK"); return; }
  if (server.argName(0) == "apmode") {
    if ((server.arg("apmode") == "1") || (server.arg("apmode").equalsIgnoreCase("on")) || (server.arg("apmode").equalsIgnoreCase("true"))) {
      noWiFiMode = true;
      server.send(200, "text/plain", "OK - APMode ENABLED");
      delay(500);
      writeConfigFile(true);
      return;
    } else if ((server.arg("apmode") == "0") || (server.arg("apmode").equalsIgnoreCase("off")) || (server.arg("apmode").equalsIgnoreCase("false"))) {
      noWiFiMode = false;
      server.send(200, "text/plain", "OK - APMode DISABLED");
      delay(500);
      writeConfigFile(true);
      return;
    }
  } 

  for (int i = 0; i < server.args(); i++) {
    String key = server.argName(i);
    String val = server.arg(i);
    if ((key == "ledstate") || (key == "state")) {
      //Hold this state for application after other params updated
      hasState = true;
      newState = (server.arg("ledstate").equalsIgnoreCase("on") || server.arg("ledstate").equalsIgnoreCase("true") || server.arg("ledstate") == "1");
      processResult = true;
    } else if (processCommand(key, val)) {
      processResult = true;
    } else {
      processResult = false;
      //restore original state and stop further processing
      break;
    }
  }  
  if (processResult) {
    //Now set LED state with all new params
    if (hasState) {
      setLEDState(newState, activeLEDColor);
    } else if (overrideSensors != prevOverrideState) {
      //No action  
    } else if (prevLEDState) {
      setLEDState(true, activeLEDColor);
    } else {
      setLEDState(false);
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(404, "text/plain", "Unknown or Invalid API Command");
  }
}

bool processCommand(String key, String val) {
  //Shared by MQTT and API
  bool recognized = true;
  key.trim(); 
  val.trim();
  if ((key == "sensoroverride") || (key == "overridesensor")) {
    bool disable = ((val.equalsIgnoreCase("on")) || (val == "1") || (val.equalsIgnoreCase("true")));
    overrideAllSensors(disable);  
    updateMQTT("sensoroverride");
  } else if ((key == "brightness") || (key == "ledbrightness")) {
    byte newBright = constrain(val.toInt(), 0, 255);
    setLEDBrightness(newBright);
  } else if ((key == "color") || (key == "ledcolor") || (key == "colormanual")) {
    activeLEDColor = stringToCRGB(val);
    setLEDState(ledState, activeLEDColor);
  } else if ((key == "ledstate") || (key == "state")) {
    bool turnOn = ((val.equalsIgnoreCase("on")) || (val == "1") || (val.equalsIgnoreCase("true")));
    ledState = turnOn;
    setLEDState(turnOn, activeLEDColor);
  //The following only set the active var and do not make any changes to LEDs  
  } else if ((key == "effect") || (key == "ledeffect")) {
    ledEffect_m1 = getEffectName(val);
    intervalDistance = calculateInterval();
    executeMqttUpdate("effect");
  } else if ((key == "brightnesssleep") || (key == "sleepbrightness")) {
    sleepBrightness = constrain(val.toInt(), 0, 255);
    executeMqttUpdate("brightnesssleep");
  } else if (key == "colorwake") {
    ledColorWake = stringToCRGB(val);
    executeMqttUpdate("colorwake");
  } else if (key == "coloractive") {
    ledColorActive = stringToCRGB(val);
    executeMqttUpdate("coloractive");
  } else if (key == "colorparked") {
    ledColorParked = stringToCRGB(val);
    executeMqttUpdate("colorparked");
  } else if (key == "colorbackup") {
    ledColorBackup = stringToCRGB(val);
    executeMqttUpdate("colorbackup");
  } else if (key == "colorstandby") {
    ledColorStandby = stringToCRGB(val);
    executeMqttUpdate("colorstandby");
  } else if (key =="distwake") {
    float rawVal = val.toFloat();
    wakeDistance = (uomDistance == 0) ? constrain(inchesToMm(rawVal), 305, 4980) : constrain((int)rawVal, 305, 4980);
    executeMqttUpdate("distwake");
  } else if ((key =="diststart") || (key == "distactive")) {
    float rawVal = val.toFloat();
    startDistance = (uomDistance == 0) ? constrain(inchesToMm(rawVal), 305, 4980) : constrain((int)rawVal, 305, 4980);
    intervalDistance = calculateInterval();
    executeMqttUpdate("diststart");
  } else if (key =="distpark") {
    float rawVal = val.toFloat();
    parkDistance = (uomDistance == 0) ? constrain(inchesToMm(rawVal), 305, 4980) : constrain((int)rawVal, 305, 4980);
    intervalDistance = calculateInterval();
    executeMqttUpdate("distpark");
  } else if (key =="distbackup") {
    float rawVal = val.toFloat();
    backupDistance = (uomDistance == 0) ? constrain(inchesToMm(rawVal), 305, 4980) : constrain((int)rawVal, 305, 4980);
    executeMqttUpdate("distbackup");
  } else if ((key =="sidedistleft") && (useSideSensor)) {
    float rawVal = val.toFloat();
    leftDistance = (uomDistance == 0) ? constrain(inchesToMm(rawVal), 50, 1220) : constrain((int)rawVal, 50, 1220);
    executeMqttUpdate("sidedistleft");
  } else if ((key =="sidedistright") && (useSideSensor)) {
    float rawVal = val.toFloat();
    rightDistance = (uomDistance == 0) ? constrain(inchesToMm(rawVal), 50, 1220) : constrain((int)rawVal, 50, 1220);
    executeMqttUpdate("sidedistright");
  } else {
    recognized = false;
  }
  return recognized;
}

void overrideAllSensors(bool sensorsOff) {
  //Turn off LEDs if lit whenever changing override
  overrideSensors = sensorsOff;
  if (sensorsOff) {
    allLEDsOff(false);
  } else {
    isAwake = false;
    updateSleepMode();
  }
}
void setLEDState(bool state, CRGB color) {
  if (state) {
    FastLED.setBrightness(activeBrightness);
    fill_solid(LEDs, numLEDs, color);
    FastLED.show();
    ledState = state;
    if (mqttEnabled) updateMQTT("ledall");
  } else {
    allLEDsOff(true);
  }
}
void setLEDBrightness(byte brightness) { 
  //Sets brightness.  If brightness = 0, LEDS will be considered 'OFF', or 'ON' otherwise
  //If setting to '0', the current activeBrightness is stored in lastBrightness for recall (and web slider)
  if (brightness == 0) {
    lastBrightness = activeBrightness;
    activeBrightness = 0;
    ledState = false;
  } else {
    activeBrightness = brightness;
    lastBrightness = activeBrightness;
    ledState = true;
  }
  FastLED.setBrightness(brightness);
  if (ledState) {
    FastLED.show();
  }
  if (mqttConnected) updateMQTT("ledall");
}

String getEffectName(String effect) {
  String retVal = "Solid";
  if (effect.equalsIgnoreCase("out-in")) {
    retVal = "Out-In";
  } else if (effect.equalsIgnoreCase("in-out")) {
    retVal = "In-Out";
  } else if (effect.equalsIgnoreCase("full-strip")) {
    retVal = "Full-Strip";
  } else if (effect.equalsIgnoreCase("full-strip-inv")) {
    retVal = "Full-Strip-Inv";
  }
  return retVal;
}
/* =====================================
    WIFI SETUP 
   =====================================
*/
void setupSoftAP() {
  //for onboarding
  String apName = APPNAME;
  apName.replace(" ", "_");
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println(F("Starting access point for onboarding..."));
  #endif
  // Synchronously flush active states to prevent configuration noise
  WiFi.mode(WIFI_OFF);
  delay(100);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apName + "_AP");
  IPAddress Ip(192, 168, 4, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  dnsServer.start(DNS_PORT, "*", Ip);
  delay(100);
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.print(F("Hotspot active. SSID: "));
    Serial.println(apName + "_AP");
    Serial.print(F("AP IP Address: "));
    Serial.println(WiFi.softAPIP());
  #endif
  server.begin();
}

bool setupWifi() {
  byte count = 0;
  //attempt connection
  //if successful, return true else false
  delay(200);
  WiFi.setSleep(false);
  WiFi.hostname(wifiHostName);
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.print(".");
    #endif
    // Stop if cannot connect
    if (count >= 60) {
      // Could not connect to local WiFi
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
            Serial.println();
            Serial.println("Could not connect to WiFi.");
      #endif
      return false;
    }
    delay(500);
    yield();
    count++;
  }
  //Successfully connected
  wifiSSID = WiFi.SSID();
  baseIP = WiFi.localIP().toString();
  WiFi.macAddress(macAddr);
  strMacAddr = WiFi.macAddress();
  //Create unique client ID for MQTT(in case it is enabled along with another system)
  String cleanMac = strMacAddr;
  cleanMac.replace(":", "");
  if (mqttClient.indexOf("_") == -1) {
    mqttClient = mqttClient + "_" + cleanMac;
  }
  // Start the mDNS responder using variable name
  if (MDNS.begin(wifiHostName.c_str())) {
    // Add HTTP service so discovery apps can see it
    MDNS.addService("http", "tcp", 80);
    
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.print(F("mDNS responder active: http://"));
      Serial.print(wifiHostName);
      Serial.println(F(".local"));
    #endif
  }
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("Connected to wifi... yay!");
    Serial.print("MAC Address: ");
    Serial.println(strMacAddr);
    Serial.print("IP Address: ");
    Serial.println(baseIP);
    Serial.println("Starting main setup...");
  #endif    
  server.begin();
  return true;
}

bool setupManualAP() {
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println(F("Starting Manual Access Point (AP Mode)..."));
  #endif

  WiFi.mode(WIFI_AP);
  
  IPAddress Ip(192, 168, 4, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);

  // Fallback to default name if empty
  if (manualAPName.length() == 0) {
    manualAPName = deviceName + "_Hotspot";
  }

  bool apResult = false;
  if (manualAPPwd.length() >= 8) {
    apResult = WiFi.softAP(manualAPName.c_str(), manualAPPwd.c_str());
  } else {
    apResult = WiFi.softAP(manualAPName.c_str()); // Open network
  }

  if (apResult) {
    baseIP = WiFi.softAPIP().toString();
    strMacAddr = WiFi.softAPmacAddress();
    mqttEnabled = false;
    mqttConnected = false;
    //Start DNS server
    dnsServer.start(DNS_PORT, "*", Ip);

    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.print(F("Manual AP Active. SSID: "));
      Serial.println(manualAPName);
      Serial.print(F("AP IP Address: "));
      Serial.println(baseIP);
    #endif

    server.begin();
    return true;
  } else {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println(F("Failed to start Manual AP Mode!"));
    #endif
    return false;
  }
}
// ===============================
// Calculations and Misc Functions
// ===============================
int calculateInterval() {
  int retVal = 0;
  if ((ledEffect_m1 == "Out-In") || (ledEffect_m1 == "In-Out")) {
    retVal = ((startDistance - parkDistance) / (numLEDs / 2));
  } else if ((ledEffect_m1 == "Full-Strip") || (ledEffect_m1 == "Full-Strip-Inv")) {
    retVal = ((startDistance - parkDistance) / (numLEDs));
  } 
  return retVal;
}

float mmToInches(int mmDistance) {
  //Converts mm distance to inches with one decimal place
  float retVal = roundf((mmDistance / 25.4f) * 10.0f) / 10.0f;
  return retVal;
}

int inchesToMm(float inchDistance) {
  int retVal = (int)roundf(inchDistance * 25.4f);;
  return retVal;
}
// ===========================
//  Color Conversions
// ===========================
bool isValidHex(String hex) {
  if (hex.startsWith("#")) hex = hex.substring(1);
  if (hex.length() != 6) return false;

  for (int i = 0; i < 6; i++) {
    char c = toupper(hex[i]);
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
      return false; // Found a non-hex character like 'Q' or 'W'
    }
  }
  return true;
}

bool splitRGBString(String colorVal, byte &r, byte &g, byte &b) {
  int firstComma = colorVal.indexOf(',');
  int secondComma = colorVal.indexOf(',', firstComma + 1);
  
  if (firstComma != -1 && secondComma != -1) {
    // Extract, convert, and constrain the values directly into the referenced variables
    r = constrain(colorVal.substring(0, firstComma).toInt(), 0, 255);
    g = constrain(colorVal.substring(firstComma + 1, secondComma).toInt(), 0, 255);
    b = constrain(colorVal.substring(secondComma + 1).toInt(), 0, 255);
    return true; // Successfully parsed
  }
  return false; // Invalid format (missing commas)
} 

void hexToRgb(String hexString, byte &r, byte &g, byte &b) {
  //Accepts hex color string (#ff00ff) and returns individual R, G and B values
  if (hexString.startsWith("#")) {
    hexString = hexString.substring(1);
  }
  char charBuf[7];
  hexString.toCharArray(charBuf, sizeof(charBuf));
  long hexValue = strtol(charBuf, NULL, 16);
  r = (hexValue >> 16) & 0xFF;
  g = (hexValue >> 8) & 0xFF;
  b = hexValue & 0xFF;
}

String crgbToHex(CRGB color){
  char hexColor[7];
  sprintf(hexColor, "%02x%02x%02x", color.r, color.g, color.b);
  return "#" + String(hexColor);  
}

void crgbToRgb(CRGB color, byte &r, byte &g, byte &b) {
  // Accepts a FastLED CRGB color and extracts individual R, G, and B bytes
  r = color.r;
  g = color.g;
  b = color.b;
}

String rgbToHex(byte red, byte green, byte blue) {
  //Accepts r, g and b values (0-255) and returns hex color code
  //e.g 255,0,0 will return #ff0000
  char hexColor[7];
  sprintf(hexColor, "%02x%02x%02x", red, green, blue);
  return "#" + String(hexColor);
}

CRGB hexToCRGB(String hexString) {
  // Takes hex value as string (ex. "#ff0000") and converts to CRGB color
  // Check if the string starts with '#' and remove it
  if (hexString.startsWith("#")) {
      hexString = hexString.substring(1);
  }

  // Convert the hex string to a long integer (base 16)
  unsigned long colorValue = strtol(hexString.c_str(), NULL, 16);

  // Create CRGB from a 32-bit integer (which is essentially the hex value)
  return CRGB(colorValue);
}

CRGB rgbToCRGB(byte red, byte green, byte blue) {
  //Accepts r, g and b values (0-255) and returns a FastLED CRGB color value
  return CRGB(red, green, blue);
}

CRGB stringToCRGB(String color) {
  //Used by MQTT and API to accept either HEX or RGB string.  Returns CRGB color
  color.trim();

  // 1. Check if it's an RGB string (contains commas, e.g., "0,255,0")
  if (color.indexOf(',') != -1) {
    byte r = 0, g = 0, b = 0;
    if (splitRGBString(color, r, g, b)) {
      return CRGB(r, g, b);
    }
  } 
  // 2. Otherwise check if it's valid HEX (handles both "#00ff00" and "00ff00")
  else if (isValidHex(color)) {
    return hexToCRGB(color);
  }

  // Fallback for invalid color strings
  return CRGB::Orange;
}
// ===============================
//  LED and Display Functions
// ===============================
void allLEDsOff(bool resumeSleep) {
  fill_solid(LEDs, numLEDs, ledColorOff);   
  FastLED.show();
  ledState = false;
  if ((resumeSleep) && (!overrideSensors)) {
    updateSleepMode();
  } else if (mqttEnabled && client.connected()) {
    updateMQTT("ledall");
  }
}

void blinkLEDs(CRGB color) {
  if (blinkOn) {
    fill_solid(LEDs, numLEDs, color);
  } else {
    fill_solid(LEDs, numLEDs, CRGB::Black);
  }
  blinkOn = !blinkOn;
}

void blinkSideLEDs(CRGB color, byte pos) {
  int startLED = 0;
  int numToLight = (numLEDs * 0.15);
  if (pos == 1) {         //End of strip
    if (rightLEDWiring) {       
      startLED = 0;
    } else {
      startLED = ((numLEDs) - (numLEDs * 0.15)) + 1;
    }
  } else if (pos == 2) {  //Start of strip
    if (rightLEDWiring) {
      startLED = ((numLEDs) - (numLEDs * 0.15)) + 1;
    } else {
      startLED = 0; 
    }
  } 

  if (blinkSideOn) {
    fill_solid(LEDs + startLED, numToLight, color);
  } else {
    fill_solid(LEDs + startLED, numToLight, ledColorActive);
  }
  blinkSideOn = !blinkSideOn;
}

void updateOutIn(int curDistance) {
   byte numberToLight = 1;
  fill_solid(LEDs, numLEDs, CRGB::Black);

  //Get number of LEDs to light up on each end, based on interval
  numberToLight = (startDistance - curDistance) / intervalDistance;
  if (numberToLight ==0 ) numberToLight = 1;  //Assure at least 1 light if integer truncation results in 0
  for (int i=0; i < numberToLight; i++) {
    LEDs[i] = ledColorActive;
    LEDs[(numLEDs-1) - i] = ledColorActive;
  }
}

void updateInOut(int curDistance) {
  byte numberToLight = 1;
  byte startLEDLeft = 0;
  byte startLEDRight = 0;
  fill_solid(LEDs, numLEDs, CRGB::Black);
  //Get number of LEDs to light up on each end, based on interval
  numberToLight = ((startDistance - curDistance) / intervalDistance);
  if (numberToLight ==0 ) numberToLight = 1;  //Assure at least 1 light if integer truncation results in 0
  //Find center LED(s) - single of odd number, two if even number of LEDS
  startLEDLeft = (numLEDs / 2);
  startLEDRight = startLEDLeft;
  if ((startLEDLeft % 2) == 0) {
    startLEDLeft --;
  }
  for (int i=0; i < numberToLight; i++) {
    LEDs[(startLEDLeft - i)] = ledColorActive;
    LEDs[(startLEDRight + i)] = ledColorActive;
  }
}

void updateFullStrip(int curDistance) {
  byte numberToLight = 1;
  fill_solid(LEDs, numLEDs, CRGB::Black);

  //Get number of LEDs to light up from start of LED strip, based on interval
  numberToLight = (startDistance - curDistance) / intervalDistance;
  if (numberToLight == 0 ) numberToLight = 1;  //Assure at least 1 light if integer truncation results in 0
  for (int i=0; i < numberToLight; i++) {
    LEDs[i] = ledColorActive;
  }
}

void updateFullStripInv(int curDistance) {
  byte numberToLight = 1;
  fill_solid(LEDs, numLEDs, CRGB::Black);

  //Get number of LEDs to light up from end of LED strip, based on interval
  numberToLight = (startDistance - curDistance) / intervalDistance;
  if (numberToLight ==0 ) numberToLight = 1;  //Assure at least 1 light if integer truncation results in 0
  for (int i=0; i < numberToLight; i++) {
    LEDs[((numLEDs - i)- 1)] = ledColorActive;
  }
}

void updateSolid(int curDistance) {
  fill_solid(LEDs, numLEDs, CRGB::Black);
  if ((curDistance > startDistance) && (curDistance <= wakeDistance)) {
    fill_solid(LEDs, numLEDs, ledColorWake); 
  } else if ((curDistance > parkDistance) && (curDistance <= startDistance)) {
    fill_solid(LEDs, numLEDs, ledColorActive);
  } else if ((curDistance > backupDistance) && (curDistance <= parkDistance)) {
    fill_solid(LEDs, numLEDs, ledColorParked);
  }
}

void updateSleepMode() {
  fill_solid(LEDs, numLEDs, CRGB::Black);
  FastLED.setBrightness(sleepBrightness);
  if (showStandbyLEDs) {
    LEDs[0] = ledColorStandby;
    LEDs[numLEDs - 1] = ledColorStandby;
  }
  FastLED.show();
  ledState = false;
  if (mqttEnabled && client.connected()) {
    executeMqttUpdate("ledstate");
    executeMqttUpdate("zone");
    executeMqttUpdate("carpresence");
    executeMqttUpdate("parkdistance");
    if (useSideSensor) {
      executeMqttUpdate("sidedistance");
    }
    executeMqttUpdate("light"); // Standardized call for HA JSON payload    
  }
}

void updateOTA() {
  fill_solid(LEDs, numLEDs, CRGB::Black);
  //Alternate LED colors using red and green
  FastLED.setBrightness(activeBrightness);
  for (int i=0; i < (numLEDs-1); i = i + 2) {
    LEDs[i] = CRGB::Red;
    LEDs[i+1] = CRGB::Green;
  }
  FastLED.show();
}
