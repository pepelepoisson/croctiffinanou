// InspiMason
// Check http://www.chezpapietmamie.com/inspimason/

// Main examples used to build this secondes
// Basic SPIFFS write/read example: https://github.com/G6EJD/SPIFFS-Examples/blob/master/ESP8266_SPIFFS_Example.ino
// Uploading a file to SPIFFS from browser: https://www.esp8266.com/viewtopic.php?t=5779&start=10#

// Wonderfull online true type to gfx fonts conversion tool: https://rop.nl/truetype2gfx/
// Wonderfull online tool to edit fonts: https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

#include <Arduino.h>
#include <GxEPD2.h>
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <FS.h> //spiff file system
#include <ArduinoJson.h>

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define LOW_BRIGHTNESS          20
#define FRAMES_PER_SECOND   60 // 125 frames/sec <=> 8 milli/frame
#define NUM_LEDS  48
#define DATA_PIN    5    // Was 5 for GeaiRareLit
#define MOSFET_GATE  12
#define LED_ANIMATIONS_DURATION 300000 // Duration of LED animations (Milliseconds)
#define SCREEN_ROTATION 0
#define NUMBER_FILES 4

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);
uint16_t brightness=LOW_BRIGHTNESS;
long start_time=0;
int code_run_counter=0;
int current_citation=0;
int current_citation_1=0, current_citation_2=0, current_citation_3=0, current_citation_4=0;
int number_of_citations=0;
int number_of_citations_1=0, number_of_citations_2=0, number_of_citations_3=0, number_of_citations_4=0;
int target_citation=0;
int code_run_counter_bkp=0;
//int current_citation_bkp=0;
int current_citation_1_bkp=0, current_citation_2_bkp=0, current_citation_3_bkp=0, current_citation_4_bkp=0;
//int number_of_citations_bkp=0;
int number_of_citations_1_bkp=0,number_of_citations_2_bkp=0, number_of_citations_3_bkp=0, number_of_citations_4_bkp=0;
int random_pickup=0;
int vbat_counter=0;
bool SetupMode=false;
bool citations_file_1_exists=false,citations_file_2_exists=false,citations_file_3_exists=false,citations_file_4_exists=false;
bool citation_fits=false;
bool first_time=true;
String wifi_status = "Disconnected";
String method4next = "Aleatoire";  // Warning!!!! Aucun accent sinon l'écriture dans log.json ne fonctionne plus!
String method4next_bkp = "Aleatoire";  // Warning!!!! Aucun accent sinon l'écriture dans log.json ne fonctionne plus!
String fw_version="2.0 (avr 2022)";

const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0
int sensorValue = 0;  // value read from the ADC0
float batteryVoltage=0;
#define batteryVoltageCalibration 165.45
#define LowBattWarningLevel 3.6
#define LowBattAlarmLevel 3.4

#define ENABLE_GxEPD2_GFX 1

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <ModifiedFreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
//#include "project_bitmaps128x296.h"
#include "project_bitmaps_3c_400x300.h"

#define RST_PIN 0 // D3(0)
#define CS_1 SS // CS = D8(15)
#define BUSY_H_x 4 // BUSY = D2(4)

