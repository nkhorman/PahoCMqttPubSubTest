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
./build_host/src/pubsub --url mqtt://host/topic
```

For publication, envoke as
```
./build_host/src/pubsub --url mqtt://host/topic --qos 1 --retain --value abc
```
