#include <PumpInteraction.h>

PumpInteraction::PumpInteraction(FUEL_TYPE fuelType, uint32_t capacity, uint32_t stock)
    : fuelType(fuelType), capacity(capacity), stock(stock) { }

PumpInteraction::~PumpInteraction() { }

esp_err_t PumpInteraction::init_pump(){
    Serial.println("Initializing pump");

    // Configure pump status LED pin as output
    pinMode(PUMP_STATUS_LED_PIN, OUTPUT);
    Serial.println(" - Pump status LED pin configured");

    // Configure pump Button pin as input
    pinMode(PUMP_BUTTON_PIN, INPUT);
    Serial.println(" - Pump button pin configured");

    // Define pump motor pin as output
    pinMode(PUMP_CONTROL_RELAY_PIN, OUTPUT);
    Serial.println(" - Pump relay motor pin configured");

    // Set the default motor mode to off
    digitalWrite(PUMP_CONTROL_RELAY_PIN, LOW);
    Serial.println(" - Pump relay motor pin set to off");

    // The Gas pump starts at locked
    return lock_pump();
}

esp_err_t PumpInteraction::unlock_pump(){
    if(locked == PUMP_LOCKED)
        return ESP_FAIL;

    digitalWrite(PUMP_STATUS_LED_PIN, HIGH);
    this->locked = PUMP_LOCKED;

    Serial.println(" - Pump unlocked");

    return ESP_OK;
}

esp_err_t PumpInteraction::lock_pump(){
    if(locked == PUMP_UNLOCKED)
        return ESP_FAIL;

    digitalWrite(PUMP_STATUS_LED_PIN, LOW);
    this->locked = PUMP_UNLOCKED;

    Serial.println(" - Pump locked");

    return ESP_OK;
}

void PumpInteraction::set_capacity(uint32_t capacity){
    this->capacity = capacity;
}

esp_err_t PumpInteraction::set_stock(double stock){
    if(stock > capacity)
        return ESP_FAIL;
    
    this->stock = stock;

    Serial.println(" - Stock set to: " + String(stock) + "L");

    return ESP_OK;
}

void PumpInteraction::set_fuel_type(FUEL_TYPE type){
    this->fuelType = type;
}

esp_err_t PumpInteraction::replenish_stock(uint32_t amount){
    uint32_t currentCapacity = get_stock();

    return set_stock(currentCapacity + amount);
}

esp_err_t PumpInteraction::supply_fuel(double &suppliedAmount){
    if(stock < MIN_FUEL_SUPPLY_IN_LITERS)
        return ESP_FAIL;

    ESP_ERROR_CHECK(unlock_pump());

    unsigned long startTime = millis();
    bool buttonPressed = false;

    Serial.println(" - Waiting for user to press button");

    // Waits x seconds for the user to press the button
    while (millis() - startTime < PUMP_ACTIVATED_WAITING_TIME_IN_SEC * 1000) {
        if (digitalRead(PUMP_BUTTON_PIN) == HIGH) { // check if button is pressed
            buttonPressed = true; // set buttonPressed to true
            break; 
        }
    }

    if(!buttonPressed){
        ESP_ERROR_CHECK(lock_pump());
        Serial.println(" - Button not pressed, aborting fuel supply");
        return ESP_FAIL;
    }

    Serial.println(" - Button pressed. Supplying fuel...");

    // Activate pump
    digitalWrite(PUMP_CONTROL_RELAY_PIN, HIGH);

    // Wait for the user to stop using the button
    while(digitalRead(PUMP_BUTTON_PIN) == HIGH){
        suppliedAmount += 0.01;
        Serial.print("\r - Supplied amount: " + String(suppliedAmount) + "L");
    }
    Serial.println();

    // Shutoff pump
    digitalWrite(PUMP_CONTROL_RELAY_PIN, LOW);

    Serial.println(" - Fuel supplied");

    // Decrement stock level
    set_stock(get_stock() - suppliedAmount);

    ESP_ERROR_CHECK(lock_pump());

    return ESP_OK;
}

FUEL_TYPE PumpInteraction::get_fuel_type(){
    return fuelType;
}

uint32_t PumpInteraction::get_capacity(){
    return capacity;
}

uint32_t PumpInteraction::get_stock(){
    return stock;
}