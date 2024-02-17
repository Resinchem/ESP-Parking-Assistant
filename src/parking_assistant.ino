/*
 * ESP8266 Parking Assistant
 * Includes captive portal and OTA Updates
 * This provides code for an ESP8266 controller for WS2812b LED strips
 * Version: 0.46 - Misc. Fixes and Updates (see release notes)
 * Last Updated: 2/16/2024
 * ResinChem Tech - Released under GNU General Public License v3.0.  There is no guarantee or warranty, either expressed or implied, as to the
 * suitability or utilization of this project, or as to the condition of this project, or whether it will be suitable to the users purposes or needs.
 * Use is solely at the end user's risk.
 */
#include <FS.h>                         //Arduino ESP8266 Core - Handles filesystem functions (read/write config file)
#include <WiFiManager.h>                //https://github.com/tzapu/WiFiManager (must be v2.0.8-beta or later) - Wifi Onboarding with Portal
#include <ESP8266WiFi.h>                //Arudino ESP8266 Core - standard wifi connnectivity
#include <ESP8266mDNS.h>                //https://github.com/mrdunk/esp8266_mdns - Provides mDNS queries and responses (needed for OTA updates)
#include <ESP8266WebServer.h>           //Arduino ESP8266 Core - Provides web server functionalities (handles HTTP requests - also needed for OTA updates)
#include <WiFiUdp.h>                    //Arduino ESP core - provides UDP
#include <ArduinoOTA.h>                 //https://github.com/jandrassy/ArduinoOTA
#include <FastLED.h>                    //https://github.com/FastLED/FastLED - LED functionality
#include <ArduinoJson.h>                //https://github.com/bblanchon/ArduinoJson
#include <WiFiClient.h>                 //Arduino ESP8266 Core - creates a client that can connect to an IP address
#include <ESP8266HTTPUpdateServer.h>    //Arudino ESP8266 Core - needed for OTA Updates
#include <TFMPlus.h>                    //https://github.com/budryerson/TFMini-Plus
#include <PubSubClient.h>               //https://github.com/knolleary/pubsubclient  Provides MQTT functions

#ifdef ESP32
  #include <SPIFFS.h>
#endif
#define VERSION "v0.46-gt (ESP8266)"

// ================================
//  User Defined values and options
// ================================
//  Change default values here. Changing any of these requires a recompile and upload.

#define LED_DATA_PIN 12                     // Pin connected to LED strip DIN
#define WIFIMODE 2                          // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both
#define MQTTMODE 1                          // 0 = Disable MQTT, 1 = Enable (will only be enabled if WiFi mode = 1 or 2 - broker must be on same network)
#define SERIAL_DEBUG 0                      // 0 = Disable (must be disabled if using RX/TX pins), 1 = enable
#define NUM_LEDS_MAX 100                    // For initialization - recommend actual max 50 LEDs if built as shown

bool ota_flag = true;                       // Must leave this as true for board to broadcast port to IDE upon boot
uint16_t ota_boot_time_window = 2500;       // minimum time on boot for IP address to show in IDE ports, in millisecs
uint16_t ota_time_window = 20000;           // time to start file upload when ota_flag set to true (after initial boot), in millsecs
uint16_t ota_time_elapsed = 0;              // Counter when OTA active
uint16_t ota_time = ota_boot_time_window;

//==========================
// LED Setup & Portal Options
//==========================
// Defaults values - these will be set/overwritten by portal or last saved vals on reboot
int numLEDs = 30;        
byte activeBrightness = 100;
byte sleepBrightness = 5;
uint32_t maxOperationTimePark = 60;
uint32_t maxOperationTimeExit = 5;
String ledEffect_m1 = "Out-In";
bool showStandbyLEDs = true;
//New with v4.1 for multi-device support
String deviceName = "parkasst";
String wifiHostName = "parkasst";
String otaHostName = "parkasstOTA";
String mqttClient = "parkasst";


CRGB ledColorOn_m1 = CRGB::White;
CRGB ledColorOff = CRGB::Black;
CRGB ledColorStandby = CRGB::Blue;
CRGB ledColorWake = CRGB::Green;
CRGB ledColorActive = CRGB::Yellow;
CRGB ledColorParked = CRGB::Red;
CRGB ledColorBackup = CRGB::Red;

//Needed for web dropdowns
byte webColorStandby = 3;  //Blue
byte webColorWake = 2;     //Green
byte webColorActive = 1;   //Yellow
byte webColorParked = 0;   //Red
byte webColorBackup = 0;   //Red

//Initial distances for default load (only used for initial onboarding)
byte uomDistance = 0;       // 0=inches, 1=centimeters
int wakeDistance = 3048;    // wake/sleep distance (~10ft)
int startDistance = 1829;   // Start countdown distance (~6')
int parkDistance = 610;     // Final parked distacce (~2')
int backupDistance = 457;   // Flash backup distance (~18")

// ===============================
//  MQTT Variables
// ===============================
//  MQTT will only be used if a server address other than '0.0.0.0' is entered via portal
byte mqttAddr_1 = 0;
byte mqttAddr_2 = 0;
byte mqttAddr_3 = 0;
byte mqttAddr_4 = 0;
int mqttPort = 0;
String mqttUser = "myusername";
String mqttPW = "mypassword";
uint16_t mqttTelePeriod = 60;
uint32_t mqttLastUpdate = 0;
String mqttTopicSub ="parkasst";  //v0.41 (for now, will always be same as pub)
String mqttTopicPub = "parkasst"; //v0.41 

bool mqttEnabled = false;         //Will be enabled/disabled depending on whether a valid IP address is defined in Settings (0.0.0.0 disables MQTT)
bool mqttConnected = false;       //Will be enabled if defined and successful connnection made.  This var should be checked upon any MQTT action.
bool prevCarStatus = false;       //v0.44 for forcing MQTT update on state change
bool forceMQTTUpdate = false;     //v0.44 for forcing MQTT update on state change

//Variables for creating unique entity IDs and topics (HA discovery)
byte macAddr[6];               //Device MAC address (array is in reverse order)
String strMacAddr;             //MAC address as string and in proper order
char uidPrefix[] = "prkast";   //Prefix for unique ID generation
char devUniqueID[30];          //Generated Unique ID for this device (uidPrefix + last 6 MAC characters)

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

// To add a color:
//   - Increase array below (if adding)
//   - Add elements and value in defineColors()
int numberOfColors = 10;
CRGB ColorCodes[10];
String WebColors[10];
// -------------------------------

//OTHER GLOBAL VARIABLES
bool tfMiniEnabled = false;
bool blinkOn = false;
int intervalDistance = 0;
bool carDetected = false;
bool isAwake = false;
bool coldStart = true;

byte carDetectedCounter = 0;
byte carDetectedCounterMax = 3;
byte nocarDetectedCounter = 0;
byte nocarDetectedCounterMax = 10;
byte outOfRangeCounter = 0;
uint32_t startTime;
bool exitSleepTimerStarted = false;
bool parkSleepTimerStarted = false;

//VARIABLES FOR PORTAL USE (JSON vars)
char device_name[18];    //v0.41
char wifi_hostname[18];  //v0.41
char ota_hostname[18];   //v0.41
char led_count[4];
char led_park_time[4];
char led_exit_time[4];
char led_brightness_active[4];
char led_brightness_sleep[4];

char uom_distance[4];
char wake_mils[6];
char start_mils[6];
char park_mils[6];
char backup_mils[6];

char color_wake[4];
char color_active[4];
char color_parked[4];
char color_backup[4];
char color_standby[4];
char led_effect[16];

char mqtt_addr_1[4];
char mqtt_addr_2[4];
char mqtt_addr_3[4];
char mqtt_addr_4[4];
char mqtt_port[6];
char mqtt_user[65];
char mqtt_pw[65];
char mqtt_tele_period[4];
char mqtt_topic_sub[18];   //v0.41
char mqtt_topic_pub[18];   //v0.41

String baseIP;
//---------------------------

WiFiClient espClient;
ESP8266WebServer server;
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  PubSubClient client(espClient);
#endif

TFMPlus tfmini;
CRGB LEDs[NUM_LEDS_MAX];  

//---- Captive Portal -------
//flag for saving data in captive portal
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  shouldSaveConfig = true;
}
//---------------------------

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

// ==============================
//  Define Colors 
// ==============================
//  Increase array size above if adding new
//  Color must be defined as a CRGB::Named Color
void defineColors() {
   ColorCodes[0] = CRGB::Red;
   ColorCodes[1] = CRGB::Yellow;
   ColorCodes[2] = CRGB::Green;
   ColorCodes[3] = CRGB::Blue;
   ColorCodes[4] = CRGB::White;
   ColorCodes[5] = CRGB::HotPink;
   ColorCodes[6] = CRGB::Orange;
   ColorCodes[7] = CRGB::Lime;
   ColorCodes[8] = CRGB::Cyan;
   ColorCodes[9] = CRGB::Gray;
   WebColors[0] = "Red";
   WebColors[1] = "Yellow";
   WebColors[2] = "Green";
   WebColors[3] = "Blue";
   WebColors[4] = "White";
   WebColors[5] = "Pink";
   WebColors[6] = "Orange";
   WebColors[7] = "Lime";
   WebColors[8] = "Cyan";
   WebColors[9] = "Gray";
}

