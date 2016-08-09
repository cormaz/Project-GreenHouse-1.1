/*Automation greenhouse project*/

/*Autors: Antonio La Mura, Umberto Festa*/

/*Date: 21/03/2016*/

/*Our idea is to allow users, who buy fruits and vegetables grown in greenhouses, to know exactly all the plantation steps that products have had,
  such as the use of chemical fertilizers and other similar products.
  A QR code will be put on the sold products that will be read by a specific smartphone app.
  It provides information about environment conditions where products have been grown and the chemical products used.
  The automatic monitoring system in greenhouses is composed of sensors that read environment data and actuators, called Slaves.
  They communicate via Wireless with central device, called Master. The latter sends possible changes to slaves
  (like change of thresholds) and data via WiFi to webserver as well.
  When the product is ready to be sold, QR code generation is requested to the webservice and it will be put on the package.
  The last part of the system is the mobile app that is responsible for the QR codes scanning and shows the relative informations to the user.*/

/*Slave*/

/*Libraries used*/
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <DHT22.h>

//Define all PIN
#define HUMIDIFIER  A4
#define HEATER     A5
#define DHT22_PIN   4
//Define motor
#define  IS_1  0
#define  IS_2  1
#define  IN_1  3
#define  IN_2  11
#define  INH_1 12
#define  INH_2 13

#define TCONST 100	//Delay Time between Steps

//Variable to reset millis()
extern unsigned long timer0_millis;

int status = WL_IDLE_STATUS;
char ssid[] = "Arduino"; //  your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet

WiFiUDP Udp;

//Define DHT22
DHT22 myDHT22(DHT22_PIN);

//Variable to send
float hmin = 0;
float hmax = 0;
float tmin = 0;
float tmax = 0;
int duty_motor = 0;
float humidity;
float temperature;

