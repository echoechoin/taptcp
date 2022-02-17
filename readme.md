# taptcp

learn TCP stack.

```txt
  ______________________________________________
 |              |                               |
 | linux kerenl | TUN/TAP virtual device module |
 |______________|____________|__________________|
 |              | TAP        |                  |
 |              |  fd = open("/dev/net/tun")    |  
 |              |  read (fd, data)              |
 |              |  write(fd, data)              |
 | user space   |           data                |
 |              |____________|__________________|
 |              | TCP/IP     |                  |
 |              |  link layer(arp)              |
 |              |  ip layer(icmp)               | 
 |              |  network layer(tcp/udp)       |
 |______________|_______________________________|
``` 