# Description: Run the MQTT5 Broker
cd mqtt5
docker compose up -d

cd ..

# Run Ditto service
cd ditto/docker
docker compose up -d