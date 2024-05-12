#include <Arduino.h>
#include <CommsHandler.h>
#include <Message.h>
#include <DataTypes.h>
#include <PumpInteraction.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
#ifndef FUEL 
#define FUEL FUEL_TYPE::DIESEL
#endif
#ifndef CAPACITY 
#define CAPACITY 5000
#endif
#ifndef STOCK 
#define STOCK 5000
#endif
#ifndef THING_NAMESPACE 
#define THING_NAMESPACE "REPLACE_WITH_YOUR_NAMESPACE"
#endif
#ifndef THING_ID 
#define THING_ID "REPLACE_WITH_YOUR_THING_ID"
#endif

/*
    ##########################################################################
    ############                 Global Variables                 ############
    ##########################################################################
*/
CommsHandler comms;
PumpInteraction pump(FUEL, CAPACITY, STOCK);
double fuelStock = STOCK;

/*
    ##########################################################################
    ############                     Functions                    ############
    ##########################################################################
*/
void setup();
void loop();
void announce_pump();

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
    ESP_ERROR_CHECK(Message::deserialize_message((char*) payload, length, doc));

    // Extract the message type
    MESSAGE_TYPE msgType = Message::get_message_type(doc);

    // Process the message based on the message type
    switch (msgType) {
        case MESSAGE_TYPE::SUPPLY_AUTHORIZED:
            Serial.println(" - Supply Authorized message received");
            break;
        
        case MESSAGE_TYPE::FUEL_REPLENISHMENT:
            Serial.println(" - Fuel Replenishment message received");
            break;
        case MESSAGE_TYPE::SUPPLY_ERROR:
            Serial.println(" - Supply Error message received");
            break;
        case MESSAGE_TYPE::UNKNOWN:
            Serial.println(" - Unknown message type received");
            break;
    }

}

/*
    ##########################################################################
    ############                  Shared Code                     ############
    ##########################################################################
*/

void announce_pump(){
    // Get the current time
    timeval currentTime;
    gettimeofday(&currentTime, NULL);
    long timeInMs = comms.get_time_in_ms(currentTime);
    String timestamp = comms.get_time_string(currentTime);

    // Create the pump init message
    JsonDocument pumpInitMessage = Message::create_pump_init_message(THING_ID, THING_NAMESPACE, FUEL, fuelStock, CAPACITY, timestamp);

    // Serialize the pump init message
    String serializedPumpInitMessage;
    ESP_ERROR_CHECK(Message::serialize_message(pumpInitMessage, serializedPumpInitMessage));

    // Publish the pump init message
    comms.publish_message(serializedPumpInitMessage);

    Serial.println("Pump initialized");
    Serial.println();
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

    // Setup Pump Interaction
    ESP_ERROR_CHECK(pump.init_pump());

    // Announce the gas pump operation to Ditto
    announce_pump();

}

void loop() {
    // Check if connected to WiFi
    if(!comms.connected_to_wifi())
        comms.connect_wifi();

    // Check if connected to MQTT
    if(!comms.connected_to_mqtt())
        comms.connect_mqtt();

    delay(10);
}
