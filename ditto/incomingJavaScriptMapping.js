function mapToDittoProtocolMsg(
    headers, 
    textPayload, 
    bytePayload,
    contentType
) {

    const jsonData = JSON.parse(textPayload || "{}"); // Handle empty payload
    const thingId = jsonData.thingId.split(':');
    const timestamp = jsonData.timestamp;
    const imageId = jsonData.imageId.toString(); // Ensure imageId is a string
    const url = jsonData.url;

    const value = { 
        imageCaptured: {
            properties: {
                timestamp: {  
                    value: jsonData.timestamp 
                },
                imageId: { 
                    properties: { 
                        value: jsonData.imageId 
                    } 
                }, 
                url: { 
                    properties: { 
                        value: jsonData.url 
                    } 
                },
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
        '/features',                // (path) modify all features at once
        headers,                    // pass the mqtt headers
        value
    );
}
  