//===============================
// Web pages and handlers
//===============================
// Main Settings page
// Root / Main Settings page handler
void handleRoot() {
  //Convert mm back to inches and round to nearest integer
  uint16_t intWakeDistance = ((wakeDistance / 25.4) + 0.5);
  uint16_t intStartDistance = ((startDistance / 25.4) + 0.5);
  uint16_t intParkDistance = ((parkDistance / 25.4) + 0.5);
  uint16_t intBackupDistance = ((backupDistance / 25.4) + 0.5);
  //If using centimeters, convert from millimeters
  if (uomDistance) {
    intWakeDistance = wakeDistance;
    intStartDistance = startDistance;
    intParkDistance = parkDistance;
    intBackupDistance = backupDistance;
  }
  String mainPage = "<html>\
  <head>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <title>Parking Assistant - Main</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Controller Settings (VAR_DEVICE_NAME)</h1><br>\
    Changes made here will be used <b><i>until the controller is restarted</i></b>, unless the box to save the settings as new boot defaults is checked.<br><br>\
    To test settings, leave the box unchecked and click 'Update'. Once you have settings you'd like to keep, check the box and click 'Update' to write the settings as the new boot defaults.<br><br>\
    If you want to change wifi settings or the device name, you must use the 'Reset All' command.<br><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      <table>\
      <tr>\
      <td><label for=\"leds\">Number of Pixels (1-100):</label></td>\
      <td><input type=\"number\" min=\"1\" max=\"100\" step=\"1\" name=\"leds\" value=\"";
  mainPage += String(numLEDs);    
  mainPage += "\"></td></tr>\
      <tr>\
      <td><label for=\"activebrightness\">Active LED Brightness (0-255):</label></td>\
      <td><input type=\"number\" min=\"0\" max=\"255\" step=\"1\" name=\"activebrightness\" value=\"";
  mainPage += String(activeBrightness);
  mainPage += "\"></td></tr>\
      <tr>\
      <td><label for=\"sleepbrightness\">Standby LED Brightness (0-255) - set to zero to disable:</label></td>\
      <td><input type=\"number\" min=\"0\" max=\"255\" step=\"1\" name=\"sleepbrightness\" value=\"";
  mainPage += String(sleepBrightness);
  mainPage += "\"></td>\
      </tr></table><br>\
      <b><u>LED Active Times</b></u>:<br>\
      This indicates how long the LEDs remain active when going from a no-car to car-detected (park) state or from a car-detected to no-car (exit)s state.<br><br>\
      <table>\
      <tr>\
      <td><label for=\"ledparktime\">Active Park Time (seconds - 300 max):</label></td>\
      <td><input type=\"number\" min=\"0\" max=\"300\" step=\"1\" name=\"ledparktime\" value=\"";
  mainPage += String(maxOperationTimePark);
  mainPage += "\"></td>\
      </tr>\
      <tr>\
      <td><label for=\"ledexittime\">Active Exit Time (seconds - 300 max):</label></td>\
      <td><input type=\"number\" min=\"0\" max=\"300\" step=\"1\" name=\"ledexittime\" value=\"";
  mainPage += String(maxOperationTimeExit);
  mainPage += "\"></td>\
      </tr>\
      </table><br>\
      <b><u>Parking Distances</u></b>:<br>\
      These values, in inches, specify when the LED strip wakes (Wake distance), when the countdown starts (Active distance), when the car is in the desired parked position (Parked distance) or when it has pulled too far forward and should back up (Backup distance).<br><br>\
      See the <a href=\"https://github.com/Resinchem/ESP-Parking-Assistant/wiki/04-Using-the-Web-Interface\">Github wiki</a> for more information on setting these values for your situation. \
      If using inches, you may enter decimal values (e.g. 27.5\") and these will be converted to millimeters in the code.  Values should decrease from Wake through Backup... maximum value is 192 inches (4980 mm) and minimum value is 6 inches (152 mm).<br><br>\
      <table>\
      <tr>\
      <td>Show distances in:</td>\
      <td><input type=\"radio\" id=\"inches\" name=\"uom\" value=\"0\"";
      if (!uomDistance) {
        mainPage += " checked=\"checked\">";
      } else {
        mainPage += ">";
      }
  mainPage += "<label for=\"inches\">Inches</label>&nbsp;&nbsp;\
      <input type=\"radio\" id=\"mm\" name=\"uom\" value=\"1\"";
      if (uomDistance) {
        mainPage += " checked=\"checked\">";
      } else {
        mainPage += ">";        
      }
  mainPage += "<label for=\"mm\">Millimeters</label>&nbsp;&nbsp;\      
      (you must update settings to switch units)</td></tr>\
      <tr>\
      <td><label for=\"wakedistance\">Wake Distance:</label></td>";

  if (uomDistance) {
    mainPage += "<td><input type=\"number\" min=\"152\" max=\"4980\" step=\"1\" name=\"wakedistance\" value=\"";   
    mainPage += String(intWakeDistance); 
    mainPage += "\"> mm</td>";
  } else {
    mainPage += "<td><input type=\"number\" min=\"6\" max=\"192\" step=\"0.1\" name=\"wakedistance\" value=\"";
    mainPage += String(intWakeDistance); 
    mainPage += "\"> inches</td>";
  }

  mainPage += "</tr>\
      <tr>\
      <td><label for=\"activedistance\">Active Distance:</label></td>";
  if (uomDistance) {
    mainPage += "<td><input type=\"number\" min=\"152\" max=\"4980\" step=\"1\" name=\"activedistance\" value=\"";
    mainPage += String(intStartDistance);     
    mainPage += "\"> mm</td>";
  } else {
    mainPage += "<td><input type=\"number\" min=\"6\" max=\"192\" step=\"0.1\" name=\"activedistance\" value=\"";
    mainPage += String(intStartDistance);     
    mainPage += "\"> inches</td>";
  }
  
  mainPage += "</tr>\
      <tr>\
      <td><label for=\"parkeddistance\">Parked Distance:</label></td>";
  if (uomDistance) {
    mainPage += "<td><input type=\"number\" min=\"152\" max=\"4980\" step=\"1\" name=\"parkeddistance\" value=\"";
    mainPage += String(intParkDistance);
    mainPage += "\"> mm</td>";
  } else {
    mainPage += "<td><input type=\"number\" min=\"6\" max=\"192\" step=\"0.1\" name=\"parkeddistance\" value=\"";
    mainPage += String(intParkDistance);
    mainPage += "\"> inches</td>";
  }

  mainPage += "</tr>\
      <tr>\
      <td><label for=\"backupistance\">Backup Distance:</label></td>";
  if (uomDistance) {
    mainPage += "<td><input type=\"number\" min=\"152\" max=\"4980\" step=\"1\" name=\"backupdistance\" value=\"";
    mainPage += String(intBackupDistance);
    mainPage += "\"> mm</td>";
  } else {
    mainPage += "<td><input type=\"number\" min=\"6\" max=\"192\" step=\"0.1\" name=\"backupdistance\" value=\"";
    mainPage += String(intBackupDistance);
    mainPage += "\"> inches</td>";
  }

  mainPage += "</tr>\
      </table><br>\
      <b><u>LED Colors</b></u>:<br>\
      Select LED color to be used for each stage of the parking process.  Colors may be duplicated, but transition from one state to another may not be obvious based on effect chosen.<br><br>\
      <table>\
      <tr>\
      <td><label for=\"wakecolor\">Wake Color:</label></td>\
      <td><select name=\"wakecolor\">";
      for (byte i = 0; i < numberOfColors; i ++) {
        mainPage += "<option value=\"" + String(i) + "\"";
        if (i == webColorWake) {
          mainPage += " selected";
        }
        mainPage += ">" + WebColors[i] + "</option>";
      }
  mainPage += "\"></td>\
      <tr>\
      <td><label for=\"activecolor\">Active Color:</label></td>\
      <td><select name=\"activecolor\">";
      for (byte i = 0; i < numberOfColors; i ++) {
        mainPage += "<option value=\"" + String(i) + "\"";
        if (i == webColorActive) {
          mainPage += " selected";
        }
        mainPage += ">" + WebColors[i] + "</option>";
      }
  mainPage += "\"></td>\
      <tr>\
      <td><label for=\"parkedcolor\">Parked Color:</label></td>\
      <td><select name=\"parkedcolor\">";
      for (byte i = 0; i < numberOfColors; i ++) {
        mainPage += "<option value=\"" + String(i) + "\"";
        if (i == webColorParked) {
          mainPage += " selected";
        }
        mainPage += ">" + WebColors[i] + "</option>";
      }
  mainPage += "\"></td>\
      <tr>\
      <td><label for=\"backupcolor\">Backup Color (flashing):</label></td>\
      <td><select name=\"backupcolor\">";
      for (byte i = 0; i < numberOfColors; i ++) {
        mainPage += "<option value=\"" + String(i) + "\"";
        if (i == webColorBackup) {
          mainPage += " selected";
        }
        mainPage += ">" + WebColors[i] + "</option>";
      }
  mainPage += "\"></td>\
      <tr>\
      <td><label for=\"standbycolor\">Standby Color:</label></td>\
      <td><select name=\"standbycolor\">";
      for (byte i = 0; i < numberOfColors; i ++) {
        mainPage += "<option value=\"" + String(i) + "\"";
        if (i == webColorStandby) {
          mainPage += " selected";
        }
        mainPage += ">" + WebColors[i] + "</option>";
      }
  mainPage += "\"></td>\
      </tr>\
      <tr>\
      <td><label for=\"effect\">Effect:</label></td>\
      <td><select name=\"effect1\">";
 // Dropdown Effects boxes
 for (byte i = 0; i < numberOfEffects; i++) {
   mainPage += "<option value=\"" + Effects[i] + "\"";
   if (Effects[i] == ledEffect_m1) {
     mainPage += " selected";
   }
   mainPage += ">" + Effects[i] + "</option>";
 }

 //MQTT Section
  mainPage += "</td></tr>\
      </table><br>\
      <b><u>MQTT Settings</u></b>:<br>\
      ONLY enter this information if you already have an MQTT broker configured. <u><i>To disable or remove MQTT functionality, set the IP address to 0.0.0.0</i></u><br>\
      Any changes to MQTT require that you check the box to update boot settings below, which will reboot the controller.  If a successful connection to your MQTT broker is made, \
      a retained message of \"connected\" will be published to the topic \"stat/your_topic/mqtt\".<br><br>";
  mainPage += "<table>\
      <tr>\
      <td><label for=\"mqttaddr1\">Broker IP Address:</label></td>\
      <td><input type=\"number\" min=\"0\" max=\"255\" step=\"1\" name=\"mqttaddr1\" style=\"width: 50px\;\" value=\"";
  mainPage += String(mqttAddr_1);
  mainPage += "\">.<input type=\"number\" min=\"0\" max=\"255\" step=\"1\" name=\"mqttaddr2\" style=\"width: 50px\;\" value=\"";  
  mainPage += String(mqttAddr_2);
  mainPage += "\">.<input type=\"number\" min=\"0\" max=\"255\" step=\"1\" name=\"mqttaddr3\" style=\"width: 50px\;\" value=\"";  
  mainPage += String(mqttAddr_3);
  mainPage += "\">.<input type=\"number\" min=\"0\" max=\"255\" step=\"1\" name=\"mqttaddr4\" style=\"width: 50px\;\" value=\"";  
  mainPage += String(mqttAddr_4);
  mainPage += "\"></td></tr>\
      <tr>\
      <td><label for=\"mqttport\">MQTT Broker Port:</label></td>\
      <td><input type=\"number\" min=\"0\" max=\"65535\" step=\"1\" name=\"mqttport\" style=\"width: 65px\;\" value=\"";
  mainPage += String(mqttPort);
  mainPage += "\"></td></tr>\
      <tr>\
      <td><label for=\"mqttuser\">MQTT User Name:</label></td>\
      <td><input type=\"text\" name=\"mqttuser\" maxlength=\"64\" value=\"";
  mainPage += mqttUser;
  mainPage += "\"></td></tr>\
      <tr>\
      <td><label for=\"mqttpw\">MQTT Password:</label></td>\
      <td><input type=\"password\" name=\"mqttpw\" maxlength=\"64\" value=\"";
  mainPage += mqttPW;
  mainPage += "\"></td></tr>\
      <tr>\
      <td><label for=\"mqtttopic\">MQTT Topic: &nbsp;&nbsp; stat/</label></td>\
      <td><input type=\"text\" name=\"mqtttopic\" maxlength=\"16\" value=\"";
  mainPage += mqttTopicPub;

  mainPage += "\"> (16 alphanumeric chars max - no spaces, no symbols)</td></tr>\
      <tr>\
      <td><label for=\"mqttperiod\">Telemetry Period:</label></td>\
      <td><input type=\"number\" min=\"60\" max=\"600\" step=\"1\" name=\"mqttperiod\" style=\"width: 50px\;\" value=\"";
  mainPage += String(mqttTelePeriod);
  mainPage += "\"> seconds (60 min, 600 max)</td></tr>\
      <tr>\
      <td><label for=\"discovery\">MQTT Discovery:</label></td>\
      <td><a href=\"http://";
  mainPage += baseIP;
  mainPage += "/discovery\">Configure Home Assistant MQTT Discovery</a> (beta)";    
  mainPage += "</td></tr>\
      </table><br>\
      <input type=\"checkbox\" name=\"chksave\" value=\"save\">Save all settings as new boot defaults (controller will reboot)<br><br>\
      <input type=\"submit\" value=\"Update\">\
    </form>\
    <br>\
    <h2>Controller Commands</h2>\
    Caution: Restart and Reset are executed immediately when the button is clicked.<br>\
    <table border=\"1\" cellpadding=\"10\">\
    <tr>\
    <td><button id=\"btnrestart\" onclick=\"location.href = './restart';\">Restart</button></td><td>This will reboot controller and reload default boot values.</td>\
    </tr><tr>\
    <td><button id=\"btnreset\" style=\"background-color:#FAADB7\" onclick=\"location.href = './reset';\">RESET ALL</button></td><td><b>WARNING</b>: This will clear all settings, including WiFi! You must complete initial setup again.</td>\
    </tr><tr>\
    <td><button id=\"btnupdate\" onclick=\"location.href = './update';\">Firmware Upgrade</button></td><td>Upload and apply new firmware from local file.</td>\
    </tr></table><br>\
    Current version: VAR_CURRRENT_VER\
  </body>\
</html>";
  mainPage.replace("VAR_DEVICE_NAME", deviceName); 
  mainPage.replace("VAR_CURRRENT_VER", VERSION);
  server.send(200, "text/html", mainPage);
}

