# Project-GreenHouse-1.1
Remote control greenhouse temperature and humidity, with master and slaves
Overview

The greenhouse environmental conditions change continuously, for this reason we need an efficient monitoring.

There are a lot of benefits of using an automatic system, for example less work or most important the users can check their own investments from their house by PC or smartphone.

Another important advantage is the possibility to store data into a DataBase. This can make the difference between gaining or losing money.

Furthermore, thanks to a real-time system control we can intervene immediately, preventing problems for cultivation.

The automatic monitoring system in greenhouses is composed of sensors that read environment data and actuators, called “Slaves”. They communicate via Wireless with central device, called “Master”. The latter sends possible changes to slaves (like change of thresholds) and data via WiFi to webserver as well.
1. How does it Work?

We can split this project in three different parts:

    Master
    Slave
    Web Server

We’ve used an Arduino/Genuino MKR1000 for the Master, an Arduino/Genuino Uno for the Slave.

The Master communicates with a Web Server via WiFi (WINC1500 integrated), the Slave acquires temperature and humidity with DHT22 sensor, and then it sends these data to the Master via WiFi (WiFi shield WINC1500).

2. Master

The Master is on “Access Point mode” and it waits for the Slave connection to receive the temperature and humidity that are elaborated and sent to the Web Server.

The Master checks if there are data available, in this case it creates an UDP packet formed by new thresholds and CRC. Infacts, it calculates the CRC that will be used by the Slave to validate the setting received.

After the Master gets out of “Access Point mode”, it connects to the WiFi, in order to send data to WebServer where they will be put in a graph.

[logo]: http://www.google.com/images/logo.gif
3. Slave

The Slave acquires temperature and humidity by DHT22 sensor and after 5 minutes it sends data to the Master. It also creates an UDP packet, it connects to the Master and then sends the data.

Later it waits if there are some data available, such as new thresholds. In this case, the Slave receives new settings and calculates the CRC using the Dallas-Maxim formulas.

Subsequently the CRC calculated is compared with the CRC received from the Master. If the two CRC are the same, the Slave saves the new setting in the EEPROM.
4. Web Server

The Web Server saves all the data that later can be stored in a history.

In order to do that, we have used a PHP script that connects to database and shows data in two different ways

    in a graph, using another PHP script
    in an Android app, in JSON format using another PHP script

5. APP

The App allows us to consult the data into a Data Base.

In the first screen, we can select the timing range to show and with the button "GRAPHIC", it contacts the web service and gets the data.

The App shows data into graphic.


