 /*
  General Purpose Webserver adjusted for Olimex ESP32-PoE board.
  It can control Olimex modules: MOD-IO; MOD-LCD 4.3"; MOD-LTR501; MOD-BME280, MOD-IRDA, MOD-RFID1356-MIFARE.
  This sketch requires Arduino for ESP32 - how to install it read here: https://github.com/espressif/arduino-esp32

  This project is based on the one created by David Bird. The original can be found and downloaded here: https://github.com/G6EJD/ESP32-General-Purpose-Webserver

  The MIT License (MIT) Copyright (c) 2017 by David Bird.
  ### The formulation and calculation method of an IAQ - Internal Air Quality index ###
  ### The provision of a general purpose webserver ###
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
  publish, distribute, but not to use it commercially for profit making or to sub-license and/or to sell copies of the Software or to
  permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  See more at http://dsbird.org.uk
*/


//################# LIBRARIES ################
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32WebServer.h>  //https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include "Adafruit_BME280.h" // search in library packages of Arduino for: "Adafruit BME280 Library" and install it
#include <IRremote.h> // https://github.com/z3t0/Arduino-IRremote
#include <string.h>


enum UART_Modules_Type
{
  NONE = 0, MOD_IRDA, MOD_RFID_MIFARE, MOD_LCD
};


//################ VARIABLES ################
const char* ssid      = "OlimexTenda";     // WiFi SSID
const char* password  = "0pen5ourceHardware"; // WiFi Password

String siteheading    = "ESP32-PoE";               // Site's Main Title
String sitetitle      = "ESP32-PoE Webserver";       // Appears on the tabe of a Web Browser
String yourfootnote   = "Olimex demo for ESP32-PoE"; // A foot note e.g. "My Web Site"
String siteversion    = "v1.3";  // Version of your Website


#define sitewidth  1024  // Adjust site page width in pixels as required
#define SEALEVELPRESSURE_HPA (1013.25)  // required for MOD-BME280
int MOD_IRDA_RECV_PIN = 36;

UART_Modules_Type Active_UART_Module = NONE;
Adafruit_BME280 bme; // I2C
IRrecv irrecv(MOD_IRDA_RECV_PIN);
decode_results MOD_IRDA_results;
const char MOD_IRDA_Decode_Type [][16] = {"UNUSED", "RC5", "RC6", "NEC", "SONY", "PANASONIC", "JVC", "SAMSUNG", "WHYNTER",
                                          "AIWA_RC_T501", "LG", "SANYO", "MITSUBISHI", "DISH", "SHARP", "DENON", "PRONTO", "LEGO_PF"};

String webpage = ""; // General purpose variable to hold HTML code

ESP32WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 perhaps, if your Router uses port 80
// To access server from outside of a WiFi (LAN) network e.g. on port 8080 add a rule on your Router that forwards a connection request
// to http://your_network_WAN_address:8080 to http://your_network_LAN_address:8080 and then you can view your ESP server from anywhere.
// Example http://yourhome.ip:8080 and your ESP Server is at 192.168.0.40, then the request will be directed to http://192.168.0.40:8080


#define BUTTON_PRESSED()  (!digitalRead (34))

void MOD_IO_SetRelay (int Value)
{
  Wire.beginTransmission(0x58);
  Wire.write(0x10);
  Wire.write(Value);
  Wire.endTransmission();
}

