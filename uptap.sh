#!/bin/bash
dev=$1
ip=$2

ip link set $dev up
if [[  $ip != "" ]]; then
    ip addr add $ip dev $dev
fi


