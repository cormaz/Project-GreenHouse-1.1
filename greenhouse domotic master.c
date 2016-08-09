/*Automation greenhouse project*/

/*Autors: Antonio La Mura, Umberto Festa*/

/*Date: 03/03/2016*/

/*Our idea is to allow users, who buy fruits and vegetables grown in greenhouses, to know exactly all the plantation steps that products have had,
  such as the use of chemical fertilizers and other similar products.
  A QR code will be put on the sold products that will be read by a specific smartphone app.
  It provides information about environment conditions where products have been grown and the chemical products used.
  The automatic monitoring system in greenhouses is composed of sensors that read environment data and actuators, called Slaves.
  They communicate via Wireless with central device, called Master. The latter sends possible changes to slaves
  (like change of thresholds) and data via WiFi to webserver as well.
  When the product is ready to be sold, QR code generation is requested to the webservice and it will be put on the package.
  The last part of the system is the mobile app that is responsible for the QR codes scanning and shows the relative informations to the user.*/

/*Master*/

/*DEVO AGGIUNGERE SOLO LA PARTE CHE MI SERVE PER RICEVERE LE NUOVE SOGLIE CHE VENGONO INVIATE DALL'APP*/

/*Libraries used*/
#include <SPI.h>
#include <WiFi101.h>
#include <RTCZero.h>
#include <WiFiUDP.h>
#include <RTCZero.h>

/*Variables for connection*/
char ssid[] = "SSID";               /*Your network SSID (name)*/
char pass[] = "pass";     /*Uour network password (use for WPA, or use as key for WEP)*/
int keyIndex = 0;                             /* Your network key Index number (needed only for WEP)*/

char ssid_ap[] = "Arduino";                   /*Created AP name*/
char pass_ap[] = "";                          /*(Not supported yet)*/

int status = WL_IDLE_STATUS;

unsigned int localPort = 2390;                /*Local port to listen on*/

char server[] = "www.cormaz.altervista.org";  /*Name address for Google (using DNS)*/

WiFiServer server_ap(80);
WiFiClient client_ap;
WiFiUDP Udp;
RTCZero rtc;
WiFiClient client;

char packetBuffer[255];                       /*Buffer to hold incoming packet*/
char ReplyBuffer[255];                        /*A string to send back*/

/*Variables for new thresholds*/
float humax = 0;
float humin = 0;
float tumax = 0;
float tumin = 0;
/*Works like Access Point (flag = false), connects to WebServer (flag = true)*/
boolean flag = false;
boolean threeshold_available = false;

void setup() {
  /*Initialize serial and wait for port to open:*/
  Serial.begin(9600);
  while (!Serial) {
    ; /*Wait for serial port to connect. Needed for native USB port only*/
  }

  Serial.println();
}

void loop() {
  int packetSize;
  double temp;
  double hum;
  int id;
  byte crc;
  String strURL;

  //Check for the presence of the shield:
  /**************************************************************************************/
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    /*Don't continue:*/
    while (true);
  }
  /**************************************************************************************/

  //Attempt to connect to WiFi network:
  /**************************************************************************************/
  while (status != WL_CONNECTED) {
    Serial.print("Creating Network named: ");
    Serial.println(ssid_ap);
    /*Connect to WPA/WPA2 network. Change this line if using open or WEP network:*/
    status = WiFi.beginAP(ssid_ap);

    /*Wait 10 seconds for connection:*/
    delay(10000);
    server_ap.begin();
  }
  Serial.println("Connected to wifi");
  /**************************************************************************************/

  //Start UDP communication
  /**************************************************************************************/
  Udp.begin(localPort);
  printWifiStatus();


  client_ap = server_ap.available();

  if (client_ap) {    /*If you get a client*/
    /*I'm waitinf for some information*/
    Serial.println("new client");           /*Print a message out the serial port*/
    /*If there's data available, read a packet*/
    packetSize = Udp.parsePacket();
    if (packetSize) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
      IPAddress remoteIp = Udp.remoteIP();
      Serial.print(remoteIp);
      Serial.print(", port ");
      Serial.println(Udp.remotePort());

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
            id = atoi(command);
            break;

          case 1:
            temp = atof(command) / 10;
            break;

          case 2:
            hum = atof(command) / 10;
            break;
        }

        command = strtok(0, ";");
        count++;
      }
      Serial.print("Package received from ");
      Serial.print(id);
      Serial.print("  T: ");
      Serial.print(temp, 1);
      Serial.print(" H: ");
      Serial.println(hum, 1);
      /**************************************************************************************/

      delay(20);

      /*Calculate the CRC-8, so create byte array*/
      /********************************************************************************/
      byte bhmax = (byte)humax;
      byte bhmin = (byte)humin;
      byte btmax = (byte)tumax;
      byte btmin = (byte)tumin;

      byte crc32_str[4] = {
        bhmax, bhmin, btmax, btmin
      };

      crc = CRC8(crc32_str);
      Serial.println("CRC: ");
      Serial.println(crc);
      /********************************************************************************/

      if (threeshold_available == true) {
        snprintf(ReplyBuffer, sizeof(ReplyBuffer), "%d;%d;%d;%d;%d;%d", id, (int)humax, (int)humin, (int)tumax, (int)tumin, (int)crc);

        /*Send a reply, to the IP address and port that sent us the packet we received*/
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(ReplyBuffer);
        Udp.endPacket();
      }
    }                 /*Print it out the serial monitor*/
    /*Close the connection:*/
    client_ap.stop();
    Serial.println("client disconnected");
    flag = true;
  }/*Fine AP*/
  /********************************************************************************/

  /*Connect to the server and send data to DataBase*/
  /********************************************************************************/
  if (flag == true) {
    /*Attempt to connect to Wifi network:*/
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      /*Connect to WPA/WPA2 network. Change this line if using open or WEP network:*/
      status = WiFi.begin(ssid, pass);

      /*Wait 10 seconds for connection:*/
      delay(10000);
    }
    Serial.println("Connected to wifi");
    printWifiStatus();

    strURL = "GET /YourAddress.php?id=";
    strURL += id;
    strURL += "&parameter1=";
    strURL += temp;
    strURL += "&parameter2=";
    strURL += hum;
    strURL += " HTTP/1.1";

    Serial.println("\nStarting connection to server...");
    // if you get a connection, report back via serial:
    if (client.connect(server, 80)) {
      Serial.println("connected to server");
      // Make a HTTP request:
      client.println(strURL);
      client.println("Host: www.cormaz.altervista.org");
      client.println("Connection: close");
      client.println();
      client.stop();

      Serial.println("Ok!");
    }

    /*If the server's disconnected, stop the client:*/
    if (!client.connected()) {
      Serial.println();
      Serial.println("Disconnecting from server.");
      client.stop();

      /*Do nothing forevermore:*/
      while (true);
    }
    flag = false;
  }
  /********************************************************************************/
}

/*Calculate algorithm CRC-8 - based on Dallas/Maxim formules*/
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

void printWifiStatus() {
  /*Print the SSID of the network you're attached to:*/
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  /*Print your WiFi shield's IP address:*/
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  /*Print the received signal strength:*/
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