void MOD_LTR501_Write(unsigned char reg, unsigned char data)
{
  Wire.beginTransmission(0x23);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

unsigned char MOD_LTR501_Read(unsigned char reg)
{
  unsigned char data;
  
  Wire.beginTransmission(0x23);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom(0x23, 1);
  while(Wire.available())
    data = Wire.read();
  
  return data;
}

int MOD_LTR501_Available (void)
{
  if(MOD_LTR501_Read(0x86) == 0x80)
    return 1;
  else
    return 0;
}

char  MOD_IRDA_Type[16]=" ", MOD_IRDA_Code[10]=" ";
void MOD_IRDA_loop_Routine()
{
  if (Active_UART_Module == MOD_IRDA)
  {
    if (irrecv.decode(&MOD_IRDA_results))
    {
      if ((MOD_IRDA_results.decode_type != UNKNOWN) && (MOD_IRDA_results.value != 0xFFFFFFFF))
      {
        sprintf (MOD_IRDA_Type, MOD_IRDA_Decode_Type[MOD_IRDA_results.decode_type]);
        sprintf (MOD_IRDA_Code, "%X", MOD_IRDA_results.value);
      }
      irrecv.resume(); // Receive the next value
    }
  }
}

void AJAX_Data_Function()
{
  static char MOD_MIFARE_Text_Output[32+1]="";
  static char MOD_LCD_Text_Output[256+1]="";
  /* server responds 200 with a json payload */
  /* although preferably concatenate your real sensor data here */
  String message="{";
  // MOD-BME
  if (bme.begin(0x76))
  {
    message += "\"BME_s\": ";
    message += "\"Connected\"";
    message += ", \"BME_t\": ";
    message += bme.readTemperature();
    message += ", \"BME_p\": ";
    message += bme.readPressure()/100.0F;
    message += ", \"BME_a\": ";
    message += bme.readAltitude(SEALEVELPRESSURE_HPA);
    message += ", \"BME_h\": ";
    message += bme.readHumidity();
  }
  else
  {
    message += "\"BME_s\": ";
    message += "\"Disconnected\"";
    message += ", \"BME_t\": ";
    message += 0;
    message += ", \"BME_p\": ";
    message += 0;
    message += ", \"BME_a\": ";
    message += 0;
    message += ", \"BME_h\": ";
    message += 0;
  }

  // MOD-LTR
  static unsigned int ADC_0=0, ADC_1=0, LTR_PS=0;
  static float distance=0;
  if (MOD_LTR501_Available())
  {
    int stat;

    MOD_LTR501_Write(0x80, 0x03);  //Active mode, 64k lux range
    MOD_LTR501_Write(0x81, 0x03);  //PS active mode, x1 GAIN
    MOD_LTR501_Write(0x82, 0x6B);  //LED 60Hz, 50% duty, 50mA
    MOD_LTR501_Write(0x83, 0x7F);  //127 pulses
    MOD_LTR501_Write(0x84, 0x02);  //PS 100ms measure rate
    MOD_LTR501_Write(0x85, 0x03);  //ALS Integration 100ms, repeat rate 500ms
    stat = MOD_LTR501_Read(0x8C);
    if (stat & 0x04)
    {
      unsigned char data[4];
      for(int i = 0; i < 4; i++)
      {
        data[i] = MOD_LTR501_Read(0x88 + i);
      }
      ADC_0 = (data[3] << 8) | data[2];
      ADC_1 = (data[1] << 8) | data[0];
    }

    if(stat & 0x01)
    {
      //PS new data
      unsigned char data[2];
      for(int i = 0; i < 2; i++)
      {
        data[i] = MOD_LTR501_Read(0x8D + i);
      }
      LTR_PS = (data[1] << 8) | data[0];
      distance = 10 -(10.0/2047)*LTR_PS;
    }
    message += ", \"LTR_s\": ";
    message += "\"Connected\"";
    message += ", \"LTR_a0\": ";
    message += ADC_0;
    message += ", \"LTR_a1\": ";
    message += ADC_1;
    message += ", \"LTR_d\": ";
    message += distance;
  }
  else
  {
    message += ", \"LTR_s\": ";
    message += "\"Disconnected\"";
    message += ", \"LTR_a0\": ";
    message += 0;
    message += ", \"LTR_a1\": ";
    message += 0;
    message += ", \"LTR_d\": ";
    message += 0;
  }

  // MOD-IrDA
  if (Active_UART_Module == MOD_IRDA)
  {
    message += ", \"IRDA_s\": ";
    message += "\"Connected\"";
    message += ", \"IRDA_t\": ";
    message += "\"";
    message += MOD_IRDA_Type;
    message += "\"";
    message += ", \"IRDA_c\": ";
    message += "\"";
    message += MOD_IRDA_Code;
    message += "\"";
  }
  else
  {
    message += ", \"IRDA_s\": ";
    message += "\"Disconnected\"";
    message += ", \"IRDA_t\": ";
    message += "\"\"";
    message += ", \"IRDA_c\": ";
    message += "\"\"";
  }

  // MOD-MIFARE
  if (Active_UART_Module == MOD_RFID_MIFARE)
  {
    int i=0, Flag=0;
    if (Serial2.available () > 0)
    {
      while (Serial2.available () > 0)
      {
        char TempChar;
        TempChar = (char)Serial2.read();
        if ((TempChar == '\n') || (TempChar == '\r'))
        {
          MOD_MIFARE_Text_Output[i] = 0;
          Flag = 0;
        }
        if (Flag)
        {
          if (i<32)
            MOD_MIFARE_Text_Output[i++] = TempChar;
        }
        if (TempChar == '-')
        {
          i = 0;
          Flag = 1;
        }
      }
      MOD_MIFARE_Text_Output[i] = 0;
    }
    message += ", \"MIFARE_s\": ";
    message += "\"Connected\"";
    message += ", \"MIFARE_d\": ";
    message += "\"";
    message += MOD_MIFARE_Text_Output;
    message += "\"";
  }
  else
  {
    MOD_MIFARE_Text_Output[0] = 0;
    message += ", \"MIFARE_s\": ";
    message += "\"Disconnected\"";
    message += ", \"MIFARE_d\": ";
    message += "\"\"";
  }

  // MOD-LCD
  if (Active_UART_Module == MOD_LCD)
  {
    int i=0;
    if (Serial2.available () > 0)
    {
      while (Serial2.available () > 0)
      {
        char TempChar;
        TempChar = (char)Serial2.read();
        if (i>250)  // 255 is the maximum of buffer
          continue;
        
        switch (TempChar)
        {
          case '\r':
            break;
          case '\n':
            MOD_LCD_Text_Output[i++] = '\\';
            MOD_LCD_Text_Output[i++] = 'n';
            break;
          case '"':
            MOD_LCD_Text_Output[i++] = '\\';
            MOD_LCD_Text_Output[i++] = '"';
            break;
          default:
            MOD_LCD_Text_Output[i++] = TempChar;
            break;
        }
      }
      MOD_LCD_Text_Output[i] = 0;
    }
    message += ", \"LCD_s\": ";
    message += "\"Connected\"";
    message += ", \"LCD_o\": ";
    message += "\"";
    message += MOD_LCD_Text_Output;
    message += "\"";
  }
  else
  {
    MOD_LCD_Text_Output[0] = 0;
    message += ", \"LCD_s\": ";
    message += "\"Disconnected\"";
    message += ", \"LCD_o\": ";
    message += "\"\"";
  }

  message += "}";
  server.send(200, "application/json",message);
}

void setup()
{
  Serial.begin(115200); // initialize serial communications
  Serial2.begin (115200, SERIAL_8N1, 36, 4, false); // UEXT UART
  Wire.begin (13, 16);  // SPP
    
  Serial.println("Connect your I2C sensors to the default SDA, SCL pins for your board shown here:");
  Serial.println("I2C SDA pin = " + String(SDA));
  Serial.println("I2C SCL pin = " + String(SCL)); // Connect I2C sensors to the default SDA and SCL pins! Check Serial port for details
  StartWiFi(ssid, password);
  //---------------------------------------------------------------------- 
  Serial.println("Use this URL to connect: http://" + WiFi.localIP().toString() + "/"); // Print the IP address
  server.on("/",          I2C_Modules);   // If the user types at their browser http://192.168.0.100/ control is passed here and then to user_input, you get values for your program...
  server.on("/I2C_Modules",  I2C_Modules);
  server.on("/UART_Modules",  UART_Modules);
  server.on("/AJAX_Data", AJAX_Data_Function);  // AJAX data

  server.onNotFound(handleNotFound);   // If the user types something that is not supported, say so
  server.begin(); Serial.println(F("Webserver started...")); // Start the webserver
  irrecv.enableIRIn(); // Start the receiver
  pinMode (34, INPUT);
}

void loop ()
{
  server.handleClient();
  if (Active_UART_Module == MOD_IRDA)
    MOD_IRDA_loop_Routine ();
}

void handleNotFound()
{
  String message = "The request entered could not be found, please try again with a different option\n";
  server.send(404, "text/plain", message);
}

void I2C_Modules ()
{
  String MOD_IO_CheckBoxChoice = "";
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<div id=\"i2c_container\"> <h1>I2C modules</h1>";

  // MOD-IO
  webpage += "<form method=\"POST\" id=\"mod-io\" action=\"/I2C_Modules\"> <fieldset> <legend>MOD-IO</legend>";
  webpage += "<div class=\"field\"> <label for=\"mod-io-r1\">Relay 1</label> <input type=\"checkbox\" id=\"Relay1\" name=\"MOD_IO_CheckBox\" value=\"1\"/> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-io-r2\">Relay 2</label> <input type=\"checkbox\" id=\"Relay2\" name=\"MOD_IO_CheckBox\" value=\"2\"/> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-io-r3\">Relay 3</label> <input type=\"checkbox\" id=\"Relay3\" name=\"MOD_IO_CheckBox\" value=\"3\"/> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-io-r4\">Relay 4</label> <input type=\"checkbox\" id=\"Relay4\" name=\"MOD_IO_CheckBox\" value=\"4\"/> </div>";
  webpage += "<button type='submit' id=\'MOD_IO_Relay\' name='MOD_IO_Relay'  value='submit-true'> Set Relay </button>";
  webpage += "</fieldset> </form>";

  // MOD-BME280
  webpage += "<form method=\"POST\" id=\"mod-bme\" action=\"/I2C_Modules\"> <fieldset> <legend>MOD-BME280</legend>";
  webpage += "<label class=\"field\"><span id='BME_s'></span></label>";
  webpage += "<div class=\"field\"> <label for=\"mod-bme-t\">Temperature</label> <div id=\"mod-bme-t\" class=\"value\"><label class=\"field\"><span id='BME_t'></span>&deg</label></div> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-bme-p\">Pressure</label> <div id=\"mod-bme-p\" class=\"value\"><label class=\"field\"><span id='BME_p'></span> hPa</label></div> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-bme-a\">Altitude</label> <div id=\"mod-bme-a\" class=\"value\"><label class=\"field\"><span id='BME_a'></span> m</label></div> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-bme-h\">Humidity</label> <div id=\"mod-bme-h\" class=\"value\"><label class=\"field\"><span id='BME_h'></span> %</label></div> </div>";
  webpage += "</fieldset> </form>";

  // MOD-LTR501
  webpage += "<form method=\"POST\" id=\"mod-ltr\" action=\"/I2C_Modules\"> <fieldset> <legend>MOD-LTR501</legend>";
  webpage += "<label class=\"field\"><span id='LTR_s'></span></label>";
  webpage += "<div class=\"field\"> <label for=\"mod-ltr-a0\">ADC0</label> <div id=\"mod-ltr-a0\" class=\"value\"><label class=\"field\"><span id='LTR_a0'></span></label></div> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-ltr-a1\">ADC1</label> <div id=\"mod-ltr-a1\" class=\"value\"><label class=\"field\"><span id='LTR_a1'></span></label></div> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-ltr-d\">Distance</label> <div id=\"mod-ltr-d\" class=\"value\"><label class=\"field\"><span id='LTR_d'></span></label></div> </div>";
  webpage += "</fieldset> </form>";
  
  webpage += "</div>";  // container

  // And so-on
  webpage += "</body>";
  append_HTML_footer();

  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  if (server.args() > 0 ) 
  { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      static int MOD_IO_Temp=0, MOD_IO_RelayValue = 0;
      String Argument_Name   = server.argName(i);
      String client_response = server.arg(i);
      if (Argument_Name == "MOD_IO_CheckBox")
      {
        MOD_IO_CheckBoxChoice = client_response; // Checking for more than one check-box being selected too, 'a' if more than one
        MOD_IO_Temp = MOD_IO_CheckBoxChoice.toInt () - 1; // SPP
        MOD_IO_RelayValue = MOD_IO_RelayValue | (1 << MOD_IO_Temp);
        Serial.println (MOD_IO_RelayValue);
        MOD_IO_SetRelay (MOD_IO_RelayValue);
      }

      if (Argument_Name == "MOD_IO_Relay")
      {
        Serial.print ("Value: ");
        Serial.println (MOD_IO_RelayValue);
        MOD_IO_SetRelay (MOD_IO_RelayValue);
        MOD_IO_RelayValue = 0;
      }
    }
  }
}

