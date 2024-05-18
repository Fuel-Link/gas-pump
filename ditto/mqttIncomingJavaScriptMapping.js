function mapToDittoProtocolMsg(
    headers, 
    textPayload, 
    bytePayload,
    contentType
) {

    const jsonData = JSON.parse(textPayload || "{}"); // Handle empty payload
    const thingId = jsonData.thingId.split(':');

    let value = {};
    let channel = "/features";
    
    if(jsonData.msgType === 0) { // Pump Init message
        value = {
            properties: {
                timestamp: {
                    properties: { 
                        value: jsonData.timestamp 
                    }
                },
                stock: { 
                    properties: { 
                        value: jsonData.stock 
                    } 
                },
                capacity: {
                    properties: {
                        value: jsonData.capacity
                    }
                },
                msgType: { 
                    properties: { 
                        value: jsonData.msgType 
                    } 
                }
            } 
        };
        channel = "/features/pump_init";
    } else if(jsonData.msgType === 2) { // Supply completed message
        value = { 
            properties: {
                timestamp: {
                    properties: { 
                        value: jsonData.timestamp 
                    }
                },
                amount: { 
                    properties: { 
                        value: jsonData.amount 
                    } 
                }, 
                stock: {
                    properties: {
                        value: jsonData.stock
                    }
                },
                msgType: { 
                    properties: { 
                        value: jsonData.msgType 
                    } 
                },
            } 
        };
        channel = "/features/supply_completed";
    } else if (jsonData.msgType === 3) {    // Fuel replenishment message
        value = { 
            properties: {
                timestamp: {
                    properties: { 
                        value: jsonData.timestamp 
                    }
                },
                amount: { 
                    properties: { 
                        value: jsonData.amount 
                    } 
                }, 
                stock: { 
                    properties: { 
                        value: jsonData.stock 
                    } 
                },
                msgType: { 
                    properties: { 
                        value: jsonData.msgType 
                    } 
                }
            } 
        };
        channel = "/features/fuel_replenishment";
    } else if (jsonData.msgType === 4) {    // Supply error message
        value = {
            properties: {
                timestamp: {
                    properties: { 
                        value: jsonData.timestamp 
                    }
                },
                error: { 
                    properties: { 
                        value: jsonData.error 
                    } 
                }, 
                msgType: { 
                    properties: { 
                        value: jsonData.msgType 
                    } 
                }
            } 
        };
        channel = "/features/supply_error";
    } else {    // error
        throw new Error('Invalid message type');
    }
  
    return Ditto.buildDittoProtocolMsg(
        thingId[0],                 // thing namespace
        thingId[1],                 // thing ID of the device
        'things',                   // (group) we deal with a thing
        'twin',                     // (channel) we want to update the twin
        'commands',                 // (criterion) create a command to update the twin
        'modify',                   // (action) modify the twin
        channel,                    // (path) modify only the chosen feature
        headers,                    // pass the mqtt headers
        value
    );
}
  