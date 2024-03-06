# Gas Pump Module

Module responsible for interacting directly with the Gas Pump. It receives instructions from the Composer to unlock the supply of a vehicle and provides the necessary metrics back.

# Topics

[Reference](https://www.hivemq.com/blog/mqtt-essentials-part-5-mqtt-topics-best-practices/) on best practices for building MQTT topics

```text
pump/{service.version}/{pump.id}/{event}
```

Where
* `service.version` is the version of Gas Pump module (Default **V1**)
* `pump.id` is the Identification of the Gas Pump, since there may exist multiple
* `event` relates to the possible actions with the Gas Pump module

Possible events:

| event      | interaction | description                                                                  |
|------------|-------------|------------------------------------------------------------------------------|
| `unlock`   | *subscribe* | Published by the *Composer* to order the Pump to allow the supply of fuel    |
| `supply`   | *publish*   | Published by the *Gas Pump* to signal a successfull supply of fuel           |

## Test topic

Although MQTT is connection-oriented (uses TCP), the clients aren't connected to each others, so they aren't directly informed if the a Gas Pump is up or not. Using the `SYS` channels can help, but that only informs if the module is up, not the individual Gas Pumps. With that idea in mind, the default topic of the Pump is periodically posted with a status message, to inform the connected modules if they can interact with the Gas Pump. The base topic is:

```text
pump/{service.version}/{pump.id}
```

Which is periodically posted with:

```json
{
    status: "up"
}
```

## Unlock Pump

Since the Gas pump is controlled by the *Composer* module and subsequently unlocked when conditions are met (Vehicle is identified and Employee authenticated), the `unlock` channel is **subcribed** by the *Gas Pump* module and listen for the publishes of `unlock` by the *Composer* module. After that, the pump is unlocked and the employee can fill the gas tank of the vehicle. The topic is as follows: 

```text
pump/{service.version}/{pump.id}/unlock
```

Which accepts messages of the following format:

```json
{
    command: "unlock"
}
```

## Supply

After the pump is unlocked and the vehicle filled up, the pump can be stopped by just placing the fuel hose in it's placed. At that step, the *Gas Pump* will **publish** a message indicating the successfull fuel of the vehicle.

```text
pump/{service.version}/{pump.id}/supply
```

Containing the messages in the following:

```json
{
    amount: double
    increment: integer
}
```




