# Gas Pump Module

Module responsible for interacting directly with the Gas Pump. It receives instructions from the Composer to unlock the supply of a vehicle and provides the necessary metrics back.

Functionalities:
* 

# Topics

[Reference](https://www.hivemq.com/blog/mqtt-essentials-part-5-mqtt-topics-best-practices/) on best practices for building MQTT topics

## Base

Default base topic. Our service includes a version so that future developments can have parallel operation with previous versions. Maybe only on-connect

* This channel is supposed to be **published** by the module

```text
pump/{service.version}
```

* Returns

```json
{
    status: "up"
}
```

### Topic structure

```text
pump/{service.version}/{pump.id}/{event}
```

Where
* `service.version` is the version of Gas Pump module (Default **V1**)
* `pump.id` is the Identification of the Gas Pump, since there may exist multiple
* `event` relates to the possible actions with the Gas Pump module

Possible events:

| event      | interaction | description                                                               |
|------------|-------------|---------------------------------------------------------------------------|
| `register` | *subscribe* | Published by the *Composer* to register a supply action by the pump       |
| `unlock`   | *subscribe* | Published by the *Composer* to order the Pump to allow the supply of fuel |
| `supply`   | *publish*   | Published by the *Gas Pump* to signal a successfull supply of fuel        |
