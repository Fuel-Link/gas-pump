#include <Message.h>

JsonDocument Message::create_supply_authorized_message(String thingId, String thingNamespace, boolean authorization, String timestamp){
    JsonDocument doc;

    doc["msgType"] = MESSAGE_TYPE::SUPPLY_AUTHORIZED;
    doc["thingId"] = thingNamespace + ":" + thingId;;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/authorize_supply/properties/";
    doc["authorization"] = authorization;

    return doc;
}

JsonDocument Message::create_supply_completed_message(String thingId, String thingNamespace, double amount, double stock, String timestamp){
    JsonDocument doc;

    doc["msgType"] = MESSAGE_TYPE::SUPPLY_COMPLETED;
    doc["thingId"] = thingNamespace + ":" + thingId;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/supply_completed/properties/";
    doc["amount"] = amount;
    doc["stock"] = stock;

    return doc;
}

JsonDocument Message::create_fuel_replenishment_message(String thingId, String thingNamespace, double amount, double stock, String timestamp){
    JsonDocument doc;

    doc["msgType"] = MESSAGE_TYPE::FUEL_REPLENISHMENT;
    doc["thingId"] = thingNamespace + ":" + thingId;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/fuel_replenishment/properties/";
    doc["amount"] = amount;
    doc["stock"] = stock;

    return doc;
}

JsonDocument Message::create_supply_error_message(String thingId, String thingNamespace, String error, String timestamp){
    JsonDocument doc;

    doc["msgType"] = MESSAGE_TYPE::SUPPLY_ERROR;
    doc["thingId"] = thingNamespace + ":" + thingId;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/supply_error/properties/";
    doc["error"] = error;

    return doc;
}

JsonDocument Message::create_pump_init_message(String thingId, String thingNamespace, FUEL_TYPE fuelType, double stock, int capacity, String timestamp){
    JsonDocument doc;

    doc["msgType"] = MESSAGE_TYPE::PUMP_INIT;
    doc["thingId"] = thingNamespace + ":" + thingId;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/pump_init/properties/";
    doc["fuelType"] = fuelType;
    doc["stock"] = stock;
    doc["capacity"] = capacity;

    return doc;
}

esp_err_t Message::serialize_message(JsonDocument& doc, String& serializedDoc) {
    if(serializeJson(doc, serializedDoc) == 0){
        Serial.println("Error: Failed to serialize JSON object");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t Message::deserialize_message(char* message, size_t length,  JsonDocument& doc) {
    DeserializationError error = deserializeJson(doc, message, length);

    if (error) {
        Serial.print(F("Error: deserializeJson() failed: "));
        Serial.println(error.f_str());
        return ESP_FAIL;
    }
    return ESP_OK;
}

MESSAGE_TYPE Message::get_message_type(JsonDocument& doc){
    return (MESSAGE_TYPE) doc["msgType"].as<int>();
}