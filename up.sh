#!/usr/bin/env bash

a1="$1"
host="${a1:=h183}"
./build.sh arm && tar -C build_arm/src -cvzf - pubsub | ssh root@${host} "tar -xvzf -"
