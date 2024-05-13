#pragma once

/*
    ##########################################################################
    ############            MESSAGE_TYPE declaration              ############
    ##########################################################################
*/
enum MESSAGE_TYPE {
    PUMP_INIT, //!< Sent by the pump to Ditto, for initialization
    SUPPLY_AUTHORIZED,  //!< Sent by Ditto to the pump, to authorize a supply
    SUPPLY_COMPLETED,   //!< Sent by the pump to Ditto, to confirm a supply
    FUEL_REPLENISHMENT, //!< Sent by pump to the Ditto, signalling that the fuel has been replenished
    SUPPLY_ERROR,    //!< Sent by the pump to Ditto, to report an error
    UNKNOWN //!< Unknown/Error message type
};

/*
    ##########################################################################
    ############             FUEL_TYPE declaration                ############
    ##########################################################################
*/
enum FUEL_TYPE {
    DIESEL,
    PETROL,
    LPG
};