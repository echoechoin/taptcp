#!/bin/bash

dev=$1
ip=$2

ip link set $dev up
ip addr add $ip dev $dev

