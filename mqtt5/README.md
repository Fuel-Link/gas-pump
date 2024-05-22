# How to setup Mosquitto MQTT Broker using docker 
These instructions will work on any Debian based OS including Ubuntu, RaspberryPi, WSL2 etc...  
(For non-Debian distros, commands for installation need to be tweaked)  
_By default the config allows only to use local connections for security reasons but since authentication is enabled below, that's not the case._

## 1. Install docker

Latest instructions are [here](https://docs.docker.com/engine/install/ubuntu/) on docker website.  
You can also use this script - [install-docker.sh](/install-docker.sh)

## 2. Create base folder for mqtt configuration

```bash

mkdir mqtt5
cd mqtt5

# for storing mosquitto.conf and pwfile (for password)
mkdir config

```

## 3. Create Mosquitto config file - mosquitto.conf
```bash
nano config/mosquitto.conf
```

Basic configuration file content below including websocket config
```
allow_anonymous true
listener 1883
listener 9001
protocol websockets
persistence true
password_file /mosquitto/config/pwfile
persistence_file mosquitto.db
persistence_location /mosquitto/data/
```

## 4. Create Mosquitto password file - pwfile

```bash
touch config/pwfile
```

## 5. Create docker-compose file called 'docker-compose.yml'

```yml

version: "3.7"
services:
  # mqtt5 eclipse-mosquitto
  mqtt5:
    image: eclipse-mosquitto
    container_name: mqtt5
    ports:
      - "1883:1883" #default mqtt port
      - "9001:9001" #default mqtt port for websockets
    volumes:
      - ./config:/mosquitto/config:rw
      - ./data:/mosquitto/data:rw
      - ./log:/mosquitto/log:rw
    restart: unless-stopped

# volumes for mapping data,config and log
volumes:
  config:
  data:
  log:

networks:
  default:
    name: mqtt5-network

```

## 6. Create and run docker container for MQTT

```bash
# In case you don't have docker-compose you can install it
sudo apt install docker-compose

# Run the docker container for mqtt
sudo docker-compose -p mqtt5 up -d

```

### Check if the container is up and working (note down container-id)

```bash

sudo docker ps

```

# Optional

## 7. Create a user/password in the pwfile

```bash

# login interactively into the mqtt container
sudo docker exec -it <container-id> sh

# add user and it will prompt for password
mosquitto_passwd -c /mosquitto/config/pwfile user1

# delete user command format
mosquitto_passwd -D /mosquitto/config/pwfile <user-name-to-delete>

# type 'exit' to exit out of docker container prompt

```
Then restart the container 
```bash
sudo docker restart <container-id>
```

## 8. Time to test !!!

### Install mosquitto client tools for testing
```bash

sudo apt install mosquitto-clients

```

### Let us start Subscriber now - topic name => 'hello/topic'

```bash

# Without authentication
mosquitto_sub -v -t 'hello/topic'

# With authentication
mosquitto_sub -v -t 'hello/topic' -u user1 -P <password>

# Alternate way in url format
# Format => mqtt(s)://[username[:password]@]host[:port]/topic
mosquitto_sub -v -L mqtt://user1:abc123@localhost/test/topic

```

### Let us start Publising to that topic

```bash

# Without authentication
mosquitto_pub -t 'hello/topic' -m 'hello MQTT'

# With authentication
mosquitto_pub -t 'hello/topic' -m 'hello MQTT' -u user1 -P <password>

# Alternate way in url format 
# Format => mqtt(s)://[username[:password]@]host[:port]/topic
mosquitto_pub -L mqtt://user1:abc123@localhost/test/topic -m 'hello MQTT'

```
## You can find C/C++ code for mosquitto client
Check [main.cpp](main.cpp) for the mosquitto client code.

## You can also install a nice MQTT Web Client
Read more about it here => https://mqttx.app/  

```bash
sudo docker run -d --name mqttx-web -p 80:80 emqx/mqttx-web
```

## Source/Reference for Mosquitto
Github => https://github.com/eclipse/mosquitto

##
![Static Badge](https://img.shields.io/badge/SPONSORING-red?style=for-the-badge)    
If you use my projects or like them, consider sponsoring me. Anything helps and encourages me to keep going.  
See details here: https://github.com/sponsors/sukesh-ak  

Your sponsorship would help me not only maintain the projects I'm involved in, but also support my other community activities and purchase hardware for testing these libraries. If you're an individual user who has enjoyed my projects or benefited from my community work, please consider donating as a sign of appreciation. If you run a business that uses my work in your products, sponsoring my development makes good business sense: it ensures that the projects your product relies on stay healthy and actively maintained.

Thank you for considering supporting my work!

# Deployment

Convert the `docker-compose-prod.yml` to k3s deployment files using kompose, with the command:

```bash
kompose convert --file docker-compose-prod.yml
```

Add the namespace to all the generated deployment files:

```yaml
metadata:
  ...
  namespace: egs-fuellink
...
```

## Volumes

Execute the deployment creation of the volumes, using the command:

```bash
kubectl apply -f gas-pump-mqtt5-claim0-persistentvolumeclaim.yaml
kubectl apply -f gas-pump-mqtt5-claim1-persistentvolumeclaim.yaml
kubectl apply -f gas-pump-mqtt5-claim2-persistentvolumeclaim.yaml
kubectl get pvc -n egs-fuellink
```

Add the mqtt configuration files to the volumes:

```bash
kubectl create configmap mosquitto-config --from-file=/tmp/mosquitto.conf -n egs-fuellink
kubectl create configmap pwfile --from-file=pwfile -n egs-fuellink
```

Better to boot up with a slim distribution, like busybox, configure the volumes since they are persistent and then deploy again with mosquitto

## Network

Change the network policy API version to:

```yaml
apiVersion: networking.k8s.io/v1
```

Execute the following command to create the shared `gas-pump.network` network:

```bash
kubectl apply -f gas-pump.network-networkpolicy.yaml
```

## Deploy

Deploy the Service:

```bash
kubectl apply -f gas-pump-mqtt5-service.yaml
```

Execute the deployment:

```bash
kubectl apply -f gas-pump-mqtt5-deployment.yaml
```

List the deployed services:

```bash
kubectl get service --namespace egs-fuellink
kubectl get pods --namespace egs-fuellink
kubectl logs gas-pump-mqtt5-7bdb488b84-2v49r -n egs-fuellink
```