// Settings submit handler - Settings results
void handleForm() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    String saveSettings;
    webColorWake = server.arg("wakecolor").toInt();
    webColorActive = server.arg("activecolor").toInt();
    webColorParked = server.arg("parkedcolor").toInt();
    webColorBackup = server.arg("backupcolor").toInt();
    webColorStandby = server.arg("standbycolor").toInt();
    
    numLEDs = server.arg("leds").toInt();
    activeBrightness = server.arg("activebrightness").toInt();
    sleepBrightness = server.arg("sleepbrightness").toInt();
    if (sleepBrightness == 0) {
      showStandbyLEDs = false;
    } else {
      showStandbyLEDs = true;
    }
    maxOperationTimePark = server.arg("ledparktime").toInt();
    maxOperationTimeExit = server.arg("ledexittime").toInt();
    
    byte tmpUOM = server.arg("uom").toInt();
    if (tmpUOM != uomDistance) {
      //UOM has changed. Convert distances
      if (tmpUOM) {
        //Was inches... convert to mm
        wakeDistance = ((server.arg("wakedistance").toInt()) * 25.4);  
        startDistance = ((server.arg("activedistance").toInt()) * 25.4);
        parkDistance = ((server.arg("parkeddistance").toInt()) * 25.4);
        backupDistance = ((server.arg("backupdistance").toInt()) * 25.4);
       
      } else {
        //Was mm... just get server val
        wakeDistance = (server.arg("wakedistance").toInt());
        startDistance = (server.arg("activedistance").toInt());
        parkDistance = (server.arg("parkeddistance").toInt());
        backupDistance = (server.arg("backupdistance").toInt());
      }
    } else {
      //no uom change... just set to server arg value
      if (uomDistance) {
        wakeDistance = (server.arg("wakedistance").toInt());
        startDistance = (server.arg("activedistance").toInt());
        parkDistance = (server.arg("parkeddistance").toInt());
        backupDistance = (server.arg("backupdistance").toInt());
      } else {
        //convert all values to mm
        wakeDistance = ((server.arg("wakedistance").toInt()) * 25.4);  
        startDistance = ((server.arg("activedistance").toInt()) * 25.4);
        parkDistance = ((server.arg("parkeddistance").toInt()) * 25.4);
        backupDistance = ((server.arg("backupdistance").toInt()) * 25.4);
      }
    }
    
    uomDistance = server.arg("uom").toInt();
    //for displaying inches on page
    uint16_t intWakeDistance = ((wakeDistance / 25.4) + 0.5);
    uint16_t intStartDistance = ((startDistance / 25.4) + 0.5);
    uint16_t intParkDistance = ((parkDistance / 25.4) + 0.5);
    uint16_t intBackupDistance = ((backupDistance / 25.4) + 0.5);
    
    ledColorWake = ColorCodes[webColorWake];
    ledColorActive = ColorCodes[webColorActive];
    ledColorParked = ColorCodes[webColorParked];
    ledColorBackup = ColorCodes[webColorBackup];
    ledColorStandby = ColorCodes[webColorStandby];
    
    ledEffect_m1 = server.arg("effect1");
    
    mqttAddr_1 = server.arg("mqttaddr1").toInt();
    mqttAddr_2 = server.arg("mqttaddr2").toInt();
    mqttAddr_3 = server.arg("mqttaddr3").toInt();
    mqttAddr_4 = server.arg("mqttaddr4").toInt();
    mqttPort = server.arg("mqttport").toInt();
    mqttUser = server.arg("mqttuser");
    mqttPW = server.arg("mqttpw");
    mqttTelePeriod = server.arg("mqttperiod").toInt();
    mqttTopicSub = server.arg("mqtttopic");
    mqttTopicPub = server.arg("mqtttopic");
    
    saveSettings = server.arg("chksave");
    
    String message = "<html>\
      </head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>Parking Assistant - Current Settings</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
      <H1>Settings updated!</H1><br>\
      <H3>Current values are:</H3>";
    message += "Device Name: " + deviceName + "<br>";
    message += "Num LEDs: " + server.arg("leds") + "<br>";
    message += "Active Brightness: " + server.arg("activebrightness") + "<br>";
    message += "Standby Brightness: " + server.arg("sleepbrightness") + "<br><br>";
    message += "<b>LED active On Times</b><br><br>";
    message += "Park Time: " + server.arg("ledparktime") + " secs.<br>";
    message += "Exit Time: " + server.arg("ledexittime") + " secs.<br><br>";
    if (uomDistance) {
      message += "<b>Parking Distances milimeters (inches)</b><br><br>";
      message += "Wake Distance: " + String(wakeDistance) + " (" + String(intWakeDistance) + ")<br>";
      message += "Active Distance: " + String(startDistance) + " (" + String(intStartDistance) + ")<br>";
      message += "Parked Distance: " + String(parkDistance) + " (" + String(intParkDistance) + ")<br>";
      message += "Backup Distance: " + String(backupDistance) + " (" + String(intBackupDistance) + ")<br><br>";
      
    } else {
      message += "<b>Parking Distances inches (millimeters)</b><br><br>";
      message += "Wake Distance: " + String(intWakeDistance) + " (" + String(wakeDistance) + ")<br>";
      message += "Active Distance: " + String(intStartDistance) + " (" + String(startDistance) + ")<br>";
      message += "Parked Distance: " + String(intParkDistance) + " (" + String(parkDistance) + ")<br>";
      message += "Backup Distance: " + String(intBackupDistance) + " (" + String(backupDistance) + ")<br><br>";
//      message += "Backup Distance: " + server.arg("backupdistance") + " (" + String(backupDistance) + ")<br><br>";
      
    }
    message += "<b>LED Colors and Effect</b>:<br><br>";
    message += "Wake Color: " + WebColors[webColorWake] + "<br>";
    message += "Active Color: " + WebColors[webColorActive] + "<br>";
    message += "Parked Color: " + WebColors[webColorParked] + "<br>";
    message += "Backup Color: " + WebColors[webColorBackup] + "<br>";
    message += "Standby Color: " + WebColors[webColorStandby] +  "<br>";
    message += "Effect: " + server.arg("effect1") + "<br><br>";
    message += "<b>MQTT Settings</b><br><br>";
    if ((mqttAddr_1 == 0) && (mqttAddr_2 == 0) && (mqttAddr_3) == 0 && (mqttAddr_4 == 0)) {
      message += "MQTT: <b><u>Disabled</u></b> ";
      if (saveSettings != "save") {
        message += "(If you just changed MQTT settings, you must save as new boot defaults for this to take effect)<br>";
      }
    } else {
      message += "MQTT Server: " + server.arg("mqttaddr1") + "." + server.arg("mqttaddr2") +  "." + server.arg("mqttaddr3") + "." + server.arg("mqttaddr4") + "<br>";
      message += "MQTT Port: " + server.arg("mqttport") + "<br>";
      message += "MQTT User: " + server.arg("mqttuser") + "<br>";
      message += "MQTT Password: ***********<br>";
      message += "MQTT Topic: stat/" + server.arg("mqtttopic") + "<br>";
      message += "Telemetry Period: " + server.arg("mqttperiod") + " seconds<br>";
      if (saveSettings != "save") {
        message += "<br>(If you just changed MQTT settings, you must save as new boot defaults for this to take effect)<br>";
      }
    }
    message += "<br>";
    if (saveSettings == "save") {
      message += "<br>";
      message += "<b>New settings saved as boot defaults.</b> Controller will now reboot.<br>";
      message += "You can return to the settings page after boot completes (lights will briefly turn blue then red/green to indicate completed boot).<br>";    
    } else {
      //Wake up system so new setting can be seen/tested... even if car present
      carDetectedCounter = carDetectedCounterMax + 1;
    }
    message += "<br><a href=\"http://";
    message += baseIP;
    message += "\">Return to settings</a><br>";
    message += "</body></html>";
    server.send(200, "text/html", message);
    delay(1000);
    if (saveSettings == "save") {
      updateSettings(true);
    } else {
      updateSettings(false);
    } 
  }
}

