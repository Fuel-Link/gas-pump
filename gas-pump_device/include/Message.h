#pragma once

#include <ArduinoJson.h>
#include <DataTypes.h>

namespace Message {

    /**
     * \brief Create a supply authorized message
     * \param thingId The ID of the thing
     * \param thingNamespace The namespace of the thing
     * \param authorization The authorization status
     * \param timestamp The timestamp of the message
     * \return The JSON document
     * \note Message format not supposed to be created by the device, just here for reference
    */
    JsonDocument create_supply_authorized_message(String thingId, String thingNamespace, boolean authorization, String timestamp);

    /**
     * \brief Create a supply completed message
     * \param thingId The ID of the thing
     * \param thingNamespace The namespace of the thing
     * \param fuelType The type of fuel
     * \param amount The amount of fuel supplied
     * \param stock The current stock of the pump
     * \param capacity The capacity of the pump
     * \param timestamp The timestamp of the message
     * \return The JSON document
    */
    JsonDocument create_supply_completed_message(String thingId, String thingNamespace, FUEL_TYPE fuelType, double amount, double stock, int capacity, String timestamp);

    /**
     * \brief Create a fuel replenishment message
     * \param thingId The ID of the thing
     * \param thingNamespace The namespace of the thing
     * \param fuelType The type of fuel
     * \param amount The amount of fuel replenished
     * \param stock The current stock of the pump
     * \param capacity The capacity of the pump
     * \param timestamp The timestamp of the message
     * \return The JSON document
    */
    JsonDocument create_fuel_replenishment_message(String thingId, String thingNamespace, FUEL_TYPE fuelType, double amount, double stock, int capacity, String timestamp);

    /**
     * \brief Create a supply error message
     * \param thingId The ID of the thing
     * \param thingNamespace The namespace of the thing
     * \param error The error message
     * \param timestamp The timestamp of the message
     * \return The JSON document
    */
    JsonDocument create_supply_error_message(String thingId, String thingNamespace, String error, String timestamp);

    /**
     * \brief Serialize a JSON document
     * \param doc The JSON document to serialized
     * \param serializedDoc The destination serialized JSON document
     * \return ESP_OK if the JSON document was serialized successfully, ESP_FAIL otherwise
    */
    esp_err_t serialize_message(JsonDocument& doc, String& serializedDoc);

    /**
     * \brief Deserialize a JSON message
     * \param message The message to deserialize
     * \param length The length of the message
     * \param doc The destination JSON document
     * \return ESP_OK if the message was deserialized successfully, ESP_FAIL otherwise
    */
    esp_err_t deserialize_message(char* message, size_t length,  JsonDocument& doc);

    MESSAGE_TYPE get_message_type(JsonDocument& doc);
}