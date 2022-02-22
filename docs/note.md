# Let's code a TCP/IP stack, 1: Ethernet & ARP

## TUN/TAP devices

### 创建TAP虚拟网卡

```c++
int tun_alloc(char *dev, int flags) {

    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
        perror("Opening /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);

    return fd;
}
/* 
 * IFF_NO_PI is crucial: Do not add protocol information header to packets.
 *                       otherwise we end up with unnecessary packet infor-
 *                       mation prepended to the Ethernet frame.
 */
int fd = tun_alloc(dev, IFF_TAP | IFF_NO_PI);
```

### 以太网帧

MTU: playload的最大长度，使用`ping 10.0.0.1 -s 1472`可以得到一个playload为1500的ip报文帧

```c++
/*
 *  ___________________________________
 * | dmac | smac | ethertype | payload |
 * |______|______|___________|_________|
 * | 6    | 6    | 2         | 0-1500  |
 * |______|______|___________|_________|
 */
struct eth_hdr
{
    u_int8_t  dmac[6];
    u_int8_t  smac[6];
    u_int16_t ethertype;
    u_int8_t  payload[];
} __attribute__((packed));
// __attribute__((packed));表示不会对结构体进行对齐。
```

- ethertype >= 1536: ethertype表示playload的类型
- ethertype <  1536: ethertype表示以太网帧的长度
- 下面的例子中，ethertype都大于等于1536：
  - IPv4:       0x0800
  - ARP:        0x0806
  - PPPoE:      0x8864
  - 802.1Q tag: 0x8100(VLAN)
  - IPV6:       0x86DD
  - MPLS Label: 0x8847

在ethertype后可能会跟着一个以太网帧的标签，这个标签可以是802.1Q tag(VLAN)，MPLS label等。**本项目暂时不实现这些标签**

## ARP（Address Resolution Protocol）协议

```c++
/*
 * 以太网arp/rarp报文格式
 *  __________________________________________________________________________________________________________________
 * | hwtype | protype | hwsize | protosize | opcode | sndr_hwaddr | sndr_protoaddr | target_hwaddr | target_protoaddr |
 * |________|_________|________|___________|________|_____________|________________|_______________|__________________|
 * | 1      | 1       | 1      | 1         | 1      | 0-6         | 0-4            | 0-6           | 0-4              |
 * |________|_________|________|___________|________|_____________|________________|_______________|__________________|
 */
struct arp_hdr
{
    u_int16_t hwtype;
    u_int16_t protype;
    u_int8_t hwsize;
    u_int8_t prosize;
    u_int16_t opcode;
    u_int8_t data[];
} __attribute__((packed)); 

struct arp_ipv4
{
    u_int8_t smac[6];
    u_int32_t sip;
    u_int8_t dmac[6];
    u_int32_t dip;
} __attribute__((packed));

```

- hwtype:  硬件地址类型，以太网是0x0001
- protype: 协议地址类型，IPv4是0x0800
- hwsize:  硬件地址长度，以太网是6
- prosize: 协议地址长度，IPv4是4
- opcode:  协议操作码，ARP请求是1，ARP响应是2 RARP请求是3，RARP响应是4

## Address Resolution Algorithm

```agm
?Do I have the hardware type in ar$hrd?
Yes: (almost definitely)
  [optionally check the hardware length ar$hln]
  ?Do I speak the protocol in ar$pro?
  Yes:
    [optionally check the protocol length ar$pln]
    Merge_flag := false
    If the pair <protocol type, sender protocol address> is
        already in my translation table, update the sender
        hardware address field of the entry with the new
        information in the packet and set Merge_flag to true.
    ?Am I the target protocol address?
    Yes:
      If Merge_flag is false, add the triplet <protocol type,
          sender protocol address, sender hardware address> to
          the translation table.
      ?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)
      Yes:
        Swap hardware and protocol fields, putting the local
            hardware and protocol addresses in the sender fields.
        Set the ar$op field to ares_op$REPLY
        Send the packet to the (new) target hardware address on
            the same hardware on which the request was received.
```

# Let's code a TCP/IP stack, 2: IPv4 & ICMPv4