// Firmware update handler
void handleUpdate() {
  String updFirmware = "<html>\
      </head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>Parking Assistant - Firmware Update</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
      <H1>Firmware Update</H1>\
      <H3>Current firmware version: ";
  updFirmware += VERSION;
  updFirmware += "</H3><br>";
  updFirmware += "Notes:<br>";
  updFirmware += "<ul>";
  updFirmware += "<li>The firmware update will begin as soon as the Update Firmware button is clicked.</li><br>";
  updFirmware += "<li>Your current settings will be retained.</li><br>";
  updFirmware += "<li><b>Please be patient!</b> The update will take a few minutes.  Do not refresh the page or navigate away.</li><br>";
  updFirmware += "<li>If the upload is successful, a brief message will appear and the controller will reboot.</li><br>";
  updFirmware += "<li>After rebooting, you'll automatically be taken back to the main settings page and the update will be complete.</li><br>";
  updFirmware += "</ul><br>";
  updFirmware += "</body></html>";    
  updFirmware += "<form method='POST' action='/update2' enctype='multipart/form-data'>";
  updFirmware += "<input type='file' accept='.bin,.bin.gz' name='Select file' style='width: 300px'><br><br>";
  updFirmware += "<input type='submit' value='Update Firmware'>";
  updFirmware += "</form><br>";
  updFirmware += "<br><a href=\"http://";
  updFirmware += baseIP;
  updFirmware += "\">Return to settings</a><br>";
  updFirmware += "</body></html>";
  server.send(200, "text/html", updFirmware); 
}

void updateSettings(bool saveBoot) {
  // This updates the current local settings for current session only.  
  // Will be overwritten with reboot/reset/OTAUpdate
  if (saveBoot) {
    updateBootSettings(saveBoot);
  } else {
    //Update FastLED with new brightness values if changed
    if (isAwake) {
      FastLED.setBrightness(activeBrightness);
    } else {
      FastLED.setBrightness(sleepBrightness);
    }
    // Set interval distance based on current Effect if changed
    intervalDistance = calculateInterval();
  }
}


void updateBootSettings(bool restart_ESP) {
  // Writes new settings to SPIFFS (new boot defaults)
  char t_led_count[4];
  char t_led_brightness_active[4];
  char t_led_brightness_sleep[4];
  char t_led_park_time[4];
  char t_led_exit_time[4];
  char t_uom_distance[4];
  char t_wake_mils[6];
  char t_start_mils[6];
  char t_park_mils[6];
  char t_backup_mils[6];
  char t_color_standby[4];
  char t_color_wake[4];
  char t_color_active[4];
  char t_color_parked[4];
  char t_color_backup[4];
  char t_led_effect[16];
  int eff_len = 16;
  char t_mqtt_addr_1[4];
  char t_mqtt_addr_2[4];
  char t_mqtt_addr_3[4];
  char t_mqtt_addr_4[4];
  char t_mqtt_port[6];
  char t_mqtt_user[65];
  char t_mqtt_pw[65];
  char t_mqtt_tele_period[4];
  int user_len = 65;
  int user_pw = 65;
  //v0.41 new values
  char t_device_name[18];
  char t_mqtt_topic_sub[18];
  char t_mqtt_topic_pub[18];
  int dev_name_len = 18;
  int topic_len = 18;
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("Attempting to update boot settings");
  #endif

  //Convert values into char arrays
  sprintf(t_led_count, "%u", numLEDs);
  sprintf(t_led_brightness_active, "%u", activeBrightness);
  sprintf(t_led_brightness_sleep, "%u", sleepBrightness);
  sprintf(t_led_park_time, "%u", maxOperationTimePark);
  sprintf(t_led_exit_time, "%u", maxOperationTimeExit);
  sprintf(t_uom_distance, "%u", uomDistance);
  sprintf(t_wake_mils, "%u", wakeDistance);
  sprintf(t_start_mils, "%u", startDistance);
  sprintf(t_park_mils, "%u", parkDistance);
  sprintf(t_backup_mils, "%u", backupDistance);
  sprintf(t_color_standby, "%u", webColorStandby);
  sprintf(t_color_wake, "%u", webColorWake);
  sprintf(t_color_active, "%u", webColorActive);
  sprintf(t_color_parked, "%u", webColorParked);
  sprintf(t_color_backup, "%u", webColorBackup);
  ledEffect_m1.toCharArray(t_led_effect, eff_len);
  sprintf(t_mqtt_addr_1, "%u", mqttAddr_1);
  sprintf(t_mqtt_addr_2, "%u", mqttAddr_2);
  sprintf(t_mqtt_addr_3, "%u", mqttAddr_3);
  sprintf(t_mqtt_addr_4, "%u", mqttAddr_4);
  sprintf(t_mqtt_port, "%u", mqttPort);
  sprintf(t_mqtt_tele_period, "%u", mqttTelePeriod);
  mqttUser.toCharArray(t_mqtt_user, user_len);
  mqttPW.toCharArray(t_mqtt_pw, user_pw);
  deviceName.toCharArray(t_device_name, dev_name_len);
  mqttTopicSub.toCharArray(t_mqtt_topic_sub, topic_len);
  mqttTopicPub.toCharArray(t_mqtt_topic_pub, topic_len);

#ifdef ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif

    json["led_count"] = t_led_count;
    json["led_brightness_active"] = t_led_brightness_active;
    json["led_brightness_sleep"] = t_led_brightness_sleep;
    json["led_park_time"] = t_led_park_time;
    json["led_exit_time"] = t_led_exit_time;
    json["uom_distance"] = t_uom_distance;
    json["wake_mils"] = t_wake_mils;
    json["start_mils"] = t_start_mils;
    json["park_mils"] = t_park_mils;
    json["backup_mils"] = t_backup_mils;
    json["color_standby"] = t_color_standby;
    json["color_wake"] = t_color_wake;
    json["color_active"] = t_color_active;
    json["color_parked"] = t_color_parked;
    json["color_backup"] = t_color_backup;
    json["led_effect"] = t_led_effect;
    json["mqtt_addr_1"] = t_mqtt_addr_1;
    json["mqtt_addr_2"] = t_mqtt_addr_2;
    json["mqtt_addr_3"] = t_mqtt_addr_3;
    json["mqtt_addr_4"] = t_mqtt_addr_4;
    json["mqtt_port"] = t_mqtt_port;
    json["mqtt_tele_period"] = t_mqtt_tele_period;
    json["mqtt_user"] = t_mqtt_user;
    json["mqtt_pw"] = t_mqtt_pw;
    //v0.41
    json["device_name"] = t_device_name;
    json["mqtt_topic_sub"] = t_mqtt_topic_sub;
    json["mqtt_topic_pub"] = t_mqtt_topic_pub;

    if (SPIFFS.begin()) {
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.println("failed to open config file for writing");
        #endif
      }
      serializeJson(json, Serial);
      serializeJson(json, configFile);
      configFile.close();
        //end save
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("Boot settings saved. Rebooting controller.");
      #endif
  }
  SPIFFS.end();
  if (restart_ESP) {   //Needed so initial onboarding doesn't cause second reboot
    ESP.restart();
  }
}

