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

Example of the terminal output when creating a new Gas-Pump thing:

```json
{"thingId":"org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20","policyId":"org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20","definition":"https://raw.githubusercontent.com/Fuel-Link/gas-pump/main/ditto/gas-pump.jsonld","attributes":{"deviceID":"gas_pump_1234"}}
```

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
            "uri": "tcp://192.168.167.79:1884",
            "sources": [
                {
                "addresses": [
                    "gas-pump/+/uplink/#"
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
                "address": "gas-pump/{{ thing:id }}/downlink",
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
                        "incomingScript": "function mapToDittoProtocolMsg(\n    headers, \n    textPayload, \n    bytePayload,\n    contentType\n) {\n\n    const jsonData = JSON.parse(textPayload || \"{}\"); // Handle empty payload\n    const thingId = jsonData.thingId.split(':');\n\n    let value = {};\n    \n    if(jsonData.msgType === 0) { // Pump Init message\n        value = {\n            pump_init: {\n                properties: {\n                    timestamp: {  \n                        value: jsonData.timestamp \n                    },\n                    stock: { \n                        properties: { \n                            value: jsonData.stock \n                        } \n                    },\n                    capacity: {\n                        properties: {\n                            value: jsonData.capacity\n                        }\n                    },\n                    msgType: { \n                        properties: { \n                            value: jsonData.msgType \n                        } \n                    }\n                } \n            }\n        };\n    } else if(jsonData.msgType === 2) { // Supply completed message\n        value = { \n            supply_completed: {\n                properties: {\n                    timestamp: {  \n                        value: jsonData.timestamp \n                    },\n                    amount: { \n                        properties: { \n                            value: jsonData.amount \n                        } \n                    }, \n                    stock: {\n                        properties: {\n                            value: jsonData.stock\n                        }\n                    },\n                    msgType: { \n                        properties: { \n                            value: jsonData.msgType \n                        } \n                    },\n                } \n            }\n        };\n    } else if (jsonData.msgType === 3) {    // Fuel replenishment message\n        value = { \n            fuel_replenishment: {\n                properties: {\n                    timestamp: {  \n                        value: jsonData.timestamp \n                    },\n                    amount: { \n                        properties: { \n                            value: jsonData.amount \n                        } \n                    }, \n                    stock: { \n                        properties: { \n                            value: jsonData.stock \n                        } \n                    },\n                    msgType: { \n                        properties: { \n                            value: jsonData.msgType \n                        } \n                    }\n                } \n            }\n        };\n    } else if (jsonData.msgType === 4) {    // Supply error message\n        value = {\n            supply_error: {\n                properties: {\n                    timestamp: {  \n                        value: jsonData.timestamp \n                    },\n                    error: { \n                        properties: { \n                            value: jsonData.error \n                        } \n                    }, \n                    msgType: { \n                        properties: { \n                            value: jsonData.msgType \n                        } \n                    }\n                } \n            }\n        };\n    } else {    // error\n        throw new Error('Invalid message type');\n    }\n  \n    return Ditto.buildDittoProtocolMsg(\n        thingId[0],                 // thing namespace\n        thingId[1],                 // thing ID of the device\n        'things',                   // (group) we deal with a thing\n        'twin',                     // (channel) we want to update the twin\n        'commands',                 // (criterion) create a command to update the twin\n        'modify',                   // (action) modify the twin\n        '/features',                // (path) modify all features at once\n        headers,                    // pass the mqtt headers\n        value\n    );\n}\n  "
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
        "sources": [
            {
                "addresses": ["gas-pump_downlink"],
                "consumerCount": 1,
                "qos": 1,
                "authorizationContext": ["ditto:ditto"],
                "enforcement": {
                    "input": "{{ header:thingId }}",
                    "filters": ["{{ entity:id }}"]
                },
                "headerMapping": {
                    "message-id": "{{ header:correlation-id }}",
                    "content-type": "application/vnd.eclipse.ditto+json"
                },
                "payloadMapping": ["Ditto"],
                "replyTarget": {
                    "enabled": true,
                    "address": "kafka-errors",
                    "headerMapping": {
                        "message-id": "{{ header:correlation-id }}",
                        "content-type": "application/vnd.eclipse.ditto+json"
                    },
                    "expectedResponseTypes": ["response", "error", "nack"]
                },
                "acknowledgementRequests": {
                    "includes": []
                },
                "declaredAcks": []
            }
        ],
        "targets": [
            {
            "address": "gas-pump_uplink",
            "qos": 1,
            "topics": [
                "_/_/things/twin/events?extraFields=thingId,attributes/_parents",
                "_/_/things/twin/events",
                "_/_/things/live/messages"
            ],
            "authorizationContext": [
                "nginx:ditto"
            ]
            }
        ],
        "clientCount": 1,
        "failoverEnabled": true,
        "validateCertificates": true,
        "processorPoolSize": 1,
        "specificConfig": {
            "saslMechanism": "plain",
            "bootstrapServers": "kafka:29092"
        },
        "tags": [],
        "mappingContext": {
            "mappingEngine": "JavaScript",
            "options": {
            "incomingScript": "function mapToDittoProtocolMsg(\n    headers, \n    textPayload, \n    bytePayload,\n    contentType\n) {\n\n    const jsonData = JSON.parse(textPayload || \"{}\"); // Handle empty payload\n    const thingId = jsonData.thingId.split(':');\n    const jsonValue = jsonData.value;\n\n    // Only handle authorized_supply messages\n    let channel = \"/features/authorize_supply\";\n    data = {\n        properties: {\n            timestamp: {\n                properties: { \n                    value: jsonValue.timestamp \n                }\n            },\n            authorization: { \n                properties: { \n                    value: jsonValue.authorization \n                } \n            },\n            msgType: { \n                properties: { \n                    value: jsonValue.msgType \n                } \n            }\n        } \n    };\n  \n    return Ditto.buildDittoProtocolMsg(\n        thingId[0],                 // thing namespace\n        thingId[1],                 // thing ID of the device\n        'things',                   // (group) we deal with a thing\n        'twin',                     // (channel) we want to update the twin\n        'commands',                 // (criterion) create a command to update the twin\n        'modify',                   // (action) modify the twin\n        channel,                    // (path) modify only the chosen feature\n        headers,                    // pass the mqtt headers\n        value\n    );\n}\n  "
            }
        }
    }
}' 'http://localhost:8080/devops/piggyback/connectivity?timeout=10'
```

`uri` and `specificConfig.bootstrapServers` values can be changes, depending on the IP address used in the connection. In this sense, don't forget that the Kafka Cluster and Ditto should be in the same network, because internally Ditto uses the `kafka:<port>` to connect to the Cluster, because of this, they need to be in the same network, as for the address resolution to work, or at least share the same DNS server.

If authorization or other issues arise in establishing the connection, please use the Web UI to establish the connection. For that, connection templates can also be used.

## Publishing to ditto, using Kafka

First, launch a Kafka producer application, using Docker. Use the following command:
```bash
docker run -it --rm --network=host confluentinc/cp-kafka:6.2.0 kafka-console-producer --bootstrap-server localhost:9092 --topic gas-pump_downlink
```

Then paste the following JSON message, which is serialized version of the one below:

```json
{"thingId":"org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20","topic":"org.eclipse.ditto/9b0ec976-3012-42d8-b9ea-89d8b208ca20/things/twin/commands/modify","path":"/features/authorize_supply/properties/","messageId":"{{ uuid() }}","timestamp":"{{ timestamp }}","source":"gas-pump","method":"update","target":"/features/authorize_supply","value":{"msgType":1,"thingId":"org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20","topic":"org.eclipse.ditto/9b0ec976-3012-42d8-b9ea-89d8b208ca20/things/twin/commands/modify","path":"/features/authorize_supply/properties/","authorization":1,"timestamp":"2024-05-18T12:03:44+0100"}}
```

```json
{
    "thingId": "org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20",
    "topic": "org.eclipse.ditto/9b0ec976-3012-42d8-b9ea-89d8b208ca20/things/twin/commands/modify",
    "path": "/features/authorize_supply/properties/",
    "messageId": "{{ uuid() }}",
    "timestamp": "{{ timestamp }}", 
    "source": "gas-pump", 
    "method": "update", 
    "target": "/features/authorize_supply",
    "value": {    
        "msgType": 1,
        "thingId": "org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20",
        "topic": "org.eclipse.ditto/9b0ec976-3012-42d8-b9ea-89d8b208ca20/things/twin/commands/modify",
        "path": "/features/authorize_supply/properties/",
        "authorization": 1,
        "timestamp": "2024-05-18T12:03:44+0100"
    }
}
```

## References

* [Tutorial](https://www.hivemq.com/blog/hands-on-guide-using-mqtt-hivemq-eclipse-ditto-digital-twins-iiot/) to configue MQTT and Ditto service
* Eclipse Ditto [examples](https://github.com/eclipse-ditto/ditto-examples/tree/master/mqtt-bidirectional) with MQTT bidirectional communication, including [Arduino project](https://github.com/eclipse-ditto/ditto-examples/blob/master/mqtt-bidirectional/iot-device/octopus/src/main.ino)
* Eclipse Ditto MQTT documentation [page](https://eclipse.dev/ditto/1.5/connectivity-protocol-bindings-mqtt.html)

## Bug description and steps

When creating the ditto connection (it needs to be a completely new connection), establish the configuration description and make sure that it's receiving data. Then proceed to specify a new Incoming JavaScript Mapping funcion. Click update and test the messages. After troubleshooting some errors, a `content-type '<unspecified>'` not defined error appeared. No matter what i did (included in the message, changed header-mapping). It wouldn't work. Noticed that when defining the mapping function, it wouldn't place the mapping function correctly. The mapping would appear like this (which is according to ditto [documentation](https://eclipse.dev/ditto/connectivity-mapping.html)):

```json
"mappingDefinitions": {
    "javascript": {
        "mappingEngine": "JavaScript",
        "options": {
            "incomingScript": "function mapToDittoProtocolMsg(\n    headers, \n    textPayload, \n    bytePayload,\n    contentType\n) {\n\n    const jsonData = JSON.parse(textPayload || \"{}\"); // Handle empty payload\n    const thingId = jsonData.thingId.split(':');\n    const jsonValue = jsonData.value;\n\n    // Only handle authorized_supply messages\n    let channel = \"/features/authorize_supply\";\n    value = {\n        properties: {\n            timestamp: {\n                properties: { \n                    value: jsonValue.timestamp \n                }\n            },\n            authorization: { \n                properties: { \n                    value: jsonValue.authorization \n                } \n            },\n            msgType: { \n                properties: { \n                    value: jsonValue.msgType \n                } \n            }\n        } \n    };\n  \n    return Ditto.buildDittoProtocolMsg(\n        thingId[0],                 // thing namespace\n        thingId[1],                 // thing ID of the device\n        'things',                   // (group) we deal with a thing\n        'twin',                     // (channel) we want to update the twin\n        'commands',                 // (criterion) create a command to update the twin\n        'modify',                   // (action) modify the twin\n        channel,                    // (path) modify only the chosen feature\n        headers,                    // pass the mqtt headers\n        value\n    );\n}\n  ",
            "outgoingScript": ""
        }
    }
}
```

But, according to ditto [examples](https://github.com/eclipse-ditto/ditto-examples/tree/master/mqtt-bidirectional), the definition should look like this:

```json
"mappingContext": {
    "mappingEngine": "JavaScript",
    "options": {
        "incomingScript": "function mapToDittoProtocolMsg(\n    headers, \n    textPayload, \n    bytePayload,\n    contentType\n) {\n\n    const jsonData = JSON.parse(textPayload || \"{}\"); // Handle empty payload\n    const thingId = jsonData.thingId.split(':');\n    const jsonValue = jsonData.value;\n\n    // Only handle authorized_supply messages\n    let channel = \"/features/authorize_supply\";\n    value = {\n        properties: {\n            timestamp: {\n                properties: { \n                    value: jsonValue.timestamp \n                }\n            },\n            authorization: { \n                properties: { \n                    value: jsonValue.authorization \n                } \n            },\n            msgType: { \n                properties: { \n                    value: jsonValue.msgType \n                } \n            }\n        } \n    };\n  \n    return Ditto.buildDittoProtocolMsg(\n        thingId[0],                 // thing namespace\n        thingId[1],                 // thing ID of the device\n        'things',                   // (group) we deal with a thing\n        'twin',                     // (channel) we want to update the twin\n        'commands',                 // (criterion) create a command to update the twin\n        'modify',                   // (action) modify the twin\n        channel,                    // (path) modify only the chosen feature\n        headers,                    // pass the mqtt headers\n        value\n    );\n}\n  "
    }
}
```

This makes the payload mapping not appear in the web editor, but still work. However, uppon a system restart, the definition would appear in the other format and would work without issues.

### Note

Error:

```json
{
  "correlationId": "a7d35a2f-3d71-4cf0-95cb-428db16f4fa9",
  "timestamp": "2024-05-18T14:02:01.296616075Z",
  "category": "source",
  "type": "mapped",
  "level": "failure",
  "message": "Got exception <connectivity:message.mapping.failed> when processing external message with mapper <javascript>: The external message with content-type '<unspecified>' could not be mapped. - Message headers: [kafka.timestamp=1716040920247, kafka.topic=gas-pump_downlink, correlation-id=a7d35a2f-3d71-4cf0-95cb-428db16f4fa9] - Message payload: \"thingId\":\"org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20\",\"topic\":\"org.eclipse.ditto/9b0ec976-3012-42d8-b9ea-89d8b208ca20/things/twin/commands/create\",\"path\":\"/features/authorize_supply/properties/\",\"messageId\":\"{{ uuid() }}\",\"timestamp\":\"{{ timestamp }}\",\"source\":\"gas-pump\",\"method\":\"update\",\"target\":\"/features/authorize_supply\",\"value\":{\"msgType\":1,\"thingId\":\"org.eclipse.ditto:9b0ec976-3012-42d8-b9ea-89d8b208ca20\",\"topic\":\"org.eclipse.ditto/9b0ec976-3012-42d8-b9ea-89d8b208ca20/things/twin/commands/create\",\"path\":\"/features/authorize_supply/properties/\",\"authorization\":1,\"timestamp\":\"2024-05-18T12:03:44+0100\"}}",
  "address": "gas-pump_downlink"
}
```