## Internet Protocol version 4
```c++
/*
 * IPv4 header(unit: bits)
 *  _____________________________________________________________________________________________________________________________
 * | ihl | version | tos | length | id | flags | frag_offset | ttl | proto | csum | saddr | daddr | options | data               |
 * |_____|_________|_____|________|____|_______|_____________|_____|_______|______|_______|_______|_________|____________________|
 * | 4   | 4       | 8   | 16     | 16 | 16    | 16          | 8   | 8     | 16   | 32    | 32    | 40bytes | 65535-60bytes      |
 * |_____|_________|_____|________|____|_______|_____________|_____|_______|______|_______|_______|_________|____________________|
 *
 * 
 */

struct iphdr {
    u_int8_t ihl : 4;             // ip报文首部长度：最长15 * 32bit / 8bit = 60bytes 20bytes头部 + 20bytes选项
    u_int8_t version : 4;
    u_int8_t tos;
    u_int16_t len;
    u_int16_t id;
    u_int16_t flags : 3;
    u_int16_t frag_offset : 13;
    u_int8_t ttl;
    u_int8_t proto;
    u_int16_t csum;
    u_int32_t saddr;
    u_int32_t daddr;
} __attribute__((packed));
```

- version: ip版本，4bit，默认为4
- ihl: ip报文首部长度，4bit，默认为5：5 * 32bit / 8bit = 20bytes
- tos: 服务类型，8bit，默认为0
    - 111–Network Control（网络控制）
    - 110–Internetwork Control（网间控制）
    - 101–Critic（关键）
    - 100–Flash Override（疾速）
    - 011–Flash（闪速）
    - 010–Immediate（快速）
    - 001–Priority（优先）
    - 000–Routine（普通）
- len: ip报文长度，16bit，报文最长为：2^16 - 1 = 65535bytes(包含头部)
- id: ip分片后，用于重组报文的标识符，如果相等，则为同一个ip报文
- flags: 该字段长度为3比特位。它分为三分部分，保留位（reserved bit）为0；分片位（Don`t fragent）当为1时标识未分片，0则标识被分片；更多位（more fragments)为0标识最后分段，为1标识更多分段。
- frag_offset: 分片偏移量，16bit
- ttl: 生存时间，8bit
- proto: 协议类型
    - 1: ICMP
    - 6: TCP
    - 17: UDP
- csum: ip头部校验和，16bit
    ```c++
    // 假设某个IPv4数据包报头为：E3 4F 23 96 44 27 99 F3 [00 00]，注意，用中括号括起来的就是checksum
    //      checksum的初始值自动被设置为0
    //      然后，以16bit为单位，两两相加，对于该例子，即为：E34F + 2396 + 4427 + 99F3 = 1E4FF
    //      若计算结果大于0xFFFF，则将，高16位加到低16位上，对于该例子，即为，0xE4FF + 0x0001 = E500
    //      然后，将该值取反，即为~(E500)=1AFF

    uint16_t checksum(void *addr, int count)
    {
        register uint32_t sum = 0;
        uint16_t * ptr = addr;

        while( count > 1 )  {
            /*  This is the inner loop */
            sum += * ptr++;
            count -= 2;
        }

        /*  Add left-over byte, if any */
        if( count > 0 )
            sum += * (uint8_t *) ptr;

        /*  Fold 32-bit sum to 16 bits */
        while (sum>>16)
            sum = (sum & 0xffff) + (sum >> 16);

        return ~sum;
    }
    ```
- saddr: 源地址，32bit
- daddr: 目的地址，32bit

## Internet Control Message Protocol version 4

```c++
/*
 * ICMP header(unit: bits)
 *  _____________________________
 * |  type  | code  | checksum  |
 * |____ ___|_______|___________|
 * | 8bits  | 8bits | 16bits    |
 * |________|_______|___________|
 *
 */
struct icmp_v4 {
    u_int8_t type;
    u_int8_t code;
    u_int16_t csum;
    u_int8_t data[];
} __attribute__((packed));
```

- type: 8bit，报文类型(本项目中暂时只使用0、3、8)
    - 0: Echo Reply (ping reply)
    - 3: Destination Unreachable (ping unreachable)
    - 4: Source Quench
    - 5: Redirect
    - 8: Echo Request (ping request)
    - 11: Time Exceeded
    - 12: Parameter Problem
    - 13: Timestamp
    - 14: Timestamp Reply
    - 15: Information Request
    - 16: Information Reply
    - 17: Address Mask Request
    - 18: Address Mask Reply
    - 30: Traceroute
    - 31: Datagram Conversion Error
- code: type产生的具体原因。
- csum: icmp报文校验和（不仅仅是首部校验和），16bit
- data: 长度受到MTU的限制，最大为1500字节，即1500 - 20 - 8 = 1472字节。 

## icmp data

```c++
/*
 * ICMP Echo Reply
 *  ______________________________
 * |  id    | seq    | data...   |
 * |________|________|___________|
 * | 16bits | 16bits |           |
 * |________|________|___________|
 */
