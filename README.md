
## Example Summary

This example introduce the MQTT Client library API and usage.

## Peripherals Exercised

* The board LEDs are used for status indication. To distinguish similar indications, the user needs to be aware of the executed procedure.   
The following table lists all options.

<table>
  <tr>
    <th>LED indication</th>
    <th>Led Color for CC3220 </th>
    <th>Led Color for CC3235 </th>
    <th>Interpretation</th>
  </tr>
  <tr>
    <td>Solidly on</td>
    <td>Green</td>
    <td>Green</td>
    <td>Indicate Simplelink is properly up - Every Reset / Initialize</td>
  </tr>
  <tr>
    <td>Blinking</td>
    <td>Red</td>
    <td>Blue</td>
    <td>Device is trying to connect to AP - Every Reset / Initialize</td>
  </tr>
  <tr>
    <td>Solidly off</td>
    <td>All</td>
    <td>All</td>
    <td>Device connected and working - Only after connection</td>
  </tr>
  <tr>
    <td>Toggling (Solidly on/off)</td>
    <td>Red</td>
    <td>Blue</td>
    <td>Publish message received in cc32xx/ToggleLED1 topic</td>
  </tr>
  <tr>
    <td>Toggling (Solidly on/off)</td>
    <td>Yellow</td>
    <td>Red</td>
    <td>Publish message received in cc32xx/ToggleLED2 topic</td>
  </tr>
  <tr>
    <td>Toggling (Solidly on/off)</td>
    <td>Green</td>
    <td>Green</td>
    <td>Publish message received in cc32xx/ToggleLED3 topic</td>
  </tr>
</table>

## Example Usage

* Access Point (AP) Configuration
	- AP information is set in 'network\_if.h' file.

* Remote Broker Configuration

	- Broker parameters can be configured in mqttConnParams parameter which can be found in 'mqtt\_client\_app.c'
	- The broker parameters are:
		- Connection types and security options
			- IPv4 connection
			- IPv6 connection
			- URL connection
			- Secure connection
			- skip domain name verification in secure connection
			- skip certificate catalog verification in secure connection
		- Server Address: URL or IP
    	- Port number of MQTT server
    	- Method to tcp secured socket
    	- Cipher to tcp secured socket
    	- Number of files for secure transfer
    	- The secure Files  

* Secured socket  
	In order to activate the secured example, SECURE\_MQTT must be defined in 'mqtt\_client\_app.c' file  ( certificates should be programmed ).

* Client Authentication  
	In order to activate the Client authentication by the server, the clientID and password must be defined in mqttClientParams located in 'mqtt_client_app.c'.
  
* Topics Configuration
	- The topics can be set by calling MQTT_IF_Subscribe with the appropriate parameters. More details in 'mqtt_if.h'.
	- The subscription topics can be set in the **SUBSCRIPTION\_TOPICX** definitions
	- The Client is subscribe to the following default topics  
		**"Broker/To/cc32xx"**  
		**"cc32xx/ToggleLEDCmdL1"**  
		**"cc32xx/ToggleLEDCmdL2"**  
		**"cc32xx/ToggleLEDCmdL3"**  
	- To publish to topics the user can call MQTT_IF_Publish with the appropriate parameters. More details in 'mqtt_if.h'.
	- The Client publish the following default topic “cc32xx/ToggleLED1” - the topic will be published by pressing SW2 on the board
	
* Build the project and flash it by using the Uniflash tool for cc32xx, or equivalently, run debug session on the IDE of your choice.

* Open a serial port session (e.g. 'HyperTerminal','puTTY', 'Tera Term' etc.) to the appropriate COM port - listed as 'User UART'.  
The COM port can be determined via Device Manager in Windows or via `ls /dev/tty*` in Linux.

	The connection should have the following connection settings:

    	Baud-rate:    115200
	    Data bits:         8
	    Stop bits:         1
	    Parity:         None
	    Flow Control:   None


* Run the example by pressing the reset button or by running debug session through your IDE.  
 `Green LED` turns ON to indicate the Application initialization is complete 

