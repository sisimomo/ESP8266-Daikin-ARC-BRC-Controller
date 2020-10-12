// http://pdacontrolen.com/wifimanager-emoncms-oem-with-esp8266-temperature-1/ Help me to create Wifi hotspot

//needed for library
#include <FS.h>						//this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h>			//https://github.com/bblanchon/ArduinoJson
#include <ESP8266WiFi.h>			//https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <ESP8266WebServer.h>		//https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
#include <DNSServer.h>				//https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
#include <WiFiManager.h>			//https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>			//https://github.com/knolleary/pubsubclient
#include <DYIRDaikin.h>				//https://github.com/danny-source/Arduino_DY_IRDaikin

/*** Default Value ***/
char charServerMQTT[40] = "homeassistant.local";
char charPortMQTT[6] = "1883";
char charUsernameMQTT[35] = "Username";
char charPasswordMQTT[33] = "Password";
char charTopicPrefixMQTT[35] = "DaikinACController";

bool useAButton = false;

#define button_config 5 // Button
#define pin_led_Wifi 16 // green led

// SSID WifiManager configuration network //
const char * ssid_apmode = "DaikinACCtrl";

// Init for PubSubClient (MQTT)
WiFiClient espClient;
PubSubClient client(espClient);

// Librarie use to send right IR codes to AC
DYIRDaikin daikinController;

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback() {
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("\n Starting");

    daikinController.begin(2);

	if (useAButton) {
		pinMode(button_config, INPUT_PULLUP);
	}
	
    pinMode(pin_led_Wifi, OUTPUT);

    ///  clean FS, for testing
    //SPIFFS.format();

    ////////////////////////////////////////////
    ////////Montar SP///////////////////////////
    ////////////////////////////////////////////

    //read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin()) {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json")) {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile) {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr < char[] > buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonBuffer jsonBuffer;
                JsonObject & json = jsonBuffer.parseObject(buf.get());
                json.printTo(Serial);
                if (json.success()) {
                    Serial.println("\nparsed json");

                    strcpy(charServerMQTT, json["charServerMQTT"]);
                    strcpy(charPortMQTT, json["charPortMQTT"]);
                    strcpy(charUsernameMQTT, json["charUsernameMQTT"]);
                    strcpy(charPasswordMQTT, json["charPasswordMQTT"]);
                    strcpy(charTopicPrefixMQTT, json["charTopicPrefixMQTT"]);
                } else {
                    Serial.println("failed to load json config");
                }
            }
        }
    } else {
        Serial.println("failed to mount FS");
    }
    //end read

    Serial.println("SSID Now");
    Serial.println(WiFi.SSID());

    /********** Connection Verification **********/
    if (WiFi.SSID() == "") {
        Serial.println("Not configured !!! or do not connect to the NETWORK !!!");
        initialConfig = true;
    } else {
        Serial.println("Correct configuration");
    }
    client.setCallback(callback);
}

