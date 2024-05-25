//#define WIFI_CREDENTIALS_MOBILE

#ifdef WIFI_CREDENTIALS_MOBILE
    //! SSID of your internet enabled WiFi network
    const char WIFI_SSID[] = "FBI surveillance van";
    //! Password of your internet enabled WiFi network
    const char WIFI_PASSWORD[] = "open_up90";
    //! IP address of the MQTT broker
    const char MQTT_SERVER_IP[] = "192.168.167.79";
#else
    //! SSID of your internet enabled WiFi network
    const char WIFI_SSID[] = "familia-silva";
    //! Password of your internet enabled WiFi network
    const char WIFI_PASSWORD[] = "PaoDe-Presunto24";
    //! IP address of the MQTT broker
    const char MQTT_SERVER_IP[] = "192.168.68.111";
#endif

//! Port of the MQTT broker
const int MQTT_SERVER_PORT = 8000;

const char URI[] = "/";