// First line = To use with GeaiRareLit epaper type and circuit
//GxEPD2_BW<GxEPD2_290_M06, GxEPD2_290_M06::HEIGHT> display1(GxEPD2_290_M06(/*CS=*/ CS_1, /*DC=D3*/ 0, /*RST=*/ -1, /*BUSY=*/ BUSY_H_x)); // GDEW029M06
// Second line = To use with waveshare 2.9 epaper - different wiring for RST pin otherwise boot problems
//GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT> display1(GxEPD2_290_T94(/*CS=D8*/ CS_1, /*DC=D3*/ 0, /*RST=D4*/ 5, /*BUSY=D2*/ BUSY_H_x)); // GDEM029T94
GxEPD2_3C<GxEPD2_420c_Z96, GxEPD2_420c_Z96::HEIGHT> display1(GxEPD2_420c_Z96(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEM0420c_Z96
// Tested OK: GxEPD2_3C<GxEPD2_420c_Z96, GxEPD2_420c_Z96::HEIGHT> display1(GxEPD2_420c_Z96(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEM0420c_Z96
// BUSY | RES | D/C | CS | SCK | SDI | GND | 3.3V
//   D2 |  D4 |  D3 | D8 |  D5 |  D7 | GND | 3.3V

// Saint Gerard 2021: GxEPD2_BW<GxEPD2_290_M06, GxEPD2_290_M06::HEIGHT> display1(GxEPD2_290_M06(/*CS=*/ CS_1, /*DC=D3*/ 0, /*RST=*/ -1, /*BUSY=*/ BUSY_H_x)); // GDEW029M06
// BUSY | RES | D/C | CS | SCK | SDI | GND | 3.3V
//  D2  |  D4 |  D3 | D8 | D5  |  D7 | GND | 3.3V

struct bitmap_pair
{
  const unsigned char* black;
  const unsigned char* red;
};

char* mySsid = "inspimason";
IPAddress local_ip(192,168,168,168);
IPAddress gateway(192,168,168,1);
IPAddress netmask(255,255,255,0);
String current_ip, current_ssid;
String citation;

char webpage[] PROGMEM = R"=====(
<html>
<meta charset="UTF-8">
<head>
</head>
<body>
  <form>
  <h1 style="font-size:60px;color:blue;" title="InspiMason">InspiMason - Une conserve d'idées inspirantes</h1>
    <fieldset>
    <legend style="color:blue;">Paramètres pour la connexion à un réseau WiFi existant:</legend>
      <div>
        <label for="ssid">SSID</label>
        <input value="" id="ssid" placeholder="SSID">
      </div>
      <div>
        <label for="password">PASSWORD</label>
        <input type="password" value="" id="password" placeholder="PASSWORD">
      </div>
      <div>
        <button class="primary" id="savebtn" type="button" onclick="myFunction()">SAVE</button>
      </div>
    </fieldset>
  </form>
  <form method='POST' action='/update' enctype='multipart/form-data'>
    <fieldset>
    <legend style="color:blue;">Mise à jour du fichier des citations:</legend>
      <input type='file' name='update'>
      <input type='submit' value='Update'>
    </fieldset>
    <fieldset>
    <legend style="color:blue;">Choix de l'ordre d'affichage:</legend>
    <div>
      <p> Ordre d'affichage des textes : <span id="method4next">__</span> </p>
      <button class="primary" id="togglebtn" type="button" onclick="SetDisplayOrderFunction()">Changer ordre</button>
    </div>
    </fieldset>
  </form>
  <form action="/action_page">
  <fieldset>
  <legend style="color:blue;">Message à afficher immédiatement:</legend>
    Message à envoyer:<br>
    <input type="text" name="message_web" size="150" >
    <br><br>
    <input type="submit" value="Envoyer">
    </fieldset>
  </form>
  <form>
  <fieldset>
  <legend style="color:blue;">En manque d'inspiration?</legend>
    <div>
    Consultez <a href="http://www.chezpapietmamie.com/inspimason/" target="_blank">le site web InspiMason</a> pour télecharger ou partager des fichiers de contenu!<br>
    </div>
  </fieldset>
</form>
</body>
<script>
function myFunction()
{
  console.log("button was clicked!");

  var ssid = document.getElementById("ssid").value;
  var password = document.getElementById("password").value;
  var data = {ssid:ssid, password:password};

  var xhr = new XMLHttpRequest();
  var url = "/settings";

  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      // Typical action to be performed when the document is ready:
      if(xhr.responseText != null){
        console.log(xhr.responseText);
      }
    }
  };

  xhr.open("POST", url, true);
  xhr.send(JSON.stringify(data));
};
function SetDisplayOrderFunction()
{
  console.log("TOGGLE button was clicked!");
  var xhr = new XMLHttpRequest();
    var url = "/change_method4next";

    xhr.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("method4next").innerHTML = this.responseText;
      }
    };

    xhr.open("GET", url, true);
    xhr.send();

};

document.addEventListener('DOMContentLoaded', SetDisplayOrderFunction, false);
</script>
</html>
)=====";

String filename="citations.txt";
String filename_1="citations.txt", filename_2="blaques.txt", filename_3="enigmes.txt", filename_4="mots_d_amour.txt";
ESP8266WebServer server(80);
//holds the current upload
File UploadFile;

// Add end line characters to avoid breaking words at end of lines + Substitute characters not part of 7 bit font with others that are less needed. Will be displayed properly using modified font.
String formatMessage(String message){
  // Add end line characters to avoid breaking words at end of lines
  uint16_t startpos=0;
  int messageLength=message.length();
  //Serial.println(message);
  //Serial.println(messageLength);

  // Substitute characters not part of 7 bit font with others that are less needed. Will be displayed properly using modified font.
  message.replace("ô","#");
  message.replace("É","$");
  message.replace("û","*");
  message.replace("ç","+");
  message.replace("î","<");
  message.replace("à","{");
  message.replace("ê","|");
  message.replace("é","}");
  message.replace("è","~");
  message.replace("ë",">");
  message.replace("ù","^");
  message.replace("ï","_");
  message.replace("Ê","=");
  message.replace("â","\\");
  message.replace("’","'");
  message.replace("œ","oe");


  char Buf[messageLength+1]; // Char array used to manipulate string content by position
  message.toCharArray(Buf, messageLength+1);  // Store sting content in a char array.
  citation_fits=true;

  for(uint16_t line=1; line<15; line++){
    if (messageLength-startpos>36){  // Code to run only if remaining char array length exceeds maximum size for one line with this font.
      if (line==14){
        //Serial.println("message doesn't fit on screen!");
        citation_fits=false;}
      for (uint16_t k=36; k>1;k--){   // Starting at the end of line, walk back until a space is found = begining of last word.
        if (Buf[startpos+k]==' '){  // Replace space with line break and update counter of next line start.
          Buf[startpos+k]='\n';
          startpos=startpos+k+1;
          break;
        }
      }
    }
  }

  message=Buf;
  return message;
}

