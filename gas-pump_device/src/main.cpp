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
CommsHandler comms(THING_ID, THING_NAMESPACE);
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
String get_string_timestamp();

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
    JsonDocument response;
    switch (msgType) {
        case MESSAGE_TYPE::SUPPLY_AUTHORIZED:
            Serial.println(" - Supply Authorized message received");
            double amount;
            if(pump.supply_fuel(amount) == ESP_OK)
                response = Message::create_supply_completed_message(THING_ID, THING_NAMESPACE, amount, fuelStock, get_string_timestamp());
            else
                response = Message::create_supply_error_message(THING_ID, THING_NAMESPACE, "Problem occurred in executing fuel supply", get_string_timestamp());
            break;
        
        case MESSAGE_TYPE::UNKNOWN:
            Serial.println(" - Unknown message type received");
            Serial.println();
            return;
            //break;
    }

    // Serialize the response message
    String serializedResponse;
    ESP_ERROR_CHECK(Message::serialize_message(response, serializedResponse));

    // Publish the response message
    comms.publish_message(serializedResponse);

    Serial.println(" - Response published");
    Serial.println();
}

/*
    ##########################################################################
    ############                  Shared Code                     ############
    ##########################################################################
*/

String get_string_timestamp(){
    // Get the current time
    timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return comms.get_time_string(currentTime);
}

void announce_pump(){
    // Get the current time
    String timestamp = get_string_timestamp();

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
