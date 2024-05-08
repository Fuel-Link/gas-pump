#pragma once 

#include <Arduino.h>
#include <DataTypes.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
#define PUMP_STATUS_LED_PIN 2
#define PUMP_BUTTON_PIN 4
#define PUMP_MOTOR_CONTROLLER_PIN 6
#define PUMP_UNLOCKED 1
#define PUMP_LOCKED 0
#define PUMP_ACTIVATED_WAITING_TIME_IN_SEC 60
#define MAX_FUEL_SUPPLY_IN_LITERS 100
#define MIN_FUEL_SUPPLY_IN_LITERS 1

/*
    ##########################################################################
    ############            CommsHandler declaration              ############
    ##########################################################################
*/
/*! \class PumpInteraction
    \brief Handles interactions with the physical pump.
*/
class PumpInteraction {
   private:
    //!< Pump lock status
    boolean locked;

    //!< Fuel type
    FUEL_TYPE fuelType;

    //!< Pump capacity
    uint32_t capacity;

    //!< Pump current volume
    double stock;

    //!< Mutex for pump lock
    esp_err_t unlock_pump();

    //!< Mutex for pump unlock
    esp_err_t lock_pump();

    /**
     * \brief Set the capacity of the pump
     * \param capacity The capacity of the pump
    */
    void set_capacity(uint32_t capacity);

    /**
     * \brief Set the stock of the pump
     * \param stock The stock of the pump
     * \return ESP_OK if the stock was set successfully, ESP_FAIL otherwise
    */
    esp_err_t set_stock(double stock);

    /**
     * \brief Set the fuel type of the pump
     * \param type The fuel type of the pump
    */
    void set_fuel_type(FUEL_TYPE type);

   public:
    /**
     * \brief Constructor for PumpInteraction class.
    */
    PumpInteraction(FUEL_TYPE fuelType, uint32_t capacity, uint32_t stock);

    /**
     * \brief Destructor for PumpInteraction class.
    */
    ~PumpInteraction();

    /**
     * \brief Handle a repleshment (fuel) of the pump stock
     * \param amount The amount of fuel to replenish
     * \return ESP_OK if the stock was replenished successfully, ESP_FAIL otherwise
    */
    esp_err_t replenish_stock(uint32_t amount);

    /**
     * \brief Handle a fuel supply from the pump
     * \param suppliedAmount The amount of fuel supplied. A reference to the variable should 
     *      be supplied and will be updated with the amount of fuel, if the fuel was supplied successfully
     * \return ESP_OK if the fuel was supplied successfully, ESP_FAIL otherwise
    */
    esp_err_t supply_fuel(double &suppliedAmount); 

    /**
     * \brief Get the fuel type of the pump
     * \return The fuel type of the pump
    */
    FUEL_TYPE get_fuel_type();

    /**
     * \brief Get the capacity of the pump
     * \return The capacity of the pump
    */
    uint32_t get_capacity();

    /**
     * \brief Get the stock of the pump
     * \return The stock of the pump
    */
    uint32_t get_stock();

};