void handleReset() {
    String resetMsg = "<HTML>\
      </head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>Controller Reset</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
      <H1>Controller Resetting...</H1><br>\
      <H3>After this process is complete, you must setup your controller again:</H3>\
      <ul>\
      <li>Connect a device to the controller's local access point: ESP_ParkingAsst</li>\
      <li>Open a browser and go to: 192.168.4.1</li>\
      <li>Enter your WiFi information and set other default settings values</li>\
      <li>Click Save. The controller will reboot and join your WiFi</li>\
      </ul><br>\
      Once the above process is complete, you can return to the main settings page by rejoining your WiFi and entering the IP address assigned by your router in a browser.<br>\
      You will need to reenter all of your settings for the system as all values will be reset to original defaults<br><br>\
      <b>This page will NOT automatically reload or refresh</b>\
      </body></html>";
    server.send(200, "text/html", resetMsg);
    delay(1000);
    SPIFFS.begin();
    SPIFFS.format();
    SPIFFS.end();
    wifiManager.resetSettings();
    delay(1000);
    ESP.restart();
}

void handleRestart() {
    String restartMsg = "<HTML>\
      </head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>Controller Restart</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
      <H1>Controller restarting...</H1><br>\
      <H3>Please wait</H3><br>\
      After the controller completes the boot process (lights will flash blue, followed by red/green for approx. 2 seconds), you may click the following link to return to the main page:<br><br>\
      <a href=\"http://";      
    restartMsg += baseIP;
    restartMsg += "\">Return to settings</a><br>";
    restartMsg += "</body></html>";
    server.send(200, "text/html", restartMsg);
    delay(1000);
    ESP.restart();
}

/* ================================
    Home Assistant MQTT Discovery - v0.45
   ================================
*/
void handleDiscovery() {
  //Main page for enabling/disabling Home Assistant MQTT Discovery
  String discMsg = "<HTML>\
      </head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>Parking Assistant MQTT Discovery</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
      <H1>Home Assistant MQTT Discovery - BETA</H1>\
      (BETA Note: Tested successfully with Home Assistant Core 2024.1 but will be considered a beta feature until broader testing by others has been completed.)<br><br>\
      This feature can be used to add or remove the parking assistant device and MQTT entities to Home Assistant without manual YAML editing.<br><br>";  
  discMsg += "<u><b>Prerequisites and Notes</b></u>";
  discMsg += "<ul><li>It is <b><i>strongly recommended</i></b> that you read the &nbsp"; 
  discMsg += "<a href=\"https://github.com/Resinchem/ESP-Parking-Assistant/wiki/08-MQTT-and-Home-Assistant\" target=\"_blank\" rel=\"noopener noreferrer\">MQTT Documentation</a> before using this feature.</li>";
  discMsg += "<li>You must have successfully completed the MQTT setup, rebooted and estabished a connection to your broker before these options will work.</li>";
  discMsg += "<li>The Home Assistant MQTT integration must be installed, discovery must not have been disabled nor the default discovery topic changed.</li>";
  discMsg += "<li>The action to enable/disable will occur immediately in Home Assistant without any interaction or prompts.</li>";
  discMsg += "<li>If you have already manually created the Home Assistant MQTT entities, enabling discovery will create duplicate entities with different names.</li></ul>";
  discMsg += "<H3>Enable Discovery</H3>";
  discMsg += "This will immediately create a device called <b>VAR_DEVICE_NAME</b> in your Home Assistant MQTT Integration.<br>";
  discMsg += "It will also create the following entities for this device:\
      <ul>\
      <li>Car Presence</li>\
      <li>Park Distance</li>\
      <li>IP Address</li>\
      <li>MAC Address</li></ul><br>";
  discMsg += "<button id=\"btnenable\" onclick=\"location.href = './discoveryEnabled';\">Enable Discovery</button>"; 
  discMsg += "<br><hr>";
  discMsg += "<H3>Disable Discovery</H3>";
  discMsg += "This will <b>immediately</b> delete the device and all entities created MQTT Discover for this device.<br>\
      If you have any automations, scripts, dashboard entries or other processes that use these entities, you will need to delete or correct those items in Home Assistant.<br><br>";
  discMsg += "<button id=\"btndisable\" onclick=\"location.href = './discoveryDisabled';\">Disable Discovery</button><br><br>"; 
      
  discMsg += "<br><a href=\"http://";
  discMsg += baseIP;
  discMsg += "\">Return to settings</a><br>";
  discMsg += "</body></html>";
  discMsg.replace("VAR_DEVICE_NAME", deviceName);
  server.send(200, "text/html", discMsg); 
}

void enableDiscovery() {
  String discMsg = "<HTML>\
      </head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>Parking Assistant MQTT Discovery</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
      <H1>Home Assistant MQTT Discovery</H1><br>";
  if (mqttEnabled) {
    byte retVal = haDiscovery(true);
    if (retVal == 0) {

      discMsg += "<b><p style=\"color: #5a8f3d;\">MQTT Discovery topics successfully sent to Home Assistant.</p></b><br>";
      discMsg += "If the Home Assistant MQTT integration has been configured correctly, you should now find a new device, <b>VAR_DEVICE_NAME</b>, under the MQTT integration.";
      discMsg += "<p style=\"color: #1c5410;\">\
                   <ul>\
                   <li>You can change the name of the device or any entities, if desired, via Home Assistant</li>\
                   <li>To remove (permanently delete) the created device and entities, disable MQTT Discovery</li>\
                   <li>If you disable MQTT Discover and then reenable it, the device and entities will be recreated with their original names.</li>\
                   </ul></p><br>";
      discMsg += "If you do not see the new device under your Home Assistant MQTT integration, please use a utility (e.g. MQTT Explorer or similar) to see if the controller is successfully connecting to your broker.<br>\ 
                  You should see an MQTT connected message, along with the IP and MAC addresses of your controller under the topic specified on the main settings page.<br><br>";
      discMsg += "For additional troubleshooting tips, please see the <a href=\"https://github.com/Resinchem/ESP-Parking-Assistant/wiki/08-MQTT-and-Home-Assistant\" target=\"_blank\" rel=\"noopener noreferrer\">MQTT Documentation</a><br><br>\
                  It is recommended that you disable MQTT Discovery in the Parking Assistant app until you resolve any MQTT/Home Assistant issues.<br><br>";
    
    } else if (retVal == 1) {
      //Unable to connect or reconnect to broker
      discMsg += "<b><p style=\"color: red;\">The Parking Assistant was unable to connect or reconnect to your MQTT broker!</p></b><br>";
      discMsg += "Please be sure your broker is running and accessible on the same network and that you have completed the following:";
      discMsg += "<ul>\
                   <li>Entered the proper MQTT broker settings and the correct User Name and Password</li>\
                   <li>Checked the box to save settings as new boot defaults</li>\
                   <li>Rebooted the controller</li>\
                   </ul><br>";
      discMsg += "Please use a utility (e.g. MQTT Explorer or similar) to test if the controller is successfully connecting to your broker.<br>\ 
                  You should see an MQTT connected message, along with the IP and MAC addresses of your controller under the topic specified on the main settings page.<br><br>\
                  Until you successfully see these topics in your broker, you will not be able to utilize MQTT Discovery.<br><br>";
      discMsg += "<p style=\"color: red;\">No Home Assistant devices or entities were created.</p><br><br>";
      
    } else {
      //Unknown error or issue
      discMsg += "<b><p style=\"color: red;\">An unknown error or issue occurred!</p></b><br>";
      discMsg += "Please be sure your broker is running and accessible on the same network and that you have completed the following:";
      discMsg += "<ul>\
                   <li>Entered the proper MQTT broker settings and the correct User Name and Password</li>\
                   <li>Checked the box to save settings as new boot defaults</li>\
                   <li>Rebooted the controller</li>\
                   </ul><br>";
      discMsg += "Please use a utility (e.g. MQTT Explorer or similar) to test if the controller is successfully connecting to your broker.<br>\ 
                  You should see an MQTT connected message, along with the IP and MAC addresses of your controller under the topic specified on the main settings page.<br><br>\
                  Until you successfully see these topics in your broker, you will not be able to utilize MQTT Discovery and may need to manually create the Home Assistant entities.<br><br>";
      discMsg += "For additional troubleshooting tips, please see the <a href=\"https://github.com/Resinchem/ESP-Parking-Assistant/wiki/08-MQTT-and-Home-Assistant\" target=\"_blank\" rel=\"noopener noreferrer\">MQTT Documentation</a><br><br>";
      discMsg += "<p style=\"color: red;\">No Home Assistant devices or entities were created.</p><br><br>";
    }
  } else {
    discMsg += "<b><p style=\"color: red;\">MQTT is not enabled or was unable to connect to your broker!</p></b><br>";
    discMsg += "Before attempting to enable MQTT Discovery, please be sure you have completed the following:";
    discMsg += "<ul>\
                 <li>Entered the proper MQTT broker settings and the correct User Name and Password</li>\
                 <li>Checked the box to save settings as new boot defaults</li>\
                 <li>Rebooted the controller</li>\
                 </ul><br>";
    discMsg += "Please use a utility (e.g. MQTT Explorer or similar) to see if the controller is successfully connecting to your broker.<br>\ 
                You should see an MQTT connected message, along with the IP and MAC addresses of your controller under the topic specified on the main settings page.<br><br>\
                Until you successfully see these topics in your broker, you will not be able to enable MQTT Discovery.<br>";
    
  }
  discMsg += "<br><a href=\"http://";
  discMsg += baseIP;
  discMsg += "\">Return to settings</a><br>";
  discMsg += "</body></html>";
  discMsg.replace("VAR_DEVICE_NAME", deviceName);
  server.send(200, "text/html", discMsg); 

}