* Once the application has completed it's initialization and the network processor is up,  
  the application banner would be displayed, showing version details:

        ============================================
           [GEN::TRACE] MQTT client Example Ver: 2.0.0
        ============================================

         [GEN::TRACE] CHIP: 0x30000019
         [GEN::TRACE] MAC:  2.0.0.0
         [GEN::TRACE] PHY:  2.2.0.7
         [GEN::TRACE] NWP:  3.15.0.1
         [GEN::TRACE] ROM:  0
         [GEN::TRACE] HOST: 3.0.1.65
         [GEN::TRACE] MAC address: 04:a3:16:45:89:8e

        ============================================

* At this point `Board_LED0` will blink until the device will connect to the hard coded AP.  
	* In case connection to the hard coded SSID AP fails, user will be requested to fill the SSID of an open AP it wishes to connect to.
  	* If no AP is available or connection failed, the example will prompt the user to enter the SSID of an open AP.
	* Once the connection success all LEDs turn off.

* Special handling
	- In case the client will disconnect (for any reason) from the remote broker, the MQTT will be restarted.   
	The user can change this behavior by removing the mq_send call for APP_MQTT_DEINIT in the MQTT_EVENT_SERVER_DISCONNECT event for the MQTT_EventCallback.

## Application Design Details

This is an MQTT Client application used to demonstrate the client side of the MQTT protocol. This application uses our MQTT module (mqtt_if.c) as an abstraction layer to our internal MQTT library to make it easier for developers to use MQTT. 

The application starts by performing all the necessary initializations for the peripherals (e.g. GPIO, SPI, UART and timer).  Once that is done the NWP is initialized and the application attempts to connect to an AP using the credentials the user configured in network_if.c. 

At this point everything is initialized except the MQTT client and the device is connected to an AP. In case the connection to the AP fails, the user will be requested to fill the SSID of an open AP to connect to. Next the application calls 'MQTT\_IF\_Init()' to initialize the MQTT module. By default the module will create an internal thread of stack size 2048 and priority 2. These parameters can be modified by changing mqttInitParams in 'mqtt\_client\_app.c'. This thread handles all the events invoked by the internal MQTT library and notifies the user of such events through the MQTT_EventCallback and specific topic callbacks they register when subscribing. 

Then 'MQTT\_IF\_Connect' is called to connect to the broker the user configured in the mqttConnParams structure in 'mqtt\_client\_app.c'. Additionally, it will configure the client parameters using the mqttClientParams structure and register the MQTT\_EventCallback to the MQTT module. If the  'MQTT\_IF\_Connect' call is successful the user will get a CONNACK event indication over UART to show the client is connected to the MQTT broker. 

The application proceeds to subscribe to 4 default topics for illustration purposes and registers 4 individual topic callbacks that are defined in 'mqtt\_client\_app.c'. 

Now the client can receive publish messages from the broker.
In this example the topics in the left will toggle the LEDs in the right
     
							"cc32xx/ToggleLEDCmdL1" <-------------> toggle LED0  
							"cc32xx/ToggleLEDCmdL2" <-------------> toggle LED1  
							"cc32xx/ToggleLEDCmdL3" <-------------> toggle LED2   


The user can invoke more commands by pressing the push buttons on the CC32xx launchpad device: 

* When pressing push button 0 - SW2, The device will publish the message that include the topic and data which is hard coded in 'mqtt\_client\_app.c' by invoking 'MQTT\_IF\_Publish' command.

* Push button 1 - SW3 has multiple functionalities. The first time it's pressed the device will disconnect from the broker and the user will see a MQTT_EVENT_CLIENT_DISCONNECT event from the MQTT_EventCallback. Every time there is a short press on the button it will toggle the connection to the MQTT broker. If the user does a long press the application will de-initialize the MQTT module by destroying the internal MQTT instance as well as freeing any module resources that were allocated. 
	
## References

[MQTT Org - MQTT Home page](http://mqtt.org/documentation)  
[MQTT v3.1.1 specification](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html)  
[MQTT v3.1 specification](http://www.ibm.com/developerworks/webservices/library/ws-mqtt/index.html)  

For further information please refer to the user programmers guide: [CC3X20 Programmer's Guide](http://www.ti.com/lit/swru455)