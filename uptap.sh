#!/bin/bash
dev=$1
ip=$2

ip link set $dev up
if [ -n "$ip" ]; then
    ip addr add $ip dev $dev