void UART_Modules ()
{
  String MOD_LCD_Text;
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  // UART Modules  
  webpage += "<div id=\"uart_container\"> <h1>UART modules</h1>";
  //webpage += "<form method=\"POST\" id=\"buttons\" action=\"/UART_Modules\"> <fieldset> <legend>Select module</legend>";
  //webpage += "UART modules can't operate together because they work on different baud rate and share same RX and TX. Initialize one of them:<br>";
  webpage += "<form method=\"POST\" id=\"buttons\" action=\"/UART_Modules\"> <fieldset>";
  webpage += "Initialize one of the modules:<br>";
  webpage += "<button type='submit' id=\'Init_MOD_IRDA\' name='Init_MOD_IRDA'  value='submit-true'>MOD-IRDA</button> ";
  webpage += "<button type='submit' id=\'Init_MOD_RFID_MIFARE\' name='Init_MOD_RFID_MIFARE'  value='submit-true'>MOD-MIFARE</button> ";
  webpage += "<button type='submit' id=\'Init_MOD_LCD\' name='Init_MOD_LCD'  value='submit-true'>MOD-LCD 4.3\"</button><br>";
  webpage += "</fieldset> </form>";

  // MOD-IRDA
  webpage += "<form method=\"POST\" id=\"mod-irda\" action=\"/UART_Modules\"> <fieldset> <legend>MOD-IRDA</legend>";
  webpage += "<label class=\"field\"><span id='IRDA_s'></span></label>";
  webpage += "<div class=\"field\"> <label for=\"mod-irda-t\">Type</label> <div id=\"mod-irda-t\" class=\"value\"><label class=\"field\"><span id='IRDA_t'></span></label></div> </div>";
  webpage += "<div class=\"field\"> <label for=\"mod-irda-c\">Code</label> <div id=\"mod-irda-c\" class=\"value\"><label class=\"field\"><span id='IRDA_c'></span></label></div> </div>";
  webpage += "</fieldset> </form>";

  // MOD-MIFARE
  webpage += "<form method=\"POST\" id=\"mod-mifare\" action=\"/UART_Modules\"> <fieldset> <legend>MOD-RFID1356-MIFARE</legend>";
  webpage += "<label class=\"field\"><span id='MIFARE_s'></span></label>";
  webpage += "<div class=\"field\"> <label for=\"mod-mifare-d\">Data</label> <div id=\"mod-mifare-d\" class=\"value\"><label class=\"field\"><span id='MIFARE_d'></span></label></div> </div>";
  webpage += "</fieldset> </form>";

  // MOD-LCD
  webpage += "<form method=\"POST\" id=\"mod-lcd\" action=\"/UART_Modules\"> <fieldset> <legend>MOD-LCD 4.3\"</legend>";
  webpage += "<label class=\"field\"><span id='LCD_s'></span></label>";
  
  webpage += "<label for='MOD_LCD_Text'>Input:</label>";
  webpage += "<input type='text' name='MOD_LCD_Text' value=''></td>";
  webpage += "<button type='submit' name='MOD_LCD_Button'  value='submit-true'> Send </button><br>";
  webpage += "<div class=\"field\"> <label for=\"mod-lcd-o\">Output</label> <div id=\"mod-lcd-o\" class=\"value\"><span id='LCD_o'></span></div> </div>";

  webpage += "</fieldset> </form>";
  
  webpage += "</div>"; // div container
  webpage += "</fieldset> </form>";  

  // And so-on
  webpage += "</body>";
  append_HTML_footer();

  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  if (server.args() > 0 )
  { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      String Argument_Name   = server.argName(i);
      String client_response = server.arg(i);
      
      if (Argument_Name == "MOD_LCD_Text")
        MOD_LCD_Text = client_response;

      if (Argument_Name == "MOD_LCD_Button")
      {
        if (Active_UART_Module == MOD_LCD)
        {
          Serial2.print (MOD_LCD_Text);
          Serial2.print ("\n\r");
        }
      }
      
      if (Argument_Name == "Init_MOD_LCD")
      {
        Serial2.begin (115200, SERIAL_8N1, 36, 4, false); // UEXT UART
        Serial2.print ("\n\r");
        Active_UART_Module = MOD_LCD;
      }
      
      if (Argument_Name == "Init_MOD_IRDA")
      {
        Active_UART_Module = MOD_IRDA;
      }
      
      if (Argument_Name == "Init_MOD_RFID_MIFARE")
      {
        Serial2.begin (38400, SERIAL_8N1, 36, 4, false); // UEXT UART
        Active_UART_Module = MOD_RFID_MIFARE;
      }
    }
  }
}

