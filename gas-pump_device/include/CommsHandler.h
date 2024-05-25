#pragma once

//! \attention for WiFi include to always be first
#include <WiFi.h>
#include <WiFiCredentials.h>
#include <ESPNtpClient.h>
#include <WebSocketsClient.h>  // include before MQTTPubSubClient.h
#include <MQTTPubSubClient.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
//! Attention: Please refer to the Readme file as to configure the WiFi and MQTT credentials.

//! MQTT defaults
#define MODULE_NAME "gas-pump"
#define DOWNLINK_CHANNEL "downlink"
#define UPLINK_CHANNEL "uplink"

//! NTP configurations
#define SHOW_TIME_PERIOD 1000
#define NTP_TIMEOUT 5000
#define NTP_TIMEZONE TZ_Europe_London

/*
    ##########################################################################
    ############            CommsHandler declaration              ############
    ##########################################################################
*/
/*! \class CommsHandler
    \brief Handles interactions with the CommsHandler.
*/
class CommsHandler {
   private:
    //!< WiFi and MQTT clients
    //WiFiClient espClient;
    WebSocketsClient webSockClient;
    MQTTPubSub::PubSubClient<MQTT_MAX_PACKET_SIZE> mqttClient;

    //! MQTT configurations
    String inTopic;
    String outTopic;
    String thingId;
    String thingNamespace;

    //! NTP configurations
    const PROGMEM char* NTP_SERVER = "pool.ntp.org";
    boolean syncEventTriggered = false; // True if a time even has been triggered
    NTPEvent_t ntpEvent; // Last triggered event

   public:
    /**
     *  \brief Constructor for CommsHandler class.
     *  \param thingId Thing ID of the device.
     *  \param thingNamespace Namespace of the device.
    */
    CommsHandler(String thingId, String thingNamespace);

    //! \brief Destructor for CommsHandler class.
    ~CommsHandler();

    /*! \brief Connect to the WiFi network.
    */
    void connect_wifi();

    /*! \brief Check if connected to the WiFi network.
        \return True if connected, false otherwise.
    */
    boolean connected_to_wifi();

    /*! \brief Connect to the MQTT broker.
    */
    void connect_mqtt();

    /*! \brief Check if connected to the MQTT broker.
        \return True if connected, false otherwise.
    */
    boolean connected_to_mqtt();

    /*! \brief Publish a message to a topic in the MQTT broker.
        \param message a Pointer to the message to be published.
        \param length Length of the message.
        \return True if the message was published, false otherwise.
    */
    bool publish_message(const char* message, size_t length);

    /*! \brief Callback function for MQTT messages. Present in the `main.cpp` file.
        \param topic Topic of the message.
        \param payload Payload of the message.
        \param length Length of the message.
        \note This function should be located in the main.cpp file, as to 
            access the processing functions of other modules.
    */
    static void mqtt_message_callback(char* topic, byte* payload, unsigned int length);

    /*! \brief Initialize the NTP client.
    */
    void initialize_ntp_client();

    /*! \brief Process the NTP event.
        \param ntpEvent NTP event to be processed.
    */
    void process_sync_event (NTPEvent_t ntpEvent);

    /*! \brief Get the current time in milliseconds.
        \param currentTime Current time.
        \return Current time in milliseconds.
    */
    long get_time_in_ms(timeval& currentTime);

    /*! \brief Get the current time in string format.
        \param currentTime Current time.
        \return Current time in string format, according to ISO 8601.
        \example "2024-03-24T12:30:00Z"
    */
    String get_time_string(timeval& currentTime);

    /*! \brief Get the current time in year.
        \param currentTime Current time.
        \return Current year.
    */
    int get_time_year(timeval& currentTime);
};