// Read log.json file used to store log counters etc. If not found or corrupted then initialize counters and settings.
void read_log_data(){
  const char * _code_run_counter = "", *_method4next = "";
  const char *_current_citation_1 = "",*_current_citation_2 = "",*_current_citation_3 = "",*_current_citation_4 = "";
  const char *_number_of_citations_1 = "",*_number_of_citations_2 = "",*_number_of_citations_3 = "",*_number_of_citations_4 = "";
  const char * _code_run_counter_bkp = "", *_method4next_bkp = "";
  const char *_current_citation_1_bkp = "",*_current_citation_2_bkp = "",*_current_citation_3_bkp = "",*_current_citation_4_bkp = "";
  const char *_number_of_citations_1_bkp = "",*_number_of_citations_2_bkp = "",*_number_of_citations_3_bkp = "",*_number_of_citations_4_bkp = "";

  // Read file #1
  if(SPIFFS.exists("/log.json")){
    Serial.println("Found /log.json file");
    File logFile = SPIFFS.open("/log.json", "r");
    if(logFile){
      size_t size = logFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      logFile.readBytes(buf.get(), size);
      logFile.close();

      DynamicJsonBuffer logjsonBuffer;
      JsonObject& logjObject = logjsonBuffer.parseObject(buf.get());
      if(logjObject.success())
      {
        //Serial.println("Read values from json");
        _code_run_counter = logjObject["code_run_counter"];
        _current_citation_1 = logjObject["current_citation_1"];
        _current_citation_2 = logjObject["current_citation_2"];
        _current_citation_3 = logjObject["current_citation_3"];
        _current_citation_4 = logjObject["current_citation_4"];
        _number_of_citations_1 = logjObject["number_of_citations_1"];
        _number_of_citations_2 = logjObject["number_of_citations_2"];
        _number_of_citations_3 = logjObject["number_of_citations_3"];
        _number_of_citations_4 = logjObject["number_of_citations_4"];
        _method4next=logjObject["method4next"];
        if(_code_run_counter==NULL || _current_citation_1==NULL || _current_citation_2==NULL || _current_citation_3==NULL || _current_citation_4==NULL || _number_of_citations_1==NULL || _number_of_citations_2==NULL ||_number_of_citations_3==NULL ||_number_of_citations_4==NULL || _method4next==NULL){
          Serial.println("Error retrieving stored counters - Will reset.");
          _code_run_counter = "1";
          _current_citation_1 = "1";
          _current_citation_2 = "1";
          _current_citation_3 = "1";
          _current_citation_4 = "1";
          _number_of_citations_1="1";
          _number_of_citations_2="1";
          _number_of_citations_3="1";
          _number_of_citations_4="1";
          _method4next="Aleatoire";
        }
      }
    }
  }
  else {
    Serial.println("File /log.json not found - will initialize");
    _code_run_counter = "1";
    _current_citation_1 = "1";
    _current_citation_2 = "1";
    _current_citation_3 = "1";
    _current_citation_4 = "1";
    _number_of_citations_1="1";
    _number_of_citations_2="1";
    _number_of_citations_3="1";
    _number_of_citations_4="1";
    _method4next="Aleatoire";
  }

  // Now repeat by reading file #2
  if(SPIFFS.exists("/log_bkp.json")){
    Serial.println("Found /log_bkp.json file");
    File logFile = SPIFFS.open("/log_bkp.json", "r");
    if(logFile){
      size_t size = logFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      logFile.readBytes(buf.get(), size);
      logFile.close();

      DynamicJsonBuffer logjsonBuffer;
      JsonObject& logjObject = logjsonBuffer.parseObject(buf.get());
      if(logjObject.success())
      {
        //Serial.println("Read values from json");
        _code_run_counter_bkp = logjObject["code_run_counter"];
        _current_citation_1_bkp = logjObject["current_citation_1"];
        _current_citation_2_bkp = logjObject["current_citation_2"];
        _current_citation_3_bkp = logjObject["current_citation_3"];
        _current_citation_4_bkp = logjObject["current_citation_4"];
        _number_of_citations_1_bkp = logjObject["number_of_citations_1"];
        _number_of_citations_2_bkp = logjObject["number_of_citations_2"];
        _number_of_citations_3_bkp = logjObject["number_of_citations_3"];
        _number_of_citations_4_bkp = logjObject["number_of_citations_4"];
        _method4next_bkp=logjObject["method4next"];
        if(_code_run_counter_bkp==NULL || _current_citation_1_bkp==NULL || _current_citation_2_bkp==NULL || _current_citation_3_bkp==NULL || _current_citation_4_bkp==NULL || _number_of_citations_1_bkp==NULL|| _number_of_citations_2_bkp==NULL|| _number_of_citations_3_bkp==NULL|| _number_of_citations_4_bkp==NULL|| _method4next_bkp==NULL){
          Serial.println("Error retrieving stored counters - Will reset.");
          _code_run_counter_bkp = "1";
          _current_citation_1_bkp = "1";
          _current_citation_2_bkp = "1";
          _current_citation_3_bkp = "1";
          _current_citation_4_bkp = "1";
          _number_of_citations_1_bkp="1";
          _number_of_citations_2_bkp="1";
          _number_of_citations_3_bkp="1";
          _number_of_citations_4_bkp="1";
          _method4next_bkp="Aleatoire";
        }
      }
    }
  }
  else {
    Serial.println("File /log_bkp.json not found - will initialize");
    _code_run_counter_bkp = "1";
    _current_citation_1_bkp = "1";
    _current_citation_2_bkp = "1";
    _current_citation_3_bkp = "1";
    _current_citation_4_bkp = "1";
    _number_of_citations_1_bkp="1";
    _number_of_citations_2_bkp="1";
    _number_of_citations_3_bkp="1";
    _number_of_citations_4_bkp="1";
    _method4next_bkp="Aleatoire";
  }

  // Convert counters from char to integer.
  code_run_counter=atoi(_code_run_counter);
  current_citation_1=atoi(_current_citation_1);
  current_citation_2=atoi(_current_citation_2);
  current_citation_3=atoi(_current_citation_3);
  current_citation_4=atoi(_current_citation_4);
  number_of_citations_1=atoi(_number_of_citations_1);
  number_of_citations_2=atoi(_number_of_citations_2);
  number_of_citations_3=atoi(_number_of_citations_3);
  number_of_citations_4=atoi(_number_of_citations_4);
  method4next=_method4next;
  // Repeat for backup from file #2
  code_run_counter_bkp=atoi(_code_run_counter_bkp);
  current_citation_1_bkp=atoi(_current_citation_1_bkp);
  current_citation_2_bkp=atoi(_current_citation_2_bkp);
  current_citation_3_bkp=atoi(_current_citation_3_bkp);
  current_citation_4_bkp=atoi(_current_citation_4_bkp);
  number_of_citations_1_bkp=atoi(_number_of_citations_1_bkp);
  number_of_citations_2_bkp=atoi(_number_of_citations_2_bkp);
  number_of_citations_3_bkp=atoi(_number_of_citations_3_bkp);
  number_of_citations_4_bkp=atoi(_number_of_citations_4_bkp);
  method4next_bkp=_method4next_bkp;

  // Ensure both sets are identical otherwise use biggest values
  code_run_counter=max(code_run_counter,code_run_counter_bkp);
  current_citation_1=max(current_citation_1,current_citation_1_bkp);
  current_citation_2=max(current_citation_2,current_citation_2_bkp);
  current_citation_3=max(current_citation_3,current_citation_3_bkp);
  current_citation_4=max(current_citation_4,current_citation_4_bkp);
  number_of_citations_1=max(number_of_citations_1,number_of_citations_1_bkp);
  number_of_citations_2=max(number_of_citations_2,number_of_citations_2_bkp);
  number_of_citations_3=max(number_of_citations_3,number_of_citations_3_bkp);
  number_of_citations_4=max(number_of_citations_4,number_of_citations_4_bkp);

  String logData = "{code_run_counter:"+String(code_run_counter)+", current_citation_1:"+String(current_citation_1)+", current_citation_2:"+String(current_citation_2)+", current_citation_3:"+String(current_citation_3)+", current_citation_4:"+String(current_citation_4)+", number_of_citations_1:"+String(number_of_citations_1)+", number_of_citations_2:"+String(number_of_citations_2)+", number_of_citations_3:"+String(number_of_citations_3)+", number_of_citations_4:"+String(number_of_citations_4)+", method4next:"+method4next+"}";
  Serial.print("Got following log data: "); Serial.println(logData);
}