//Variable to send every 10 seconds
unsigned long interval = 600000;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //Initialize PIN (INPUT - OUTPUT)
  pinMode(HEATER, OUTPUT);
  digitalWrite(HEATER, LOW);
  pinMode(HUMIDIFIER, OUTPUT);
  digitalWrite(HUMIDIFIER, LOW);

  //Set the PIN for the fan
  pinMode(IN_1, OUTPUT);
  pinMode(IN_2, OUTPUT);
  pinMode(INH_1, OUTPUT);
  pinMode(INH_2, OUTPUT);
  //Reset
  reset_ports();
  digitalWrite(INH_1, 1);
  digitalWrite(INH_2, 1);

  Serial.println("Beginning ... ");
  delay(2000);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  unsigned long time = millis();
  unsigned long currentMillis = millis();
  DHT22_ERROR_t errorCode;
  int i = 0;
  char name[] = "clie1";
  humidity = myDHT22.getHumidity() * 10;
  temperature = myDHT22.getTemperatureC() * 10;
  char toSend[32];
  errorCode = myDHT22.readData();
  byte crc;
  int crc_ric;

  //Check sensor humidity and temperature DHT22 errors
  /********************************************************************************/
  switch (errorCode)
  {
    case DHT_ERROR_NONE:
      char buf[128];
      sprintf(buf, "Integer-only reading: Temperature %hi.%01hi C, Humidity %i.%01i %% RH",
              myDHT22.getTemperatureCInt() / 10, abs(myDHT22.getTemperatureCInt() % 10),
              myDHT22.getHumidityInt() / 10, myDHT22.getHumidityInt() % 10);
      break;
    case DHT_ERROR_CHECKSUM:
      break;
    case DHT_BUS_HUNG:
      break;
    case DHT_ERROR_NOT_PRESENT:
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      break;
    case DHT_ERROR_TOOQUICK:
      break;
  }
  /********************************************************************************/

  //Print the values, when it changes
  /********************************************************************************/
  if (humidity != myDHT22.getHumidity() * 10 || temperature != myDHT22.getTemperatureC() * 10) {
    Serial.print("T: ");
    Serial.print(myDHT22.getTemperatureC(), 1);
    Serial.print("C");
    Serial.print(" H: ");
    Serial.print(myDHT22.getHumidity(), 1);
    Serial.println("%");
  }
  /********************************************************************************/

  //Send the parameters every 10 minutes
  /********************************************************************************/
  if (millis() > interval) {
    
    //Connection to AP
    /********************************************************************************/
    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      // don't continue:
      while (true);
    }

    // attempt to connect to Wifi network:
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid);

      // wait 10 seconds for connection:
      delay(10000);
    }
    Serial.println("Connected to wifi");
    /********************************************************************************/
    
    Serial.println("\nStarting connection to server...");
    // if you get a connection, report back via serial:
    Udp.begin(localPort);

    snprintf(toSend, sizeof(toSend), "%s;%d;%d", name, (int)humidity, (int)temperature);
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(toSend);
    Udp.endPacket();

    Serial.println("Finished sending");

    resetMillis();

    while (millis() < 10000) {
      packetSize = Udp.parsePacket();
      if (packetSize) {
        /*Read the packet into packetBuffer*/
        int len = Udp.read(packetBuffer, 255);
        if (len > 0) packetBuffer[len] = 0;
        Serial.println("Contents:");
        Serial.println(packetBuffer);

        char* command = strtok((char *)packetBuffer, ";");
        int count = 0;
        while (command != 0) {

          /*Divide the information*/
          switch (count) {
            case 0:
              snprintf(name, sizeof(name), "%s", command);
              break;

            case 1:
              hmax = atof(command) / 10; //atof(char* ) mi converte un tipo char* in double
              break;

            case 2:
              hmin = atof(command) / 10;
              break;

            case 3:
              tmax = atof(command) / 10;
              break;

            case 4:
              tmin = atof(command) / 10;
              break;

            case 5:
              crc_ric = atoi(command);
              break;
          }
          command = strtok(0, ";");
          count++;
        }
        Serial.print("Answer: ");
        Serial.print(name);
        Serial.print(";");
        Serial.print(hmax, 1);
        Serial.print(";");
        Serial.print(hmin, 1);
        Serial.print(";");
        Serial.print(tmax, 1);
        Serial.print(";");
        Serial.println(tmin, 1);
        Serial.print("CRC recived: ");
        Serial.println(crc_ric);

        //calculate CRC-8, and I obtain the byte's array
        /********************************************************************************/
        byte bhmax = (byte)hmax;
        byte bhmin = (byte)hmin;
        byte btmax = (byte)tmax;
        byte btmin = (byte)tmin;

        byte crc32_str[4] = {
          bhmax, bhmin, btmax, btmin
        };

        crc = CRC8(crc32_str);
        Serial.println("CRC: ");
        Serial.println(crc);
        /********************************************************************************/

        if (crc_ric == (int)crc) {
          //Save in the EEPROM
          EEPROM_writeDouble(0, tmax);
          EEPROM_writeDouble(4, tmin);
          EEPROM_writeInt(8, hmax);
          EEPROM_writeInt(10, hmin);
        }
      }
    }
    delay(10);
  }
  /********************************************************************************/

  WiFi.disconnect(); 

  //Manage the HUMIDIFIER, HEATER and fan according to the sensor's value
  /**************************************************************************************/
  //HEATER
  if (myDHT22.getTemperatureC() >= tmax) {
    digitalWrite(HEATER, HIGH);
  }
  if (myDHT22.getTemperatureC() <= tmin + 1) {
    digitalWrite(HEATER, LOW);
  }

  //HUMIDIFIER
  if ((int)myDHT22.getHumidity() >= hmax) {
    digitalWrite(HUMIDIFIER, HIGH);
  }
  if ((int)myDHT22.getHumidity() <= hmin + 1) {
    digitalWrite(HUMIDIFIER, LOW);
  }

  //Fan, Brushless motor
  if (myDHT22.getTemperatureC() >= tmax + 4) {
    //Rotation of the motor according to with temperature, duty -> 0 per t = tmax+4 and duty -> 100 per t > tmax+10
    //Rotazione del motore al variare della temperatura, duty -> 0 per t = tmax+4 e duty -> 100 per t > tmax+10
    duty_motor = map(i , tmax + 4, tmax + 10, 0, 100);
    if (tmax > tmax + 10) {
      duty_motor = 100;
    }
    analogWrite(IN_2, duty_motor);
    delay(TCONST);
  }
  if (myDHT22.getTemperatureC() <= (tmax + tmin) / 2) {
    reset_ports();
    //Rotation of the motor according to with temperature, duty -> 0 per t = tmax+4 and duty -> 255 per t > tmax+10
    duty_motor = 0;
    analogWrite(IN_2, duty_motor);
    delay(TCONST);
  }
  /**************************************************************************************/

  delay(1000);
}

//Save double in EEPROM
void EEPROM_writeDouble(int ee, double value) {

  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
}

//Save int in EEPROM
void EEPROM_writeInt(int ee, int value) {

  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
}

//Read double to EEPROM
double EEPROM_readDouble(int ee) {

  double value = 0.0;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < 4; i++)
    *p++ = EEPROM.read(ee++);
  return value;
}

//Read int to EEPROM
int EEPROM_readInt(int ee) {

  int value = 0;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < 2; i++)
    *p++ = EEPROM.read(ee++);
  return value;
}

//Reset input
void reset_ports()
{
  digitalWrite(IN_1, 0);
  digitalWrite(IN_2, 0);
}

//Algorithm for CRC-8 based on Dallas/Maxim's farmulas
byte CRC8(const byte * data) {
  byte crc = 0x00;

  while (*data) {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

//It uses to reset millis()
void resetMillis() {
  cli();
  timer0_millis = 0;
  sei();
}
