## Eclipse Ditto :: kubernetes 

This folder contains example yaml files which can be used to start Eclipse Ditto 
with its backing Database - MongoDB - and a reverse proxy - nginx - in front of the HTTP and WebSocket API.

[Source](https://github.com/eclipse-ditto/ditto/blob/master/deployment/kubernetes/README.md)

## Requirements

* [k3s](https://rancher.com/docs/k3s/latest/en/)
* Port 30080 needs to be available on the node

## Install k3s

Run the following command to install k3s. 
```bash
curl -sfL https://get.k3s.io | sh -
```

Change the owner of `/etc/rancher/k3s/k3s.yaml` to your group and user.
```bash
sudo chown <groupId>:<userId> /etc/rancher/k3s/k3s.yaml
```

Copy k3s kube config to local .kube directory as `config` file (backup existing kubectl `config` if you have one).
```bash
cp /etc/rancher/k3s/k3s.yaml .kube/config
```

Verify that the `kubectl` command works:
```bash
kubectl get nodes
```

The command should print something like this:
```bash
kubectl get nodes
NAME              STATUS   ROLES                  AGE     VERSION
<your-hostname>   Ready    control-plane,master   5h21m   v1.20.2+k3s1
```

The container logs can be found here: `/var/log/containers/...`

## Running Eclipse Ditto on kubernetes

### Configure nginx
The nginx's configuration is located in the `nginx.conf` file and contains a "Basic authentication"
for accessing the HTTP and WebSocket API. The users for this sample authentication are configured
in the `nginx.httpasswd` file also located in this directory.

In order to add a new entry to this file, use the "openssl passwd" tool to create a hashed password:
```bash
openssl passwd -quiet
 Password: <enter password>
 Verifying - Password: <enter password>
```

Append the printed hash in the `nginx.httpasswd` file placing the username who shall receive this
password in front of Ditto like this:
```
ditto:A6BgmB8IEtPTs
```

#### Create configuration mappings for nginx
```bash
kubectl create configmap -n egs-fuellink ditto-nginx-conf --from-file=nginx/nginx.conf
kubectl create configmap -n egs-fuellink ditto-nginx-cors --from-file=nginx/nginx-cors.conf
kubectl create configmap -n egs-fuellink ditto-nginx-htpasswd --from-file=nginx/nginx.htpasswd
kubectl create configmap -n egs-fuellink ditto-nginx-index --from-file=nginx/index.html
kubectl create configmap -n egs-fuellink swagger-ui-api --from-file=nginx/openapi
```

#### Delete config maps

```bash
kubectl delete configmap -n egs-fuellink nginx-conf
kubectl delete configmap -n egs-fuellink nginx-cors
kubectl delete configmap -n egs-fuellink nginx-htpasswd
kubectl delete configmap -n egs-fuellink nginx-index
kubectl create configmap -n egs-fuellink swagger-ui-api
```
#### MongoDB
There are two ways starting a MongoDB instance.
Either use a simple MongoDB container without persistence.
```bash
kubectl apply -f mongodb/mongodb.yaml
```

Or use the stateful MongoDB set with a local persistent volume.
Before running the following commands be sure that the `/data/db/` directory is existing.
```bash
kubectl apply -f mongodb-statefulset/storage-class.yaml
envsubst < mongodb-statefulset/persistent-volume.yaml | kubectl apply -f -
kubectl apply -f mongodb-statefulset/mongodb-statefulset.yaml
```

##### Dedicated MongoDB
In case you already have a MongoDB in the cloud or elsewhere it is possible to connect Ditto to this MongoDB. 
This can be done by setting the MongoDB URI via env variable "MONGO_DB_URI" in the 
`deployment/kubernetes/deploymentFiles/ditto/ditto-cluster.yml` for all services except the `gateway`.
Other MongoDB settings can be set via env variables and are documented in
[Operating Ditto](https://www.eclipse.dev/ditto/installation-operating.html) section.

In case your "MONGO_DB_URI" contains sensitive information like username and password it is recommended to use
a kubernetes secret. 
To create a kubernetes secret use the following command;
```bash
kubectl create secret generic mongodb --from-literal=mongodb-uri='<mongodb_uri>' 
```

In order to use the kubernetes secret replace the variable "MONGO_DB_HOSTNAME" with the following lines:
```yaml
  - name: MONGO_DB_URI    
    valueFrom:
      secretKeyRef:
        name: mongodb
        key: mongodb-uri
```

### Start Eclipse Ditto
Ditto uses the `latest` tag for its images. If you want to use a different version replace the `latest` tag in
`deployment/kubernetes/deploymentFiles/ditto/ditto-cluster.yml` with the version you want to use.

If _DITTO_LOGGING_FILE_APPENDER_ is set to 'true' then the following step have to be done.
In order to be able to access ditto log files run the following command to initialize the hostPath.
```bash
kubectl apply -f ditto/ditto-log-files.yaml
```

Start Ditto with the predefined version or another of choice.
```bash
kubectl apply -f ditto/
# Start ditto services with an alternative version e.g. 0-SNAPSHOT
# cat deployment/kubernetes/deploymentFiles/ditto/ditto-cluster.yaml | sed s/latest/0-SNAPSHOT/ | kubectl apply -f -
```

#### Start Swagger UI
```bash
kubectl apply -f swagger/swagger.yaml
```

#### Start Reverse Proxy (nginx)
```bash
kubectl apply -f nginx/nginx.yaml
```

#### Verify all pods are running
Run the following command to verify that everything is up & running.

```bash
kubectl get pods
NAME                             READY   STATUS    RESTARTS   AGE
mongodb-0                        1/1     Running   0          5m
policies-5d6798cc6-dzklx         1/1     Running   0          3m
gateway-d9f9cbb65-4fsbk          1/1     Running   0          3m
things-search-768c894bd4-v4n2z   1/1     Running   0          3m
things-5787ffdf7f-mn2cs          1/1     Running   0          3m
connectivity-54b9799b8f-496f5    1/1     Running   0          3m
swagger-b8asd6f857-651bg         1/1     Running   0          2m
nginx-7bdb84f965-gf2lp           1/1     Running   0          1m
```


Have Fun!