// Write counters and setting variables into json datalog
void update_log_data(){
  String logData = "{code_run_counter:"+String(code_run_counter)+", current_citation_1:"+String(current_citation_1)+", current_citation_2:"+String(current_citation_2)+", current_citation_3:"+String(current_citation_3)+", current_citation_4:"+String(current_citation_4)+", number_of_citations_1:"+String(number_of_citations_1)+", number_of_citations_2:"+String(number_of_citations_2)+", number_of_citations_3:"+String(number_of_citations_3)+", number_of_citations_4:"+String(number_of_citations_4)+", method4next:"+method4next+"}";
  DynamicJsonBuffer logjBuffer;
  JsonObject& logjObject = logjBuffer.parseObject(logData);
  File logFile = SPIFFS.open("/log.json", "w");
  logjObject.printTo(logFile);
  logFile.close();

  // Now repeat for backup fileName
  File logFile_bkp = SPIFFS.open("/log_bkp.json", "w");
  logjObject.printTo(logFile_bkp);
  logFile_bkp.close();

  Serial.print("Updated log data with: ");Serial.println(logData);
}

// Read citation.txt to establish the number of lines. Update number_of_citations and write to json datalog.
void ReadDataFile() {
  String NewLine;
  int number_of_lines=0;
  Serial.println("reading...");
  Serial.println("/data/" + filename);

  if (SPIFFS.exists("/data/" + filename)) {

    //SPIFFS.remove("/data/" + filename); //DEBUG
    File FileRead = SPIFFS.open("/data/" + filename, "r");
    Serial.print("output: ");
    Serial.println();
    while (FileRead.available() > 0) {
      char In = FileRead.read();
      NewLine += In;

      if (In == '\n') {
        //Serial.print(NewLine);
        // Uncoment lines below to search citation.txt to test and report lines exceeding screen size
        formatMessage(NewLine);
        if(!citation_fits){Serial.print("Line "); Serial.print(number_of_lines+1);Serial.println(" doesn't fit");}
        NewLine = "";
        number_of_lines=number_of_lines+1;
      }
    }

    FileRead.close();
    Serial.print("Number of lines in file: "); Serial.println(number_of_lines);
    if (filename==filename_1){
      number_of_citations_1=number_of_lines;
      Serial.print("Updated number_of_citations_1: "); Serial.println(number_of_citations_1);
      current_citation_1=1; // Reset current_citation counter
      update_log_data();
    }
    if (filename==filename_2){
      number_of_citations_2=number_of_lines;
      Serial.print("Updated number_of_citations_2: "); Serial.println(number_of_citations_2);
      current_citation_2=1; // Reset current_citation counter
      update_log_data();
    }
    if (filename==filename_3){
      number_of_citations_3=number_of_lines;
      Serial.print("Updated number_of_citations_3: "); Serial.println(number_of_citations_3);
      current_citation_3=1; // Reset current_citation counter
      update_log_data();
    }
    if (filename==filename_4){
      number_of_citations_4=number_of_lines;
      Serial.print("Updated number_of_citations_4: "); Serial.println(number_of_citations_4);
      current_citation_4=1; // Reset current_citation counter
      update_log_data();
    }
  }
}