void disableDiscovery() {
  String discMsg = "<HTML>\
      </head>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>Parking Assistant MQTT Discovery</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
      <H1>Home Assistant MQTT Discovery</H1><br>";
  if (mqttEnabled) {
    byte retVal = haDiscovery(false);
    if (retVal == 0) {
      discMsg += "<b><p style=\"color: #5a8f3d;\">MQTT Removal topics successfully sent to Home Assistant.</p></b><br>";
      discMsg += "The device <b>VAR_DEVICE_NAME</b> should now be deleted from Home Assistant, along with the following entities:\
        <ul>\
        <li>Car Presence</li>\
        <li>Park Distance</li>\
        <li>IP Address</li>\
        <li>MAC Address</li></ul><br>";
            
    } else if (retVal == 1) {
      //Unable to connect or reconnect to broker
      discMsg += "<b><p style=\"color: red;\">The Parking Assistant was unable to connect or reconnect to your MQTT broker!</p></b><br>";
      discMsg += "Please be sure your broker is running and accessible on the same network and that you have completed the following:";
      discMsg += "<ul>\
                   <li>Entered the proper MQTT broker settings and the correct User Name and Password</li>\
                   <li>Checked the box to save settings as new boot defaults</li>\
                   <li>Rebooted the controller</li>\
                   </ul><br>";
      discMsg += "Please use a utility (e.g. MQTT Explorer or similar) to test if the controller is successfully connecting to your broker.<br>\ 
                  You should see an MQTT connected message, along with the IP and MAC addresses of your controller under the topic specified on the main settings page.<br><br>\
                  Until you successfully see these topics in your broker, you will not be able to utilize MQTT Discovery.<br><br>";
      discMsg += "<p style=\"color: red;\">No Home Assistant devices or entities were removed.</p><br><br>";
      
    } else {
      //Unknown error or issue
      discMsg += "<b><p style=\"color: red;\">An unknown error or issue occurred!</p></b><br>";
      discMsg += "Please be sure your broker is running and accessible on the same network and that you have completed the following:";
      discMsg += "<ul>\
                   <li>Entered the proper MQTT broker settings and the correct User Name and Password</li>\
                   <li>Checked the box to save settings as new boot defaults</li>\
                   <li>Rebooted the controller</li>\
                   </ul><br>";
      discMsg += "Please use a utility (e.g. MQTT Explorer or similar) to test if the controller is successfully connecting to your broker.<br>\ 
                  You should see an MQTT connected message, along with the IP and MAC addresses of your controller under the topic specified on the main settings page.<br><br>\
                  Until you successfully see these topics in your broker, you will not be able to utilize MQTT Discovery and may need to manually create the Home Assistant entities.<br><br>";
      discMsg += "For additional troubleshooting tips, please see the <a href=\"https://github.com/Resinchem/ESP-Parking-Assistant/wiki/08-MQTT-and-Home-Assistant\" target=\"_blank\" rel=\"noopener noreferrer\">MQTT Documentation</a><br><br>";
      discMsg += "<p style=\"color: red;\">No Home Assistant devices or entities were removed.</p><br><br>";
      
    }
  } else {
    discMsg += "<b><p style=\"color: red;\">MQTT is not enabled or was unable to connect to your broker!</p></b><br>";
    discMsg += "Before attempting to use MQTT Discovery, please be sure you have completed the following:";
    discMsg += "<ul>\
                 <li>Entered the proper MQTT broker settings and the correct User Name and Password</li>\
                 <li>Checked the box to save settings as new boot defaults</li>\
                 <li>Rebooted the controller</li>\
                 </ul><br>";
    discMsg += "Please use a utility (e.g. MQTT Explorer or similar) to see if the controller is successfully connecting to your broker.<br>\ 
                You should see an MQTT connected message, along with the IP and MAC addresses of your controller under the topic specified on the main settings page.<br><br>\
                Until you successfully see these topics in your broker, you will not be able to enable MQTT Discovery.<br>";
    
  }
  discMsg += "<br><a href=\"http://";
  discMsg += baseIP;
  discMsg += "\">Return to settings</a><br>";
  discMsg += "</body></html>";
  discMsg.replace("VAR_DEVICE_NAME", deviceName);
  server.send(200, "text/html", discMsg); 

}

// Not found or invalid page handler
void handleNotFound() {
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
  
  IPAddress myserver = IPAddress(mqttAddr_1, mqttAddr_2, mqttAddr_3, mqttAddr_4);
  
  client.setServer(myserver, mqttPort);
  client.setBufferSize(512); 
  client.setCallback(callback);
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.print("Connecting to MQTT broker.");
  #endif
  while (!client.connected( )) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.print(".");
    #endif
    client.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPW.c_str());
    if (mcount >= 60) {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println();
        Serial.println("Could not connect to MQTT broker. MQTT disabled.");
      #endif
      // Could not connect to MQTT broker
      return false;
    }
    delay(500);
    mcount++;
  }
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println();
    Serial.println("Successfully connected to MQTT broker.");
  #endif
  client.subscribe(("cmnd/" + mqttTopicSub + "/#").c_str());
  client.publish(("stat/" + mqttTopicPub + "/mqtt").c_str(), "connected", true);
  //Publish current IP and MAC addresses
  client.publish(("stat/" + mqttTopicPub + "/ipaddress").c_str(), baseIP.c_str(), true);
  client.publish(("stat/" + mqttTopicPub + "/macaddress").c_str(), strMacAddr.c_str(), true);
  mqttConnected = true;
  return true;
}

void reconnect() {
  int retries = 0;
  while (!client.connected()) {
    if(retries < 150)
    {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.print("Attempting MQTT connection...");
      #endif
      if (client.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPW.c_str())) 
      {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.println("connected");
        #endif
        // ... and resubscribe
        client.subscribe(("cmnd/" + mqttTopicSub + "/#").c_str());
      } 
      else 
      {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
        #endif
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if ((retries > 149) && (mqttEnabled))
    {
    ESP.restart();
    }
  }
}

bool reconnect_soft() {
  //Attempt MQTT reconnect.  If fails, return false instead of forcing ESP Reboot
  int retries = 0;
  while (!client.connected()) {
    if(retries < 10)
    {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.print("Attempting MQTT connection...");
      #endif
      if (client.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPW.c_str())) 
      {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.println("connected");
        #endif
        // ... and resubscribe
        client.subscribe(("cmnd/" + mqttTopicSub + "/#").c_str());
        return true;
      } 
      else 
      {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
        #endif
        retries++;
        // Wait 3 seconds before retrying
        delay(3000);
        yield();
      }
    } 
    if ((retries > 9) && (mqttEnabled)) 
    {
       return false;
    }
  }
  return false;
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = (char*)payload;
  /*
   * Add any commands submitted here
   * Example:
   * if (strcmp(topic, "cmnd/matrix/mode")==0) {
   *   MyVal = message;
   *   Do something
   *   return;
   * };
   */
}