void StartWiFi(const char* ssid, const char* password)
{
  int connAttempts = 0;
  Serial.print(F("\r\nConnecting to: ")); Serial.println(String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED )
  {
    delay(500); Serial.print(".");
    if (connAttempts > 20) {
      Serial.println("Failed to connect to WiFi");
    }
    connAttempts++;
  }
  Serial.print(F("WiFi connected at: "));
  Serial.println(WiFi.localIP());
}

void append_HTML_header()
{
  webpage  = "";
  webpage += "<!DOCTYPE html><html manifest=\"appcache.manifest\"><head>";
  webpage += "<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>"; // AJAX data
  webpage += "<meta http-equiv='refresh' content='300'>"; // 5-min refresh time, test needed to prevent auto updates repeating some commands
  webpage += "\n\
  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>\n\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n\
  <meta name=\"mobile-web-app-capable\" content=\"yes\" />\n";
  webpage += "<style>";
  webpage += "* {\n\
      box-sizing: border-box;\n\
      -moz-box-sizing: border-box;\n\
    }\n\
    \n\
    #menu {\n\
      margin-left: auto;\n\
      margin-right: auto;\n\
      width: 360px;\n\
    }\n\
    \n\
    #menu a {\n\
      width: 178px;\n\
      display: inline-block;\n\
      background: lightgrey;\n\
      color: white;\n\
      padding: 8px;\n\
      font-weight: bold;\n\
      border-bottom: 1px solid #fff;\n\
      text-align: center;\n\
      text-decoration: none;\n\
    }\n\
    \n\
    #menu a:hover {\n\
      background: darkgrey;\n\
    }\n\
    body {\n\
      font: 14px Arial, sans-serif;\n\
      line-height: 30px;\n\
    }\n\
    \n\
    #i2c_container {\n\
      position: relative;\n\
      display: block;\n\
      \n\
      border: solid 1px black;\n\
      width: 360px;\n\
      min-height: 470px;\n\
      padding: 10px;\n\
      margin: 0 auto;\n\
    }\n\
    \n\
    #uart_container {\n\
      position: relative;\n\
      display: block;\n\
      \n\
      border: solid 1px black;\n\
      width: 360px;\n\
      min-height: 470px;\n\
      padding: 10px;\n\
      margin: 0 auto;\n\
    }\n\
    #LCD_o {\n\
      display: block;\n\
      white-space: pre;\n\
    }\n\
    footer {\n\
    width: 360px;\n\
    margin: 0 auto;\n\
    font-size: 12px;\n\
    line-height: 1.7;\n\
    }\n\
    \n\
    form {\n\
      display: block;\n\
      vertical-align: top;\n\
      margin: 0;\n\
      width: 100%;\n\
    }\n\
    \n\
    h1 {\n\
      margin: 0;\n\
      margin-bottom: 10px;\n\
      text-align: center;\n\
      font-weight: bold;\n\
      font-size: 20px;\n\
      line-height: 30px;\n\
      background-color: lightgray;\n\
    }\n\
    \n\
    fieldset {\n\
      display: block;\n\
      border: solid 1px black;\n\
      margin-bottom: 30px;\n\
      width: 100%;\n\
      padding: 10px;\n\
      padding-top: 0px;\n\
    }\n\
    \n\
    legend {\n\
      font-size: 18px;\n\
    }\n\
    \n\
    label {\n\
      display: block;\n\
      color: grey;\n\
    }\n\
    \n\
    textarea,\n\
    input,\n\
    select {\n\
      font: 14px Arial, sans-serif;\n\
      line-heigth: 20px;\n\
      padding: 3px;\n\
      width: 100%;\n\
      border: solid 1px lightgrey;\n\
    }\n\
    \n\
    input[type=checkbox] {\n\
      width: 20px;\n\
      height: 20px;\n\
    }\n\
    \n\
    div.field {\n\
      display: inline-block;\n\
      width: 100%;\n\
      margin-right: 10px;\n\
      margin-bottom: 15px;\n\
    }\n\
    \n\
    div.value {\n\
      font: 14px Arial, sans-serif;\n\
      line-heigth: 20px;\n\
      display: inline-block;\n\
      width: 100%;\n\
      border: solid 1px lightgrey;\n\
      padding: 3px;\n\
      width: 100%;\n\
    }\n\
    \n\
    #mod-io div.field {\n\
      width: 20%;\n\
    }\n\
    \n\
    #mod-bme div.field {\n\
      width: 45%;\n\
      margin-right: 10px;\n\
    }\n\
    #mod-ltr div.field {\n\
      width: 30%;\n\
    }\n\
    \n\
    #mod-irda div.field {\n\
      width: 40%;\n\
    }\n\
    \n\
    #mod-mifare div.field {\n\
      width: 40%;\n\
    }\n\
    \n\
    #mod-lcd div.field {\n\
      width: 100%;\n\
    }\n\
    \n";
  webpage += "</style>";
  webpage += "</head>";
  webpage +="\
  <script>\
  $(function()\
  {\
    setInterval(requestData, 3000);\
    function requestData()\
    {\
      $.get(\"/AJAX_Data\")\
        .done(function(data)\
        {\
          if (data)\
          {\
            $(\"#BME_s\").text(data.BME_s);\
            $(\"#BME_t\").text(data.BME_t);\
            $(\"#BME_p\").text(data.BME_p);\
            $(\"#BME_a\").text(data.BME_a);\
            $(\"#BME_h\").text(data.BME_h);\
            $(\"#LTR_s\").text(data.LTR_s);\
            $(\"#LTR_a0\").text(data.LTR_a0);\
            $(\"#LTR_a1\").text(data.LTR_a1);\
            $(\"#LTR_d\").text(data.LTR_d);\
            $(\"#IRDA_s\").text(data.IRDA_s);\
            $(\"#IRDA_t\").text(data.IRDA_t);\
            $(\"#IRDA_c\").text(data.IRDA_c);\
            $(\"#MIFARE_s\").text(data.MIFARE_s);\
            $(\"#MIFARE_d\").text(data.MIFARE_d);\
            $(\"#LCD_s\").text(data.LCD_s);\
            $(\"#LCD_o\").text(data.LCD_o);\
          }\
          else\
          {\
            $(\"#BME_s\").text(\"?\");\
            $(\"#BME_t\").text(\"?\");\
            $(\"#BME_p\").text(\"?\");\
            $(\"#BME_a\").text(\"?\");\
            $(\"#BME_h\").text(\"?\");\
            $(\"#LTR_s\").text(\"?\");\
            $(\"#LTR_a0\").text(\"?\");\
            $(\"#LTR_a1\").text(\"?\");\
            $(\"#LTR_d\").text(\"?\");\
            $(\"#IRDA_s\").text(\"?\");\
            $(\"#IRDA_t\").text(\"?\");\
            $(\"#IRDA_c\").text(\"?\");\
            $(\"#MIFARE_s\").text(\"?\");\
            $(\"#MIFARE_d\").text(\"?\");\
            $(\"#LCD_s\").text(\"?\");\
            $(\"#LCD_o\").text(\"?\");\
          }\
        }).fail(function()\
      {\
      });\
    }\
  });\
</script>";

  webpage += "<body>";
  webpage += "<div class='navbar'>";
  webpage += "</div>";
  webpage += "<br><title>" + sitetitle + "</title><br>";
  webpage += "<div class='main'><h1>" + siteheading + " " + siteversion + "</h1>";
  // For each new page you add or remove, make sure there is a menu item to call it or remove it when not used
  webpage += "<div id='menu'> <a href='/I2C_Modules'>I2C Modules</a> <a href='/UART_Modules'>UART Modules</a> </div>";
}

void append_HTML_footer()
{
  webpage += "<footer><p>" + yourfootnote + "<br>";
  webpage += "&copy; Original project made by David Bird. You can find it here:<br>https://github.com/G6EJD/ESP32-General-Purpose-Webserver</p></footer>";
  webpage += "</div></body></html>";
}

