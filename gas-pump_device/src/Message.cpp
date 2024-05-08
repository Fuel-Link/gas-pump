#include <Message.h>

JsonDocument Message::create_supply_authorized_message(String thingId, String thingNamespace, boolean authorization, String timestamp){
    JsonDocument doc;

    doc["thingId"] = thingNamespace + ":" + thingId;;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/authorize_supply/properties/";
    doc["authorization"] = authorization;

    return doc;
}

JsonDocument Message::create_supply_completed_message(String thingId, String thingNamespace, FUEL_TYPE fuelType, double amount, double stock, int capacity, String timestamp){
    JsonDocument doc;

    doc["thingId"] = thingNamespace + ":" + thingId;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/supply_completed/properties/";
    doc["fuelType"] = fuelType;
    doc["amount"] = amount;
    doc["stock"] = stock;
    doc["capacity"] = capacity;

    return doc;
}

JsonDocument Message::create_fuel_replenishment_message(String thingId, String thingNamespace, FUEL_TYPE fuelType, double amount, double stock, int capacity, String timestamp){
    JsonDocument doc;

    doc["thingId"] = thingNamespace + ":" + thingId;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/fuel_replenishment/properties/";
    doc["fuelType"] = fuelType;
    doc["amount"] = amount;
    doc["stock"] = stock;
    doc["capacity"] = capacity;

    return doc;
}

JsonDocument Message::create_supply_error_message(String thingId, String thingNamespace, String error, String timestamp){
    JsonDocument doc;

    doc["thingId"] = thingNamespace + ":" + thingId;
    doc["timestamp"] = timestamp;
    doc["topic"] = thingNamespace + "/" + thingId + "/things/twin/commands/create";
    doc["path"] = "/features/supply_error/properties/";
    doc["error"] = error;

    return doc;
}

esp_err_t Message::serialize_message(JsonDocument& doc, String& serializedDoc) {
    if(serializeJson(doc, serializedDoc) == 0){
        Serial.println("Error: Failed to serialize JSON object");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t Message::deserialize_message(String& message, JsonDocument& doc) {
    DeserializationError error = deserializeJson(doc, message, message.length());

    if (error) {
        Serial.print(F("Error: deserializeJson() failed: "));
        Serial.println(error.f_str());
        return ESP_FAIL;
    }
    return ESP_OK;
}