void readConfigFile() {
  if (SPIFFS.begin()) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println("mounted file system");
    #endif
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println("reading config file");
      #endif
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.println("opened config file");
        #endif
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
         #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
            Serial.println("\nparsed json");
          #endif
          // Read values here from SPIFFS (v0.42 - add defaults for all values in case they don't exist to avoid potential boot loop)
          strcpy(led_count, json["led_count"]|"30");
          strcpy(led_park_time, json["led_park_time"]|"60");
          strcpy(led_exit_time, json["led_exit_time"]|"5");
          strcpy(led_brightness_active, json["led_brightness_active"]|"100");
          strcpy(led_brightness_sleep, json["led_brightness_sleep"]|"5");
          strcpy(uom_distance, json["uom_distance"]|"0");                    // Default needed if upgrading to v0.41, because value won't exist in config
          strcpy(wake_mils, json["wake_mils"]|"3048");
          strcpy(start_mils, json["start_mils"]|"1829");
          strcpy(park_mils, json["park_mils"]|"610");
          strcpy(backup_mils, json["backup_mils"]|"457");
          strcpy(color_standby, json["color_standby"]|"3");
          strcpy(color_wake, json["color_wake"]|"2");
          strcpy(color_active, json["color_active"]|"1");
          strcpy(color_parked, json["color_parked"]|"0");
          strcpy(color_backup, json["color_backup"]|"0");
          strcpy(led_effect, json["led_effect"]|"Out-In");
          strcpy(mqtt_addr_1, json["mqtt_addr_1"]|"0");
          strcpy(mqtt_addr_2, json["mqtt_addr_2"]|"0");
          strcpy(mqtt_addr_3, json["mqtt_addr_3"]|"0");
          strcpy(mqtt_addr_4, json["mqtt_addr_4"]|"0");
          strcpy(mqtt_port, json["mqtt_port"]|"0");
          strcpy(mqtt_tele_period, json["mqtt_tele_period"]|"60");
          strcpy(mqtt_user, json["mqtt_user"]|"mqttuser");
          strcpy(mqtt_pw, json["mqtt_pw"]|"mqttpwd");
          strcpy(device_name, json["device_name"]|"parkasst");               // Default needed if upgrading to v0.41, because value won't exist in config
          strcpy(mqtt_topic_sub, json["mqtt_topic_sub"]|"parkasst");         // Default needed if upgrading to v0.41, because value won't exist in config
          strcpy(mqtt_topic_pub, json["mqtt_topic_pub"]|"parkasst");         // Default needed if upgrading to v0.41, because value won't exist in config
          //Need to set wifihostname here
          deviceName = String(device_name);
        } else {
          #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
            Serial.println("failed to load json config");
          #endif
        }
        configFile.close();
      }
    }
    SPIFFS.end();  
  } else {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println("failed to mount FS");
    #endif
  }
}


