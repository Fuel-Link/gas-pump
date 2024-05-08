#include <PumpInteraction.h>

PumpInteraction::PumpInteraction(FUEL_TYPE fuelType, uint32_t capacity, uint32_t stock)
    : fuelType(fuelType), capacity(capacity), stock(stock) {
    // Configure pump status LED pin as output
    pinMode(PUMP_STATUS_LED_PIN, OUTPUT);

    // Configure pump Button pin as input
    pinMode(PUMP_BUTTON_PIN, INPUT_PULLUP);

    // Define pump motor pin as output
    pinMode(PUMP_MOTOR_CONTROLLER_PIN, OUTPUT);

    // Set the default motor mode to off
    digitalWrite(PUMP_MOTOR_CONTROLLER_PIN, LOW);

    // Initialize random number generator with analog pin 0
    randomSeed(analogRead(0)); 

    // The Gas pump starts at locked
    ESP_ERROR_CHECK(lock_pump());
}

PumpInteraction::~PumpInteraction() { }

esp_err_t PumpInteraction::unlock_pump(){
    if(locked == PUMP_LOCKED)
        return ESP_FAIL;

    digitalWrite(PUMP_STATUS_LED_PIN, HIGH);
    this->locked = PUMP_LOCKED;

    return ESP_OK;
}

esp_err_t PumpInteraction::lock_pump(){
    if(locked == PUMP_UNLOCKED)
        return ESP_FAIL;

    digitalWrite(PUMP_STATUS_LED_PIN, LOW);
    this->locked = PUMP_UNLOCKED;

    return ESP_OK;
}

void PumpInteraction::set_capacity(uint32_t capacity){
    this->capacity = capacity;
}

esp_err_t PumpInteraction::set_stock(double stock){
    if(stock > capacity)
        return ESP_FAIL;
    
    this->stock = stock;

    return ESP_OK;
}

void PumpInteraction::set_fuel_type(FUEL_TYPE type){
    this->fuelType = type;
}

esp_err_t PumpInteraction::replenish_stock(uint32_t amount){
    uint32_t currentCapacity = get_stock();

    return set_stock(currentCapacity + amount);
}

esp_err_t PumpInteraction::supply_fuel(double &suppliedAmmount){
    if(stock < MIN_FUEL_SUPPLY_IN_LITERS)
        return ESP_FAIL;

    ESP_ERROR_CHECK(unlock_pump());

    unsigned long startTime = millis();
    bool buttonPressed = false;

    // Waits x seconds for the user to press the button
    while (millis() - startTime < PUMP_ACTIVATED_WAITING_TIME_IN_SEC * 1000) {
        if (digitalRead(PUMP_BUTTON_PIN) == LOW) { // check if button is pressed
            buttonPressed = true; // set buttonPressed to true
            break; 
        }
    }

    // Activate pump
    digitalWrite(PUMP_MOTOR_CONTROLLER_PIN, HIGH);

    // Wait for the user to stop using the button
    while(digitalRead(PUMP_BUTTON_PIN) == LOW);

    // Shutoff pump
    digitalWrite(PUMP_MOTOR_CONTROLLER_PIN, LOW);

    // Register a random number, between 1 and half of the current stock, with max at 100
    uint32_t maxAmmount = get_stock() / 2;
    if(maxAmmount > MAX_FUEL_SUPPLY_IN_LITERS)
        maxAmmount = MAX_FUEL_SUPPLY_IN_LITERS;
    
    // Generate a random integer value between 0 and 9999
    int randomInt = random(10000); 
    
    // Scale the random integer to the desired range of double numbers
    suppliedAmmount = map(randomInt, 0, 9999, MIN_FUEL_SUPPLY_IN_LITERS * 10000, maxAmmount * 10000) / 10000.0;

    // Decrement stock level
    set_stock(get_stock() - suppliedAmmount);

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