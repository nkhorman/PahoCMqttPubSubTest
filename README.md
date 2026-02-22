This is a prototype MQTT C++11 client using the [Eclipse-Paho MQTT C library](https://github.com/eclipse-paho/paho.mqtt.c) and LibreSSL for Nest.


---
### Compiling
It compiles for Nest using
```
./build.sh arm
```
or native host (Ubuntu 22 or MacOS) using
```
./build.sh host
```
---
### Upload to Nest
```
./up.sh [hostname | ip address]
```
This will first automatically envoke
```
./build.sh arm
```
---
### Usage
For subscription, envoke as
```
./build_host/src/pubsub --url mqtt://broker.hostname/topic
```

For publication, envoke as
```
./build_host/src/pubsub --url mqtt://broker.hostname/topic --qos 1 --retain --value abc
```
You should be able to use both mqtt://... and mqtts://...

For username password authentication;
```
mqtt://username:password@broker.hostname/topic
```

For SSL validation of the broker;
```
mqtts://broker.hostname/topic --serverchain /some/path/chain.pem
```

For SSL brokers with public certs;
```
mqtts://public.broker.hostname/topic --capath /some/path/to/ca/repository
```

For SSL brokers requiring client certs;
```
mqtts://broker.hostname/topic --clientcert /some/path/to/client.cert.key.pem
```