// ==================================
//  Main Setup
// ==================================
void setup() {
  // Serial monitor
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.begin(115200);
    Serial.println("Booting...");
  #endif

  //Define Effects and Colors
  defineEffects();
  defineColors();

  // Default Wifi to station mode - fixes issue #16
  // Local AP will be handed by WifiManager for onboarding
  WiFi.mode(WIFI_STA);
  // -----------------------------------------
  //  Captive Portal and Wifi Onboarding Setup
  // -----------------------------------------
  //clean FS, for testing - uncomment next line ONLY if you wish to wipe current FS
  //SPIFFS.format();
  // *******************************
  // read configuration from FS json
  // *******************************
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("mounting FS...");
  #endif
  readConfigFile();

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  
  WiFiManagerParameter custom_text("<p>Each parking assistant must have a unique name. Letters and numbers only, no spaces, 16 characters max. See the wiki documentation for more info.</p>");
  WiFiManagerParameter custom_dev_id("devName", "Unique Device Name", "parkasst", 16, " 16 chars/alphanumeric only");

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

  //add all your parameters here
  wifiManager.addParameter(&custom_text);
  wifiManager.addParameter(&custom_dev_id);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(360);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name "ESP_ParkingAsst"
  //If not supplied, will use ESP + last 7 digits of MAC
  //and goes into a blocking loop awaiting configuration. If a password
  //is desired for the AP, add it after the AP name (e.g. autoConnect("MyApName", "12345678")
  if (!wifiManager.autoConnect("ESP_ParkingAsst")) {  //
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println("failed to connect and hit timeout");
    #endif
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //Get custom device name and set wifi and OTA host name, mqttclient and 
  if (shouldSaveConfig) {
    strcpy(device_name, custom_dev_id.getValue());
  }
  deviceName = String(device_name);
  //Use device name to define host names and mqttclient name
  wifiHostName = deviceName;
  otaHostName = deviceName + "OTA";
  mqttClient = deviceName;

  //if you get here you have connected to the WiFi
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.hostname(wifiHostName.c_str());
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("connected to your wifi...yay!");
  #endif
  //If callback was excuted, flag was set to save parameters
  if (shouldSaveConfig) {
    updateBootSettings(false);  //don't reboot
    delay(1000);
    readConfigFile();  //Load settings
  }

  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("local ip");
    Serial.println(WiFi.localIP());
  #endif
  baseIP = WiFi.localIP().toString();
  WiFi.macAddress(macAddr);         //Array for Unique ID gen
  strMacAddr = WiFi.macAddress();   //String for MQTT
  // ----------------------------

  //Convert config values to local values
  webColorStandby = atoi(color_standby);
  webColorWake = atoi(color_wake);
  webColorActive = atoi(color_active);
  webColorParked = atoi(color_parked);
  webColorBackup = atoi(color_backup);

  numLEDs = (String(led_count)).toInt();
  maxOperationTimePark = (String(led_park_time)).toInt();
  maxOperationTimeExit = (String(led_exit_time)).toInt();
  activeBrightness = (String(led_brightness_active)).toInt();
  sleepBrightness = (String(led_brightness_sleep)).toInt();
  if (sleepBrightness == 0) {
    showStandbyLEDs = false;
  } else {
    showStandbyLEDs = true;
  }
  uomDistance = (String(uom_distance)).toInt();
  wakeDistance = (String(wake_mils)).toInt();
  startDistance = (String(start_mils)).toInt();
  parkDistance = (String(park_mils)).toInt();
  backupDistance = (String(backup_mils)).toInt();

  ledColorStandby = ColorCodes[webColorStandby];
  ledColorWake = ColorCodes[webColorWake];
  ledColorActive = ColorCodes[webColorActive];
  ledColorParked = ColorCodes[webColorParked];
  ledColorBackup = ColorCodes[webColorBackup];
  ledEffect_m1 = String(led_effect);

  mqttAddr_1 = (String(mqtt_addr_1)).toInt();
  mqttAddr_2 = (String(mqtt_addr_2)).toInt();
  mqttAddr_3 = (String(mqtt_addr_3)).toInt();
  mqttAddr_4 = (String(mqtt_addr_4)).toInt();
  //Disable MQTT if IP = 0.0.0.0
  if ((mqttAddr_1 == 0) && (mqttAddr_2 == 0) && (mqttAddr_3 == 0) && (mqttAddr_4 == 0)) {
    mqttPort = 0;
    mqttEnabled = false;         
    mqttConnected = false;       
    
  } else {
    mqttPort = (String(mqtt_port)).toInt();
    mqttTelePeriod = (String(mqtt_tele_period)).toInt();
    mqttUser = String(mqtt_user);
    mqttPW = String(mqtt_pw);
    mqttTopicSub = String(mqtt_topic_sub);
    mqttTopicPub = String(mqtt_topic_pub);
    mqttEnabled = true;
  }

  //------------------------------
  // Setup handlers for web calls
  //------------------------------
  server.on("/", handleRoot);

  server.on("/postform/", handleForm);

  server.onNotFound(handleNotFound);

  server.on("/update", handleUpdate);

  server.on("/restart", handleRestart);

  server.on("/reset", handleReset);

  server.on("/discovery", handleDiscovery);

  server.on("/discoveryEnabled", enableDiscovery);

  server.on("/discoveryDisabled", disableDiscovery);

  server.on("/otaupdate",[]() {
    //Called directly from browser address (//ip_address/otaupdate) to put controller in ota mode for uploadling from Arduino IDE
    server.send(200, "text/html", "<h1>Ready for upload...<h1><h3>Start upload from IDE now</h3>");
    ota_flag = true;
    ota_time = ota_time_window;
    ota_time_elapsed = 0;
  });
  //Firmaware Update Handler
  httpUpdater.setup(&server, "/update2");
  httpUpdater.setup(&server);
  server.begin();
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("Setup complete - starting main loop");
  #endif

  // =====================
  //  MQTT Setup
  // =====================

  if (mqttEnabled) {
    //Attempt to connect to MQTT broker - if fails, disable MQTT
    if (!setup_mqtt()) {
      mqttEnabled = false;
    }
  }
  
  //-----------------------------
  // Setup OTA Updates
  //-----------------------------
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
  
  // -------------
  // SETUP FASTLED  
  // -------------
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(LEDs, NUM_LEDS_MAX);
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(activeBrightness);
  
  // --------------
  // SETUP TFMINI
  // --------------
  // TFMini uses Serial pins, so SERIAL_DEBUB must be 0 - otherwise only zero distance will be reported
#if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 0)
  Serial.begin(115200);
  delay(20);
  tfmini.begin(&Serial);
  tfMiniEnabled = true;
#endif

  // ---------------------------------------------------------
  // Flash LEDs blue for 2 seconds to indicate successful boot 
  // ---------------------------------------------------------
  fill_solid(LEDs, numLEDs, CRGB::Blue);
  FastLED.show();
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("LEDs Blue - FASTLED ok");
  #endif
  delay(2000);
  fill_solid(LEDs, numLEDs, CRGB::Black);
  FastLED.show();
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println("LEDs Reset to off");
  #endif

  // Set interval distance based on current Effect
  intervalDistance = calculateInterval();
 }


// =============================
//   MAIN LOOP
// =============================
void loop() {
  //Handle OTA updates when OTA flag set via HTML call to http://ip_address/otaupdate
  if (ota_flag) {
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

  //Handle MQTT calls (if enabled)
  if (mqttEnabled) {
  #if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
    if (!client.connected()) 
    {
      reconnect();
    }
    client.loop();
  #endif
    
  }
  uint32_t currentMillis = millis();
  int16_t tf_dist = 0;
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

  //Determine if car (or other object) present in any zones
  if (tf_dist <= wakeDistance) {
    if (!carDetected) {
      carDetectedCounter ++;
    }
    if (carDetectedCounter > carDetectedCounterMax) {  //eliminate trigger on noise
      carDetectedCounter = 0;
      nocarDetectedCounter = 0;      //new v0.50 - attempt to address bounce
      carDetected = true;
      exitSleepTimerStarted = false;
      parkSleepTimerStarted = true;
      startTime = currentMillis;
      FastLED.setBrightness(activeBrightness);
      isAwake = true;
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
    }
  }

  //v0.44 - force MQTT update if car state changes
  if (carDetected != prevCarStatus) {
    prevCarStatus = carDetected;
    forceMQTTUpdate = true;
  }

  //Update LEDs
  if ((carDetected) && (isAwake)) {
    if (tf_dist <= backupDistance) {
      //Beyond minimum distance - flash backup!
      blinkLEDs(ledColorBackup);
      
    } else if (tf_dist <= parkDistance) {  
      //In desired parked distance
      fill_solid(LEDs, numLEDs, ledColorParked);
  
    } else if ((tf_dist > startDistance) && (tf_dist <= wakeDistance)) {
      //Beyond start distance but within wake distance
      fill_solid(LEDs, numLEDs, ledColorWake);
      
    } else if ((tf_dist <= startDistance) && (tf_dist > parkDistance)) {
      //Update based on selected effect
      if (ledEffect_m1 == "Out-In") {
        updateOutIn(tf_dist);
      } else if (ledEffect_m1 == "In-Out") {
        updateInOut(tf_dist);
      } else if (ledEffect_m1 == "Full-Strip") {
        updateFullStrip(tf_dist);
      } else if (ledEffect_m1 == "Full-Strip-Inv") {
        updateFullStripInv(tf_dist);
      } else if (ledEffect_m1 == "Solid") {
        updateSolid(tf_dist);
      }
    }
  }
 
  //Put system to sleep if parking or exit time elapsed 
  uint32_t elapsedTime = currentMillis - startTime;
  if (((elapsedTime > (maxOperationTimePark * 1000)) && (parkSleepTimerStarted)) || ((elapsedTime > (maxOperationTimeExit * 1000)) && (exitSleepTimerStarted  ))) {
    updateSleepMode();
    isAwake = false;
    startTime = currentMillis;
    exitSleepTimerStarted = false;
    parkSleepTimerStarted = false;
  }

  //Show/Refresh LED Strip
  FastLED.show();

  //Update MQTT Stats per tele period
  if (mqttEnabled) {
    if (((currentMillis - mqttLastUpdate) > (mqttTelePeriod * 1000)) || (forceMQTTUpdate)) {
      mqttLastUpdate = currentMillis;
      forceMQTTUpdate = false;
      if (!client.connected()) {
        reconnect();
      }
      // Publish MQTT values
      char outMsg[6];
      byte carStatus = 0;
      float measureDistance = 0;
      if (carDetected) carStatus = 1;

      if (tf_dist > 50000) {
        measureDistance = tf_dist * 1.0 ;  //TFMini returning error code, just output to MQTT for troubleshooting
      } else if (tf_dist > 5080) {         // Out of range
        if (uomDistance) {
          measureDistance = 5080;
        } else {
          measureDistance = 200;
        }
     } else {
        if (uomDistance) {
          measureDistance = tf_dist;
        } else {
          measureDistance = tf_dist / 25.4;
        }
     }
      
      sprintf(outMsg, "%1u",carStatus);
      client.publish(("stat/" + mqttTopicPub + "/cardetected").c_str(), outMsg, true);
      sprintf(outMsg, "%2.1f", measureDistance);
      client.publish(("stat/" + mqttTopicPub + "/parkdistance").c_str(), outMsg, true);
    }
  }
  delay(200);
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

// ===============================
//  LED and Display Functions
// ===============================
void blinkLEDs(CRGB color) {
  if (blinkOn) {
    fill_solid(LEDs, numLEDs, color);
  } else {
    fill_solid(LEDs, numLEDs, CRGB::Black);
  }
  blinkOn = !blinkOn;
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
  if (numberToLight ==0 ) numberToLight = 1;  //Assure at least 1 light if integer truncation results in 0
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

// ==============================
//  MQTT Functions and Procedures
// ==============================
void createUniqueId() {
  // Get Unique ID as uidPrefix + last 6 MAC digits

  strcpy(devUniqueID, uidPrefix);
  int preSizeBytes = sizeof(uidPrefix);
  int preSizeElements = (sizeof(uidPrefix) / sizeof(uidPrefix[0]));
  //Now add last 6 chars from MAC address (these are 'reversed' in the array)
  int j = 0;
  //for (int i = 2; i >= 0; i--) {
  for (int i = 3; i < 6; i++) {
    sprintf(&devUniqueID[(preSizeBytes - 1) + (j)], "%02X", macAddr[i]);   //preSizeBytes indicates num of bytes in prefix - null terminator, plus 2 (j) bytes for each hex segment of MAC
    j = j + 2;
  }
  //devUniqueID would now contain something like "prkast02BE4F" which can be used for topics/payloads
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.print("UNIQUE ID CREATED: ");
    Serial.println(devUniqueID);
  #endif
}


byte haDiscovery (bool enable) {
  char topic[128];
  char buffer1[512];
  char buffer2[512];
  char buffer3[512];
  char buffer4[512];
  char uid[128];
  if (mqttEnabled) {
    createUniqueId();
    if (enable) {    
      //Add device and entities
      if (!client.connected()) { 
        if (!reconnect_soft()) {
          return 1;
        }
      }
      //Should have valid MQTT Connection at this point
      DynamicJsonDocument doc(512);
      
      //Create primary car presence binary sensor with full device details
      //topic
      strcpy(topic, "homeassistant/binary_sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "P/config");
      //Unique ID
      strcpy(uid, devUniqueID);
      strcat(uid, "P");
      //JSON Payload
      doc.clear();
      doc["name"] = "Car presence";
      doc["uniq_id"] = uid;
      doc["deve_cla"] = "occupancy";
      doc["stat_t"] = "stat/" + mqttTopicPub + "/cardetected";
      doc["pl_on"] = "1";
      doc["pl_off"] = "0";
      JsonObject deviceP = doc.createNestedObject("device");
        deviceP["ids"] = deviceName + devUniqueID;
        deviceP["name"] = deviceName;
        deviceP["mdl"] = "Parking Assistant";
        deviceP["mf"] = "Resinchem Tech";
        deviceP["sw"] = VERSION;
        deviceP["cu"] = "http://" + baseIP;
      serializeJson(doc, buffer1);
      client.publish(topic, buffer1, true);

      //Create Parked Distance sensor
      //topic
      strcpy(topic, "homeassistant/sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "D/config");
      //Unique ID
      strcpy(uid, devUniqueID);
      strcat(uid, "D");
      //JSON Payload
      doc.clear();
      doc["name"] = "Park distance";
      doc["uniq_id"] = uid;
      doc["deve_cla"] = "distance";
      doc["stat_t"] = "stat/" + mqttTopicPub + "/parkdistance";
      if (uomDistance) {
        doc["unit_of_meas"] = "mm";
      } else {
        doc["unit_of_meas"] = "in";
      }
      JsonObject deviceD = doc.createNestedObject("device");
        deviceD["ids"] = deviceName + devUniqueID;
        deviceD["name"] = deviceName;
      serializeJson(doc, buffer2);
      client.publish(topic, buffer2, true);
       
      //Create IP Address as Diagnostic Sensor
      //topic
      strcpy(topic, "homeassistant/sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "I/config");
      //Unique ID
      strcpy(uid, devUniqueID);
      strcat(uid, "I");
      //JSON Payload
      doc.clear();
      doc["name"] = deviceName + " IP address";
      doc["uniq_id"] = uid;
      doc["ent_cat"] = "diagnostic";
      doc["stat_t"] = "stat/" + mqttTopicPub + "/ipaddress";
      JsonObject deviceI = doc.createNestedObject("device");
        deviceI["ids"] = deviceName + devUniqueID;
        deviceI["name"] = deviceName;
      serializeJson(doc, buffer3);
      client.publish(topic, buffer3, true);
     
      //Create MAC Address as Diagnostic Sensor
      //topic
      strcpy(topic, "homeassistant/sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "M/config");
      //Unique ID
      strcpy(uid, devUniqueID);
      strcat(uid, "M");
      //JSON Payload
      doc.clear();
      doc["name"] = deviceName + " MAC address";
      doc["uniq_id"] = uid;
      doc["ent_cat"] = "diagnostic";
      doc["stat_t"] = "stat/" + mqttTopicPub + "/macaddress";
      JsonObject deviceM = doc.createNestedObject("device");
        deviceM["ids"] = deviceName + devUniqueID;
        deviceM["name"] = deviceName;
      serializeJson(doc, buffer4);
      client.publish(topic, buffer4, true);
      return 0;
      
    } else {
      //Remove all Discovered Devices/entities  
      if (!client.connected()) { 
        if (!reconnect_soft()) {
          return 1;
        }
      }
      //Publish empty payload (retained false) for each of the topics created above
      //Car Presence
      strcpy(topic, "homeassistant/binary_sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "P/config");
      client.publish(topic, "");

      //Park Distance
      strcpy(topic, "homeassistant/sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "D/config");
      client.publish(topic, "");

     //IP Address
      strcpy(topic, "homeassistant/sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "I/config");
      client.publish(topic, "");

     //MAC Address
      strcpy(topic, "homeassistant/sensor/");
      strcat(topic, devUniqueID);
      strcat(topic, "M/config");
      client.publish(topic, "");

      return 0;
    }
  } else {
    //MQTT not enabled... should never hit this as it is checked before calling  
    return 90;
  }
  //catch all
  return 99;
}