struct icmp_v4_echo_reply { 
    u_int16_t id;
    u_int16_t seq;
    u_int8_t data[];
} __attribute__((packed));
```

- id: 16bit，标识符，用于区分不同的ping请求, 可以设置为进程的pid。
- seq: 16bit，一个从零开始的数字，每当形成新的回显请求时就加一。这用于检测回显消息在传输过程中是否消失或重新排序。
- data: 长度受到MTU的限制，最大为1500字节，即1500 - 20 - 8 - 4 = 1468字节。 


```c++
/*
 * ICMP Destination Unreachable
 *  __________________________________
 * |  unused | len  | var   | data...|
 * |_________|______|_______|________|
 * | 8bits   | 8bits| 16bits|        |
 * |_________|______|_______|________|
 */

struct icmp_v4_dst_unreachable {
    u_int8_t unused;
    u_int8_t len;
    u_int16_t var;
    u_int8_t data[];
} __attribute__((packed));

```
- len: 原始数据报的长度，对于 IPv4，以32字节为一个单位。
- var: 可能是一个标识符，也可能是一个错误代码。

# Let's code a TCP/IP stack, 3: TCP Basics & Handshake

## Reliability mechanisms 可靠性机制

- 发送方需要等待接收方返回确认报文多久？
- 接收方处理报文的速度没有发送方发送报文的速度高怎么办？
- 网络间（如路由器）的报文处理速度没有发送方发送报文的速度高怎么办？

- 接收方响应的ack报文在传输中丢失了怎么办？

### 滑动窗口

### 拥塞控制


## TCP Header Format

```c++
/*
 * TCP header(unit: bits)
 *  _____________________________________________________________________________________________________________________________________________
 * |  sport    | dport    | seq   | ack_seq | rsvd | hl   | fin  | syn  | rst  | psh  | ack  | urg | ece | cwr | win   | csun  | urp  | options  |
 * |___________|__________|_______|_________|______|______|______|______|______|______|______|_____|_____|_____|_______|_______|______|__________|
 * | 16bits    | 16bits   | 32bits| 32bits  | 4bits| 4bits| 1bit | 1bit | 1bit | 1bit | 1bit | 1bit| 1bit| 1bit| 16bits| 16bits| 8bits| 0-40bytes|
 * |___________|__________|_______|_________|______|______|______|______|______|______|______|_____|_____|_____|_______|_______|______|__________|
 */
struct tcphdr {
    u_int16_t sport;
    u_int16_t dport;
    u_int32_t seq;
    u_int32_t ack_seq;
    u_int8_t rsvd : 4;
    u_int8_t hl : 4;
    u_int8_t fin : 1,
            syn : 1,
            rst : 1,
            psh : 1,
            ack : 1,
            urg : 1,
            ece : 1,
            cwr : 1;
    u_int16_t win;
    u_int16_t csum;
    u_int16_t urp;
    u_int8_t data[];
} __attribute__((packed));
```

- sport: 16bit，源端口号。
- dport: 16bit，目的端口号。
- seq: 32bit，序列号，每次发送报文时，序列号加一。
- ack_seq: 32bit，确认序列号，接收方收到报文时，确认序列号加一。
- rsvd: 4bit，保留字段，默认为0。
- hl: 4bit，首部长度。 以4字节为一个单位。
- fin: 1bit，结束标志。
- syn: 1bit，用于在初始握手中同步序列号。
- rst: 1bit，重置标志。
- psh: 1bit，指示接收方应尽快将数据“推送”到应用程序。
- ack: 1bit，确认标志。
- urg: 1bit，紧急标志。
- ece: 1bit，1：通知对方已将拥塞窗口缩小；
- cwr: 1bit，1：用于通知发送方降低其发送速率。
- win : 16bit，接收方窗口大小。
- csum: 16bit，校验和: 包含TCP首部和ip伪首部：

    ```c++
    /*
     * TCP Pseudo Header:
     *  ________________________________________________
     * |  src_addr | dst_addr | zero  | proto | tcp_len |
     * |___________|__________|_______|_______|_________|
     * | 32bits    | 32bits   | 1bit  | 8bits | 16bits  |
     * |___________|__________|_______|_______|_________|
     */
    ```
    - src_addr: 32bit，源地址。
    - dst_addr: 32bit，目的地址。
    - zero: 1bit，保留字段，填充0
    - proto: 8bit，协议类型，TCP为6, UDP为17。
    - tcp_len: 16bit，TCP报文长度（首部 + 数据）。
    
- urp: 16bit，紧急指针。
- data: 可变长度，用于存储数据。
