function mapToDittoProtocolMsg(
    headers, 
    textPayload, 
    bytePayload,
    contentType
) {

    const jsonData = JSON.parse(textPayload || "{}"); // Handle empty payload
    const thingId = jsonData.thingId.split(':');
    const jsonValue = jsonData.value;

    // Only handle authorized_supply messages
    let channel = "/features/authorize_supply";
    value = {
        properties: {
            timestamp: {
                properties: { 
                    value: jsonValue.timestamp 
                }
            },
            authorization: { 
                properties: { 
                    value: jsonValue.authorization 
                } 
            },
            msgType: { 
                properties: { 
                    value: jsonValue.msgType 
                } 
            }
        } 
    };
  
    return Ditto.buildDittoProtocolMsg(
        thingId[0],                 // thing namespace
        thingId[1],                 // thing ID of the device
        'things',                   // (group) we deal with a thing
        'twin',                     // (channel) we want to update the twin
        'commands',                 // (criterion) create a command to update the twin
        'modify',                   // (action) modify the twin
        channel,                    // (path) modify only the chosen feature
        headers,                    // pass the kafka headers
        value
    );
}
  