//format bytes - Used to format file size when displaying SPIFFS file content
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

// Print a string onto epaper display
void print_string(GxEPD2_GFX& display, String message)
{
  display1.init(115200); // enable diagnostic output on Serial
  display1.mirror(true); //PP added 420c Z96
  display.setRotation(SCREEN_ROTATION);// was 1 - PP added 420c Z96
  display.setFont(&FreeMonoBold9pt7b);
  //display.setTextColor(GxEPD_BLACK);
  display.setTextColor(GxEPD_RED);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 10);
    display.println(message);
  }
  while (display.nextPage());
}

// Draw a bitmap image onto epaper
void drawBitmaps3c400x300(GxEPD2_GFX& display,uint16_t target_image)
{
  display1.init(115200); // enable diagnostic output on Serial -WARNING: this disables LEDs
  display1.mirror(true); //PP added 420c Z96
  display.setFullWindow();
  display.setRotation(SCREEN_ROTATION);

  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c400x300_1_black, Bitmap3c400x300_1_red},
    {Bitmap3c400x300_2_black, Bitmap3c400x300_2_red}
  };

  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawInvertedBitmap(0, 0, bitmap_pairs[target_image].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
    display.drawInvertedBitmap(0, 0, bitmap_pairs[target_image].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
  }
  while (display.nextPage());


  // Initialise LEDs again
  pinMode(MOSFET_GATE, OUTPUT); digitalWrite(MOSFET_GATE, HIGH);
  strip.begin();
  strip.setBrightness(LOW_BRIGHTNESS);
}

// Input a value 0 to 255 to get a color value. The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Wheel of fortune LED animation
void colorWheelFortune(uint32_t target_LED) {
  //target_LED=target_LED%(5*strip.numPixels());  // Ensures that fortune wheel will never make more than 5 turns
  uint16_t min_delay=5;
  uint16_t max_delay=100;
  float decay=0.9;
  int time_delay=0;
  // Spin LED to target position
  for(uint16_t i=0; i<=target_LED; i++) {
    time_delay=int(min_delay+max_delay*pow(decay,target_LED-i));
    strip.setPixelColor((i+1)%strip.numPixels(), Wheel((((i+1)%strip.numPixels() * 256 / strip.numPixels())) & 255));
    strip.setPixelColor((i-2)%strip.numPixels(), strip.Color(0, 0, 0));
    strip.show();
    delay(time_delay);
  }
  // Spread color to complete ring
  uint16_t pos=target_LED%strip.numPixels()+strip.numPixels();
  for(uint16_t i=0; i<=strip.numPixels()/2; i++) {
    time_delay=int(min_delay+max_delay*pow(decay,strip.numPixels()/2-i));
    strip.setPixelColor((pos+i)%strip.numPixels(), Wheel((((pos)%strip.numPixels() * 256 / strip.numPixels())) & 255));
    strip.setPixelColor((pos-i)%strip.numPixels(), Wheel((((pos)%strip.numPixels() * 256 / strip.numPixels())) & 255));
    strip.show();
    delay(time_delay);
  }

}