void createWifi() {
    WiFiManagerParameter custom_server_MQTT("charServerMQTT", "server MQTT", charServerMQTT, 40, "title= \"Parameter 1: Server or Host for MQTT example: 192.168.0.12 or domain.com\"");
    WiFiManagerParameter custom_port_MQTT("charPortMQTT", "port MQTT", charPortMQTT, 6, "title= \"Parameter 2: Port 1883 default. \"   type=\"number\"  ,  min=\"1\" max=\"65535\"   ");
    WiFiManagerParameter custom_username_MQTT("charUsernameMQTT", "username MQTT", charUsernameMQTT, 36, "title= \"Parameter 3: Username for MQTT. \"   maxlength=\"35\"  \"");
    WiFiManagerParameter custom_password_MQTT("charPasswordMQTT", "password MQTT", charPasswordMQTT, 34, "title= \"Parameter 4: Password for MQTT. \"   maxlength=\"33\"  \"");
    WiFiManagerParameter custom_topic_prefix_MQTT("charTopicPrefixMQTT", "topic", charTopicPrefixMQTT, 36, "title= \"Parameter 5: Topic prefix for MQTT. \"   maxlength=\"35\"  \"");

    WiFiManager wifiManager;

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    WiFiManagerParameter custom_text("<p><strong>Mqtt Configuration</strong></p>");
    wifiManager.addParameter( & custom_text);
    wifiManager.addParameter( & custom_server_MQTT);
    wifiManager.addParameter( & custom_port_MQTT);
    wifiManager.addParameter( & custom_topic_prefix_MQTT);
    wifiManager.addParameter( & custom_username_MQTT);
    wifiManager.addParameter( & custom_password_MQTT);

    String ssid = String(ssid_apmode) + String("_") + String(ESP.getChipId());

    if (!wifiManager.startConfigPortal(ssid.c_str())) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(5000);
    }
    strcpy(charServerMQTT, custom_server_MQTT.getValue());
    strcpy(charPortMQTT, custom_port_MQTT.getValue());
    strcpy(charUsernameMQTT, custom_username_MQTT.getValue());
    strcpy(charPasswordMQTT, custom_password_MQTT.getValue());
    strcpy(charTopicPrefixMQTT, custom_topic_prefix_MQTT.getValue());

    if (shouldSaveConfig) {
        Serial.println("saving config");
        DynamicJsonBuffer jsonBuffer;
        JsonObject & json = jsonBuffer.createObject();
        Serial.print("charServerMQTT: ");
        Serial.println(charServerMQTT);
        Serial.print("charPortMQTT: ");
        Serial.println(charPortMQTT);
        Serial.print("charUsernameMQTT: ");
        Serial.println(charUsernameMQTT);
        Serial.print("charPasswordMQTT: ");
        Serial.println(charPasswordMQTT);
        Serial.print("charServerMQTT: ");
        Serial.println(charTopicPrefixMQTT);

        json["charServerMQTT"] = charServerMQTT;
        json["charPortMQTT"] = charPortMQTT;
        json["charUsernameMQTT"] = charUsernameMQTT;
        json["charPasswordMQTT"] = charPasswordMQTT;
        json["charTopicPrefixMQTT"] = charTopicPrefixMQTT;

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile) {
            Serial.println("failed to open config file for writing");
        }

        json.printTo(Serial);
        json.printTo(configFile);
        configFile.close();
    }
}

/* MQTT STOCK START */

void publishMQTTMessage(char * topic, char * payload, boolean retain) {
    client.publish(topic, (uint8_t * ) payload, strlen(payload), retain);
}

void publishMQTTMessage(char * topic, boolean payload, boolean retain) {
    publishMQTTMessage(topic, (payload ? (char *)"True" : (char *)"False"), retain);
}

void publishMQTTMessage(char * topic, uint8_t payload, boolean retain) {
    String str = String(payload);
    char buf[sizeof(str)];
    str.toCharArray(buf, sizeof(buf));
    publishMQTTMessage(topic, buf, retain);
}

void publishMQTTMessage(char * topic, unsigned int payload, boolean retain) {
    String str = String(payload);
    char buf[sizeof(str)];
    str.toCharArray(buf, sizeof(buf));
    publishMQTTMessage(topic, buf, retain);
}

void publishMQTTMessage(char * topic, String payload, boolean retain) {
    char charArray[sizeof(payload)];
    payload.toCharArray(charArray, sizeof(charArray));
    publishMQTTMessage(topic, charArray, true);
}

