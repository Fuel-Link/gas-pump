# What is Eclipse-Ditto

Eclipse Ditto is a technology in the IoT realm implementing a software pattern called “digital twins”. A digital twin is a virtual, cloud based, representation of his real world counterpart (real world “Things”, e.g. devices like sensors, smart heating, connected cars, smart grids, EV charging stations, …).

The technology mirrors potentially millions and billions of digital twins residing in the digital world with physical “Things”. This simplifies developing IoT solutions for software developers as they do not need to know how or where exactly the physical “Things” are connected.

With Ditto a thing can just be used as any other web service via its digital twin. You can check out more about ditto [here](https://eclipse.dev/ditto/intro-overview.html).

## Starting Ditto

In a terminal window, navigate to the `docker` folder, then execute the following command:

```bash
docker compose up -d
```

This will initiate the docker containers used in the Ditto service. Wait some time and then try to access the website frontend at [http://localhost:8080/ui/](`http://localhost:8080/ui/`).

### Stopping Ditto

In the same folder, execute the following command:

```bash
docker compose down
```

This will stop and remove all containers from the system. Don't worry, since the data will still be saved. However, upon the next restart, the system will take some time to recover all the data.

# Configuring the system

Follow the next sub-sections in order, to properly configure the Ditto service.

## Creating the Plate Reader

Based on the Linked-State JSON definition in the file `gas-pump.jsonld`, located in this directory, create the Ditto object to mirror our gas-pump device using the following request. For that, in a terminal windows execute the following request, with `deviceID` as the name to give to the device (*hint*: use something referencible and unique):

```bash
curl -u ditto:ditto -X POST -H 'Content-Type: application/json' -d '{
    "title": "Gas Pump",
    "description": "Gas Pump controller device",  
    "definition": "https://raw.githubusercontent.com/Fuel-Link/gas-pump/main/ditto/gas-pump.jsonld",
    "attributes": {
        "deviceID": "gas_pump_1234"
    }
}' 'http://localhost:8080/api/2/things'
```

The device can also be created with other attributes to better identify it's capabilities and location. This are:
```json
(...)
    "attributes": {
        "deviceID": {
			"title": "Device ID",
			"type": "string",
			"readOnly": true
		},
		"capacity": {
			"title": "Capacity of the fuel tank",
			"type": "number",
			"unit": "schema:capacity"
		},
		"location": {
			"title": "Location",
			"type": "geo:point",
			"description": "GPS coordinates of the mounted location (latitude, longitude)"
		}
    }
(...)
```

Creating the Plate object will output it's generated **ThingID** (annotate this value), which is in format `org.eclipse.ditto:<Generated_Thing>`, since this value will be placed in the ESP32 License Plate Reader, for it's identification the Ditto Service.

## Connecting to MQTT

To connect the ESP32 to the Ditto Service, we use MQTT due to it's simplicity and available robust libraries
for Arduino. For that, we need to configure Ditto to listen to the MQTT broker and configured topics. In a 
terminal window, execute the following request:

```bash
curl -u ditto:ditto -X POST -H 'Content-Type: application/json' -d '{
    "targetActorSelection": "/system/sharding/connection",
    "headers": {
    	"aggregate": false
    },
    "piggybackCommand": {
        "type": "connectivity.commands:createConnection",
        "connection": {
            "id": "mqtt_localhost_connection",
            "name": "mqtt",
            "connectionType": "mqtt",
            "connectionStatus": "open",
            "failoverEnabled": true,
            "uri": "tcp://192.168.68.111:1883",
            "sources": [
                {
                "addresses": [
                    "gas_pump/+/uplink/#"
                ],
                "qos": 0,
                "authorizationContext": [
                    "nginx:ditto"
                ],
                "filters": []
                }
            ],
            "targets": [
                {
                "address": "gas_pump/{{ thing:id }}/downlink",
                "topics": [
                    "_/_/things/twin/events",
                    "_/_/things/live/messages"
                ],
                "qos": 0,
                "authorizationContext": [
                    "nginx:ditto"
                ]
                }
            ],
            "mappingContext": {
                "mappingEngine": "JavaScript",
                "options": {
                "incomingScript": "function mapToDittoProtocolMsg(\n    headers, \n    textPayload, \n    bytePayload,\n    contentType\n) { const jsonData = JSON.parse(textPayload || \"{}\"); // Handle empty payload\n    const thingId = jsonData.thingId.split(':');\n    const timestamp = jsonData.timestamp;\n    const imageId = jsonData.imageId.toString(); // Ensure imageId is a string\n    const url = jsonData.url;\n\n    const value = { \n        imageCaptured: {\n            properties: {\n                timestamp: {  \n                    value: jsonData.timestamp \n                },\n                imageId: { \n                    properties: { \n                        value: jsonData.imageId \n                    } \n                }, \n                url: { \n                    properties: { \n                        value: jsonData.url \n                    } \n                },\n            } \n        }\n    };    \n  \n    return Ditto.buildDittoProtocolMsg(\n        thingId[0],                 // thing namespace\n        thingId[1],                 // thing ID of the device\n        'things',                   // (group) we deal with a thing\n        'twin',                     // (channel) we want to update the twin\n        'commands',                 // (criterion) create a command to update the twin\n        'modify',                   // (action) modify the twin\n        '/features',  // (path) modify all features at once\n        headers,                    // pass the mqtt headers\n        value\n    );\n}\n  "
                }
            }
        }
    }
}' 'http://localhost:8080/devops/piggyback/connectivity?timeout=10'
```

Ditto needs incomming Payload Mapping to transform messages from the format sent by the arduino, to the Ditto Protocol Message Format. The developed function can be observed in the `incomingJavaScriptMapping.js` and is already present in the Connection String sent in the request above.

**Attention:** If you use the Ditto UI website, please be carefull about the inclusion of the Payload Mapping function, sometimes when placing the payload mapping function in the side menu, it's inclusion in the connection JSON get's mis-formatted and `content-type` errors start to appear. This is due to new format specifications and Ditto treating MQTT version 3.1 connection to MQTT version 5.

## Connecting to Kafka

Our Architecture layout uses Kafka to take Message events from Ditto (When the Plate-Reader captures an image and publishes in Ditto), perform their analysis, processing and then forward the data to the Composer. For that, we need to configure Ditto to publish messages into Kafka topics (In this case, the Composer isn't ment to interact with the twin itself, so no source topics are provided). Open a terminal window and execute the following request or add the connection in the Ditto UI:

```bash
curl -u ditto:ditto -X POST -H 'Content-Type: application/json' -d '{
    "targetActorSelection": "/system/sharding/connection",
    "headers": {
    	"aggregate": false
    },
    "piggybackCommand": {
        "name": "Kafka 2.x",
        "connectionType": "kafka",
        "connectionStatus": "open",
        "failoverEnabled": true,
        "uri": "tcp://localhost:29092",
        "specificConfig": {
            "bootstrapServers": "localhost:29092",
            "saslMechanism": "plain"
        },
        "sources": [ ],
        "targets": [
            {
            "address": "imageCaptured",
            "qos": 1,
            "topics": [
                "_/_/things/twin/events?extraFields=thingId,attributes/_parents,features/imageCaptured/properties",
                "_/_/things/twin/events",
                "_/_/things/live/messages"
            ],
            "authorizationContext": [
                "nginx:ditto"
            ]
            }
        ]
    }
}' 'http://localhost:8080/devops/piggyback/connectivity?timeout=10'
```

`uri` and `specificConfig.bootstrapServers` values can be changes, depending on the IP address used in the connection. In this sense, don't forget that the Kafka Cluster and Ditto should be in the same network, because internally Ditto uses the `kafka:<port>` to connect to the Cluster, because of this, they need to be in the same network, as for the address resolution to work, or at least share the same DNS server.

If authorization or other issues arise in establishing the connection, please use the Web UI to establish the connection. For that, connection templates can also be used.

## References

* [Tutorial](https://www.hivemq.com/blog/hands-on-guide-using-mqtt-hivemq-eclipse-ditto-digital-twins-iiot/) to configue MQTT and Ditto service
* Eclipse Ditto [examples](https://github.com/eclipse-ditto/ditto-examples/tree/master/mqtt-bidirectional) with MQTT bidirectional communication, including [Arduino project](https://github.com/eclipse-ditto/ditto-examples/blob/master/mqtt-bidirectional/iot-device/octopus/src/main.ino)
* Eclipse Ditto MQTT documentation [page](https://eclipse.dev/ditto/1.5/connectivity-protocol-bindings-mqtt.html)