// Set all LEDs to the same color, one after the other, with a visible delay.
void colorTransientWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(20);
  }
}

// Set all LEDs to different colors covering a full rainbow.
void rainbowCycle() {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
  }
}

// Define a few LED apperns for easy use later + control of MOSFET powering LEDs.
void LED(String pattern){

  if (pattern=="deco"){
    digitalWrite(MOSFET_GATE,HIGH);
    rainbowCycle();
  }

  if (pattern=="off"){
    digitalWrite(MOSFET_GATE,LOW);
  }

  // send the 'leds' array out to the actual LED strip
  strip.show();
  // insert a delay to keep the framerate modest
  delay(1000/FRAMES_PER_SECOND);
}

// Connect to WiFi: try credentials in config.json or start as access point.
void wifiConnect()
{
  //reset networking
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();
  delay(1000);
  //check for stored credentials

  if(SPIFFS.exists("/config.json")){
    Serial.println("Found /config.json file");
    const char * _ssid = "", *_pass = "";
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();

      DynamicJsonBuffer jsonBuffer;
      JsonObject& jObject = jsonBuffer.parseObject(buf.get());
      if(jObject.success())
      {
        _ssid = jObject["ssid"];
        _pass = jObject["password"];
        Serial.println(_ssid); Serial.println(_pass);
        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid, _pass);
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
          delay(500);
          Serial.print(".");
          if ((unsigned long)(millis() - startTime) >= 10000) break;
        }
        current_ssid=WiFi.SSID();
        current_ip=WiFi.localIP().toString();
      }
    }
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, netmask);
    WiFi.softAP(mySsid, NULL);  // Creating a WiFi network with no password
    Serial.println("Unable to connect to a known wifi network - Starting ESP as Access Point.");
    wifi_status="Access Point";
    current_ssid=mySsid;
    current_ip="192.168.168.168";
  }
  else {Serial.println(""); Serial.println("ESP now connected in STAtion mode to known network.");wifi_status = "Station";}
}

// Triggered by webpage when WiFi connection credentials are updated.
void handleSettingsUpdate()
{
  String data = server.arg("plain");
  DynamicJsonBuffer jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);

  File configFile = SPIFFS.open("/config.json", "w");
  jObject.printTo(configFile);
  configFile.close();

  server.send(200, "application/json", "{\"status\" : \"ok\"}");
  delay(500);

  wifiConnect();
}

// Triggered by webpage when a text message is sent to be displayed onto epaper..
void handleForm() {
 String messageWeb = server.arg("message_web");

 Serial.print("Message venu du web:");
 Serial.print("Before reformating: ");Serial.println(messageWeb);
 messageWeb=formatMessage(messageWeb);  // Modify message to replace letters not in default font and avoid breaking words at end of number_of_lines
Serial.print("After reformating: ");Serial.println(messageWeb);
 print_string(display1,messageWeb);

 //String s = "<a href='/'> Go Back </a>";
 //server.send(200, "text/html", s); //Send web page
 LED("off");
 delay(500);

 display1.powerOff();
 Serial.println("Job done! Now starting deep sleep.");
 ESP.deepSleep(0);
}

// Display setup info on a single epaper dashboard.
void displayInfo(GxEPD2_GFX& display, String current_fw, String current_ip, String current_ssid, int code_run_counter, int current_citation, int number_of_citations, float batteryVoltage){
  display1.init(115200); // enable diagnostic output on Serial -WARNING: this disables LEDs
  display.setFullWindow();
  display.setRotation(SCREEN_ROTATION);
  display.mirror(true); //PP added 420c Z96
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    // Draw boxes with round corners
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(2, 20);
    display.print("Version FW: "+ current_fw);
    display.setCursor(2, 40);
    //display.print("Citation affich}e: "+String(current_citation%number_of_citations));
    display.print("Ex}cutions: "+ String(code_run_counter));
    display.setCursor(2, 60);
    display.print("Citations en m}moire: "+String(number_of_citations));
    display.setCursor(2, 80);
    display.print("SSID: "+ current_ssid);
    display.setCursor(2, 100);
    display.setTextColor(GxEPD_RED);
    display.print("IPaddress: "+ current_ip);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(2, 120);
    display.print("Batterie(V):  "+String(batteryVoltage));
  }
  while (display.nextPage());
}

