#include <Arduino.h>
#include <CommsHandler.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
#ifdef MQTT_MAX_PACKET_SIZE 
#define MAX_PHOTO_SIZE MQTT_MAX_PACKET_SIZE
#else
#define MAX_PHOTO_SIZE 20000    // in bytes
#endif

/*
    ##########################################################################
    ############                 Global Variables                 ############
    ##########################################################################
*/
CommsHandler comms;

/*
    ##########################################################################
    ############                     Functions                    ############
    ##########################################################################
*/
void setup();
void loop();
void program_life();

/*
    ##########################################################################
    ############                   Gated Code                     ############
    ##########################################################################
*/
void CommsHandler::mqtt_message_callback(char* topic, byte* payload, unsigned int length){
    Serial.print("Message arrived in topic [");
    Serial.print(topic);
    Serial.println("]:");
    Serial.println(" - Size: " + String(length));
    Serial.print(" - Message: ");
    for(int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Allocate the JSON document
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        Serial.print(F("Error: deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // Extract values
    //Serial.println(F("Response:"));
    //Serial.println(doc["sensor"].as<const char*>());
}

/*
    ##########################################################################
    ############                  Shared Code                     ############
    ##########################################################################
*/

/*
void despatcher_api_fN(void* pvParameters){
    CommsHandler *comms = (CommsHandler*) pvParameters;
    while(true){
        comms->listen_to_api_clients();
        delay(10);
    }
}*/

void program_life(){
    
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("Gas Pump Terminal ready");
    Serial.println();
    Serial.flush();

    // Setup WiFi
    comms.connect_wifi();

    // Setup MQTT
    comms.connect_mqtt();

    // Start task of processing api requests
    //xTaskCreate(despatcher_api_fN, "API request processing function", 4096, (void*) &comms, 5, NULL);

}

void loop() {
    // Check if connected to WiFi
    if(!comms.connected_to_wifi())
        comms.connect_wifi();

    // Check if connected to MQTT
    if(!comms.connected_to_mqtt())
        comms.connect_mqtt();

    program_life();
}
