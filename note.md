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

```c++
/*
 * IPv4 header(unit: bits)
 *  _____________________________________________________________________________________________________________________________
 * | version | ihl | tos | length | id | flags | frag_offset | ttl | proto | csum | saddr | daddr | options | data               |
 * |_________|_____|_____|________|____|_______|_____________|_____|_______|______|_______|_______|_________|____________________|
 * | 4       | 4   | 8   | 16     | 16 | 16    | 16          | 8   | 8     | 16   | 32    | 32    | 40bytes | 65535-60bytes      |
 * |_________|_____|_____|________|____|_______|_____________|_____|_______|______|_______|_______|_________|____________________|
 *
 * 
 */

struct iphdr {
    u_int8_t version : 4;
    u_int8_t ihl : 4;             // ip报文首部长度：最长15 * 32bit / 8bit = 60bytes 20bytes头部 + 20bytes选项
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
- len: ip报文长度，16bit，报文最长为：2^16 - 1 = 65535bytes
- id: ip分片后，用于重组报文的标识符，16bit
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
