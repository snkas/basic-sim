# How ToS works for sockets in ns-3


## Step-by-step

**Step 1.** Set the IP TOS field in the Socket. 
For stream sockets, the least significant two bits
are nullified as they are reserved for ECN.

```
// tcp-socket.cc
void
Socket::SetIpTos (uint8_t tos)
{
  Address address;
  GetSockName (address);
  if (GetSocketType () == NS3_SOCK_STREAM)
    {
      // preserve the least two significant bits of the current TOS
      // value, which are used for ECN
      tos &= 0xfc;
      tos |= m_ipTos & 0x3;
    }
  m_ipTos = tos;
  m_priority = IpTos2Priority (tos);
}
```

... and there is a mapping of IP TOS to the priority
(which will eventually be used in its priority tag):

```
// tcp-socket.cc
uint8_t
Socket::IpTos2Priority (uint8_t ipTos)
{
  uint8_t prio = NS3_PRIO_BESTEFFORT;
  ipTos &= 0x1e;
  switch (ipTos >> 1)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      prio = NS3_PRIO_BESTEFFORT;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      prio = NS3_PRIO_BULK;
      break;
    case 8:
    case 9:
    case 10:
    case 11:
      prio = NS3_PRIO_INTERACTIVE;
      break;
    case 12:
    case 13:
    case 14:
    case 15:
      prio = NS3_PRIO_INTERACTIVE_BULK;
      break;
    }
  return prio;
}
```

... together this results in the following procedure to determine SocketPriorityTag (priority).
It uses the IP TOS field to map to a band.
The band it is assigned to is based on bits 3-6 (with 0 the most significant) of the IP TOS field.

```
Bits 3-6    Priority                pfifo_fast band
0 to 3	    0 (Best Effort)         1
4 to 7	    2 (Bulk)                2
8 to 11	    6 (Interactive)         0
12 to 15    4 (Interactive Bulk)    1
```
(sources: https://www.nsnam.org/docs/models/html/sockets-api.html 
and https://www.nsnam.org/docs/models/html/pfifo-fast.html)

As a concrete example, let's say we set TOS to 55:
```
55 = 0b00110111

... the least significant 2 bits are set to zero for ECN (for a stream socket):
0b00110100

... of this, take bits 3-6, which is: 

0b00110100
     ^^^^

... resulting in:
0b00010100

... and then shift one to the right (to get in range 0-15):
0b00001010

... which equals: 10
... which (from the table) corresponds to band: 0 (highest priority)
```

**Step 2.** The TCP socket adds tags to a packet it wants to send out. These tags
are because the IP layer underneath forms the IP header -- as such, the TCP
socket somehow has to signal to it what it wants to set for those fields.
The tags are: IpTosTag, IpTtlTag, SocketPriorityTag.

```
// tcp-socket-base.cc
void TcpSocketBase::AddSocketTags (const Ptr<Packet> &p) const
```

**Step 3.** The Ipv4L3Protocol removes the IpTosTag and IpTtlTag and
set the corresponding values in the IP header it creates.
The SocketPriorityTag however stays with the packet, as it is used by
the queueing discipline later down the line.

```
// ipv4-l3-protocol.cc
void  Ipv4L3Protocol::Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
```

**Step 4** The packet is given to the traffic-control layer (which might use the tag).
Just before passing it to the network device (either via dequeue, or if there
is no queue, giving it directly), it will remove the SocketPriorityTag.
The network device, which sends it out.

```
// traffic-control-layer.cc
void TrafficControlLayer::Send (Ptr<NetDevice> device, Ptr<QueueDiscItem> item) {
  // ...
  item->AddHeader ();
  // a single queue device makes no use of the priority tag
  if (!devQueueIface || devQueueIface->GetNTxQueues () == 1)
    {
      SocketPriorityTag priorityTag;
      item->GetPacket ()->RemovePacketTag (priorityTag);
    }
  device->Send (item->GetPacket (), item->GetAddress (), item->GetProtocol ());
  // ...
}

// queue-disc.cc
bool QueueDisc::Transmit (Ptr<QueueDiscItem> item) {
  // ...
  if (!m_devQueueIface || m_devQueueIface->GetNTxQueues () == 1)
    {
      SocketPriorityTag priorityTag;
      item->GetPacket ()->RemovePacketTag (priorityTag);
    }
  NS_ASSERT_MSG (m_send, "Send callback not set");
  m_send (item);
}
```

**Step 5.** The next node's Ipv4L3Protocol receives the packet, it removes any
SocketPriorityTag still lingering, and creates a new one based on the
IP TOS value in the header. It might give it down to an application, or step 4 happens
to the next hop.

```
// ipv4-l3-protocol.cc
void Ipv4L3Protocol::IpForward (Ptr<Ipv4Route> rtentry, Ptr<const Packet> p, const Ipv4Header &header)
```

## Code for implementing your own queueing discipline

Get header TOS from a queue disc item by casting it to IPv4
(note: you should check header->GetProtocol beforehand):

```
Ipv4Header header = ((Ipv4QueueDiscItem*) (PeekPointer (item)))->GetHeader();
std::cout << "IP header TOS: " << (int) header.GetTos() << std::endl;
```
