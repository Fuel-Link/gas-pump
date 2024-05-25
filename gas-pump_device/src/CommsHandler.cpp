#include <CommsHandler.h>

CommsHandler::CommsHandler(String thingId, String thingNamespace) {
    this->thingId = thingId;
    this->thingNamespace = thingNamespace;
    this->inTopic = String(MODULE_NAME) + "/" + thingNamespace + ":" + thingId + "/" + String(DOWNLINK_CHANNEL);
    this->outTopic = String(MODULE_NAME) + "/" + thingNamespace + ":" + thingId + "/" + String(UPLINK_CHANNEL);
}

CommsHandler::~CommsHandler() {}

/**
 * This function should be in a shared folder with both this and main.cpp files
*/
void CommsHandler::connect_wifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("Connecting to %s", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print(" - Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    initialize_ntp_client();
}

boolean CommsHandler::connected_to_wifi(){
    return WiFi.isConnected();
}

 void CommsHandler::connect_mqtt(){
    if (!WiFi.isConnected()) {
        Serial.println("Error: WiFi not connected");
        return;
    }

    // initialize mqtt client
    mqttClient.begin(webSockClient);

    Serial.printf("Connecting to MQTT broker at %s:%d%s", MQTT_SERVER_IP, MQTT_SERVER_PORT, URI);
    Serial.println();

    connect_to_host:
        Serial.println(" - Connecting to WebSocket");
        // connect to host with MQTT over WebSocket
        webSockClient.disconnect();
        webSockClient.begin(MQTT_SERVER_IP, MQTT_SERVER_PORT, URI, "mqtt");
        webSockClient.setReconnectInterval(2000);

        Serial.println(" - Connecting to MQTT broker");
        while (!mqttClient.connect(thingId.c_str())) {
            Serial.print(".");
            delay(1000);
            if (!webSockClient.isConnected()) {
                Serial.println(" - WebSocketsClient disconnected");
                goto connect_to_host;
            }
        }

    Serial.println(" - Connected to MQTT broker");

    // subscribe topic and callback which is called when /hello has come
    mqttClient.subscribe(inTopic, [this](const String& payload, const size_t size) {
        char* topic = (char*)this->inTopic.c_str();
        mqtt_message_callback(topic, (byte*)payload.c_str(), size);
    });

    Serial.print(" - Subscribed to MQTT channel: ");
    Serial.println(inTopic);
    Serial.println();
}

boolean CommsHandler::connected_to_mqtt(){
    mqttClient.update();  
    return mqttClient.isConnected() && webSockClient.isConnected();
}

long CommsHandler::get_time_in_ms(timeval& currentTime){
    String timeStr = NTP.getTimeStr(currentTime); // Get the time string
    long hours = timeStr.substring(0, 2).toInt(); // Extract hours
    long minutes = timeStr.substring(3, 5).toInt(); // Extract minutes
    long seconds = timeStr.substring(6, 8).toInt(); // Extract seconds
    String millisecondsStr = timeStr.substring(9, 12);
    long milliseconds = millisecondsStr.toInt(); // Convert milliseconds string to integer
    long totalTimeMs = (hours * 3600 + minutes * 60 + seconds) * 1000 + milliseconds;
    return totalTimeMs;
}

String CommsHandler::get_time_string(timeval& currentTime){
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%z", localtime(&currentTime.tv_sec));
    return String(buffer);
}

int CommsHandler::get_time_year(timeval& currentTime){
    return localtime(&currentTime.tv_sec)->tm_year + 1900;
}

void CommsHandler::process_sync_event (NTPEvent_t ntpEvent) {
    switch (ntpEvent.event) {
        case timeSyncd:
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            Serial.printf ("[NTP-event] %s", NTP.ntpEvent2str (ntpEvent));
            Serial.println();
            break;
        default:
            break;
    }
    syncEventTriggered = true;
}

void CommsHandler::initialize_ntp_client(){
    NTP.onNTPSyncEvent ([this] (NTPEvent_t event) {
        this->ntpEvent = event;
        this->syncEventTriggered = true;
        process_sync_event (ntpEvent);
    });

    NTP.setTimeZone (NTP_TIMEZONE);
    NTP.setInterval (600);
    NTP.setNTPTimeout (NTP_TIMEOUT);
    // NTP.setMinSyncAccuracy (5000);
    // NTP.settimeSyncThreshold (3000);
    NTP.begin (NTP_SERVER);

    // Wait for the response from NTP server
    Serial.println("Initializing NTP client: ");
    Serial.print(" - Fetching current time");
    timeval currentTime;
    do{
        Serial.print(".");
        delay(2500);
        gettimeofday(&currentTime, NULL);
    }while(!syncEventTriggered || get_time_year(currentTime) == 1970);

    Serial.print(" - NTP client initialized with time: "); Serial.println(NTP.getTimeDateStringUs());
    Serial.println();

}

bool CommsHandler::publish_message(const char* message, size_t length) {
    if (!WiFi.isConnected()) {
        Serial.println("Error: WiFi not connected");
        return false;
    }

    if (!mqttClient.isConnected()) {
        Serial.println("Error: MQTT broker not connected");
        return false;
    }

    if(length == 0){
        Serial.println("Error: The message is empty");
        return false;
    }

    Serial.print("Publishing message to MQTT broker: ");
    Serial.print(" - Topic: "); 
    Serial.println(outTopic);
    Serial.print(" - Message: ");
    Serial.println(message);
    Serial.print(" - Length: ");
    Serial.println(length);
    Serial.println();

    if(!mqttClient.publish(outTopic.c_str(), message, length)){
        Serial.println("Error: Failed to publish message to MQTT broker");
        return false;
    }

    return true;
}