// Read targeted row from target text file.
String GetCitation()
{
  citation="Houston we've got a problem!";  // Default string displayed if code fails to retrieve expected text.
  char char_filepath[50];  // Char array to store filepath - max 50 chars
  String filepath;  // String to store filepath
  filepath="/data/" + filename;
  filepath.toCharArray(char_filepath,filepath.length()+1);
  File myDataFile = SPIFFS.open(char_filepath, "r");              // Open the file again, this time for reading
  if (!myDataFile) Serial.println("file open failed");  // Check for errors
  int entry=0;

if (filename==filename_1){current_citation=current_citation_1; number_of_citations=number_of_citations_1;}
if (filename==filename_2){current_citation=current_citation_2; number_of_citations=number_of_citations_2;}
if (filename==filename_3){current_citation=current_citation_3; number_of_citations=number_of_citations_3;}
if (filename==filename_4){current_citation=current_citation_4; number_of_citations=number_of_citations_4;}
if (method4next=="Sequentiel"){target_citation=current_citation%number_of_citations;}
else {
  target_citation=random(0, number_of_citations-1);
  current_citation_1=target_citation;
}

  Serial.print("target_citation: ");Serial.println(target_citation);
  while (entry<target_citation){
    entry=entry+1;
    if(myDataFile.available()){citation=myDataFile.readStringUntil('\n');}
  }
  myDataFile.close();    // Close the file
  return citation;
}

// Triggered by web page when method to set next row is changed. Write modified setting into json log. Includes a dirty patch to avoid toggling method when webpage is loaded.
void change_method4next(){
  if (first_time==false){
    if (method4next=="Aleatoire"){method4next="Sequentiel";}
    else {method4next="Aleatoire";}
    Serial.print("Méthode pour prochaine citation:"); Serial.println(method4next);
    // Update log.json with modified method4next value
    update_log_data();
  }
  else {first_time=false;}
  server.send(200,"text/plain", method4next);
}

