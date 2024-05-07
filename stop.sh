# Description: Run the MQTT5 Broker
cd mqtt5
docker compose down

cd ..

# Run Ditto service
cd ditto/docker
docker compose down