void reconnect() {
    // Loop until we're reconnected
    if (!client.connected() && WiFi.status() == WL_CONNECTED) {
        Serial.println("Attempting MQTT connection...");

        char charDeviceLabel[50];
        String deviceLabel = String(ssid_apmode) + String("_") + String(ESP.getChipId());
        deviceLabel.toCharArray(charDeviceLabel, 50);
        /*Serial.print("MQTT Client id: ");
        Serial.println(charDeviceLabel);
        Serial.print("MQTT username: ");
        Serial.println(charUsernameMQTT);
        Serial.print("MQTT password: ");
        Serial.println(charPasswordMQTT);*/
        client.setServer(charServerMQTT, String(charPortMQTT).toInt());
        // Attemp to connect
        if (client.connect(charDeviceLabel, charUsernameMQTT, charPasswordMQTT)) {
            Serial.println("Connected to MQTT server");
			subscribeToTopic("%s/Power");
			subscribeToTopic("%s/Mode");
			subscribeToTopic("%s/Temperature");
			subscribeToTopic("%s/FanSpeed");
			subscribeToTopic("%s/Swing");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
        }
    }
}

void subscribeToTopic(char * topic) {
	char completeTopic[50];
	sprintf(completeTopic, topic, charTopicPrefixMQTT);
	client.subscribe(completeTopic);
	Serial.print("SUBSCRIBE TO:");
	Serial.println(completeTopic);
}

void callback(char * topic, byte * payload, unsigned int length) {
    Serial.print("Message arrived [");
    String actualTopic = topic;
    actualTopic = actualTopic.substring(strlen(charTopicPrefixMQTT) + 1);
    Serial.print(actualTopic);
    Serial.print("] ");
    payload[length] = '\0';
    String actualPayload = String((char * ) payload);
    Serial.println(actualPayload);

    if (actualTopic.equals("Power")) {
        Serial.print("Power : ");
        Serial.println(actualPayload);

        daikinController.off();
    } else if (actualTopic.equals("Mode")) {
        Serial.print("Mode : ");
    		if (actualPayload.equals("off")) {
            Serial.println("off");
            daikinController.off();
        } else {
      			daikinController.on();
      			String modes[] = { "fan_only", "cool", "dry", "heat" };
      			for (int i = 0; i < sizeof(modes); i++) {
        				if (actualPayload.equals(modes[i])) {
          					Serial.println(modes[i]);
          					daikinController.setMode(i);
          					break;
        				}
      			}
    	  }
    } else if (actualTopic.equals("Temperature")) {
        Serial.print("Temperature : ");
        Serial.println(actualPayload.toInt());
        daikinController.setMode(actualPayload.toInt());
    } else if (actualTopic.equals("FanSpeed")) {
        Serial.print("FanSpeed : ");
    		String fanSpeeds[] = { "Min", "Low", "Medium", "High", "Auto" };
    		for (int i = 0; i < sizeof(fanSpeeds); i++) {
    		  	if (actualPayload.equals(fanSpeeds[i])) {
    			  	  Serial.println(fanSpeeds[i]);
    				    daikinController.setFan(i+1);
    				    break;
			      }
    		}
    } else if (actualTopic.equals("Swing")) {
        Serial.print("Swing : ");
        if (actualPayload.equals("On") || actualPayload.equals("on")) {
			Serial.print("On");
            daikinController.setSwing_on();
        } else {
			Serial.print("Off");
            daikinController.setSwing_off();
        }
    }
    daikinController.sendCommand();
}

/* MQTT STOCK END */

void loop() {

    if (initialConfig) {
        Serial.println("Network connection failed");
        Serial.println("configure network credentials (Config button)");
        delay(2000);
    }

    // MQTT connection
    if (!client.connected()) {
        reconnect();
        delay(2000);
    } else {
        client.loop(); //For MQTT, see if a new message has arrived
    }
	
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(pin_led_Wifi, HIGH);
    } else {
        digitalWrite(pin_led_Wifi, LOW);
    }
	if (useAButton) {
		if (digitalRead(button_config) == HIGH) {
			digitalWrite(pin_led_Wifi, LOW);
			Serial.println("Creating a Wifi to configure the ESP");
			createWifi();
		}
	}
}