void setup()
{
  Serial.begin(115200);
  delay(50);

  Serial.println("Start setup.");

  // If starting on switch pressing then battery voltage can be measured. Do so and calculate battery voltage by averaging 25 readings.
  if (analogRead(analogInPin)>100){
    vbat_counter=0;
    for(int i=0; i<25; i++) {
      if (analogRead(analogInPin)>100){
        vbat_counter++;
        sensorValue = sensorValue+analogRead(analogInPin);
        delay(10);
      }
    }
    batteryVoltage=sensorValue/(batteryVoltageCalibration*vbat_counter);
    Serial.print("Battery voltage: "); Serial.print(batteryVoltage); Serial.println("V");
  }

  //SPIFFS.begin();

  Serial.println("(1) Setup LED strip.");
  // Initialize MOSFET_GATE used to turn LED strip power on/off
  pinMode(MOSFET_GATE, OUTPUT); digitalWrite(MOSFET_GATE, HIGH);

  // Initialize LED strip and set all pixels to BLUE
  strip.begin();
  strip.setBrightness(LOW_BRIGHTNESS);
  //colorTransientWipe(strip.Color(0, 0, 255));

  random_pickup=int(random(60,400));
  colorWheelFortune(random_pickup);

  Serial.print("random_pickup: "); Serial.println(random_pickup);
  if ((random_pickup/12)%4==0){filename=filename_1;}
  if ((random_pickup/12)%4==1){filename=filename_2;}
  if ((random_pickup/12)%4==2){filename=filename_3;}
  if ((random_pickup/12)%4==3){filename=filename_4;}
  Serial.println(filename);
  delay(2000);

  Serial.println("(2) Setup SPIFFS file system.");
  SPIFFS.begin();
  if (!SPIFFS.begin()) { Serial.println("SPIFFS failed");
    } else {
      Serial.println("Content of SPIFFS memory:");
      Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        String filepath = dir.fileName();
        //if (dir.fileName()=="/data/citations.txt"){citations_file_exists=true;}
        if (dir.fileName()=="/data/"+filename_1){citations_file_1_exists=true;}
        if (dir.fileName()=="/data/"+filename_2){citations_file_2_exists=true;}
        if (dir.fileName()=="/data/"+filename_3){citations_file_3_exists=true;}
        if (dir.fileName()=="/data/"+filename_4){citations_file_4_exists=true;}
        size_t fileSize = dir.fileSize();
        Serial.printf(" FS File: %s, size: %s\n", filepath.c_str(), formatBytes(fileSize).c_str());
        if (dir.fileName()=="/log.json" && fileSize<50){
          Serial.println("Found a suspicious log.json file because it is too small - Will delete.");
          SPIFFS.remove("/log.json");
          Serial.println("Now removed!");
        }
        // Now repeat with backup file
        if (dir.fileName()=="/log_bkp.json" && fileSize<50){
          Serial.println("Found a suspicious log_bkp.json file because it is too small - Will delete.");
          SPIFFS.remove("/log_bkp.json");
          Serial.println("Now removed!");
        }
      }
    }


  Serial.println("(3) Setup ePaper.");
  // ePaper display reset
  digitalWrite(RST_PIN, HIGH);
  pinMode(RST_PIN, OUTPUT);
  delay(20);
  digitalWrite(RST_PIN, LOW);
  delay(20);
  digitalWrite(RST_PIN, HIGH);
  delay(200);


  Serial.println("(4) Display bitmaps if needed.");
  if ((batteryVoltage<=LowBattWarningLevel)&(batteryVoltage>0.5)&(vbat_counter>=20)){
    //drawBitmaps3c400x300(display1,0);
    colorTransientWipe(strip.Color(255, 0, 0));
    delay(5000);
    if (batteryVoltage<=LowBattAlarmLevel){
      display1.powerOff();
      Serial.println("Insufficient battery! Now starting deep sleep.");
      ESP.deepSleep(0);
    }
  }

  if (batteryVoltage<=0.5){
    //drawBitmaps3c400x300(display1,0);
    colorTransientWipe(strip.Color(0, 255, 0));
    delay(5000);
  }

  Serial.println("(5) Read and update execution counters from SPIFFS.");
  read_log_data();

  if ((number_of_citations_1==1)&(citations_file_1_exists)){ // If _number_of_citations is not red fron log file and citations.txt file exists then read it to update number of citations counter
    colorTransientWipe(strip.Color(255, 255, 0));
    filename=filename_1;
    ReadDataFile();
  }
  if ((number_of_citations_2==1)&(citations_file_2_exists)){ // If _number_of_citations is not red fron log file and citations.txt file exists then read it to update number of citations counter
    colorTransientWipe(strip.Color(255, 255, 0));
    filename=filename_2;
    ReadDataFile();
  }
  if ((number_of_citations_3==1)&(citations_file_3_exists)){ // If _number_of_citations is not red fron log file and citations.txt file exists then read it to update number of citations counter
    colorTransientWipe(strip.Color(255, 255, 0));
    filename=filename_3;
    ReadDataFile();
  }
  if ((number_of_citations_4==1)&(citations_file_4_exists)){ // If _number_of_citations is not red fron log file and citations.txt file exists then read it to update number of citations counter
    colorTransientWipe(strip.Color(255, 255, 0));
    filename=filename_4;
    ReadDataFile();
  }

  while (millis()<2000){
    Serial.print(".");
    delay(100);
  } // Wait here until at least 2 seconds have elapsed since stratup then test if switch is still pressed
  LED("deco");
  Serial.println(":");
  Serial.println(" Ready to move on.");

  // read the analog in value
  sensorValue = analogRead(analogInPin);
  if (sensorValue>100){
    SetupMode=true;
    Serial.print("Time up to here: ");Serial.println(millis());
    Serial.println("(5) Setup Wifi.");
    // Connect to wifi, either in Access Point or STAtion mode
    wifiConnect();

    if (wifi_status=="Station"){
      colorTransientWipe(strip.Color(255, 255, 0));
      delay(1000);
    }

    if (wifi_status=="Access Point"){
      colorTransientWipe(strip.Color(0, 255, 255));
      delay(1000);
    }

    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/html", webpage);
    });
    server.on("/action_page", handleForm); //form action is handled here

    server.onFileUpload([]() {
      if (server.uri() != "/update") return;
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        filename = upload.filename;
        //filename="citations.txt";
        Serial.print("Upload Name: "); Serial.println(filename);
        UploadFile = SPIFFS.open("/data/" + filename, "w");
        //UploadFile = SPIFFS.open("/data/citations.txt", "w");
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (UploadFile)
          UploadFile.write(upload.buf, upload.currentSize);
      } else if (upload.status == UPLOAD_FILE_END) {
        if (UploadFile)
          UploadFile.close();
          ReadDataFile();  // After file downloads, read it
      }

    });

    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    });

    server.on("/settings", HTTP_POST, handleSettingsUpdate);
    server.on("/change_method4next",change_method4next);

    server.begin();
    Serial.println("Webserver running");
    //drawTargetBitmap400x300(display1,2);
    //delay(1000);
    //drawTargetBitmap400x300(display1,0);
    //delay(1000);
    displayInfo(display1,fw_version,current_ip,current_ssid,code_run_counter, current_citation_1,number_of_citations_1, batteryVoltage);
  }

  Serial.println("Setup done.");

  start_time=millis();
}

void loop()
{
  LED("deco");

  if (SetupMode==true){
    server.handleClient();
    delay(1);
  }

  if ((millis()-start_time>LED_ANIMATIONS_DURATION)||!SetupMode){
    LED("off");
    delay(50);


    int attempts=1;
    while ((!citation_fits)&(attempts<10)){
      Serial.println("Attempt number "+String(attempts));
      GetCitation();
      citation=formatMessage(citation);  // Modify message to avoid breaking words at end of number_of_lines
      attempts=attempts+1;
      current_citation_1=current_citation_1+1;
    }

    print_string(display1,citation);
    code_run_counter=code_run_counter+1;
    update_log_data();
    delay(500);

    display1.powerOff();
    Serial.println("Job done! Now starting deep sleep.");
    ESP.deepSleep(0);
  }
}
