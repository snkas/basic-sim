# General coding notes

Below is a collection of notes about how ns-3 functions, in particular how certain
concepts are modeled and implemented. In particular, this is to highlight how certain
aspects function in the basic-sim module, and how one would adapt these.

## TCP

Upon the creation of a Socket, one passes a Node object and a SocketFactory type identifier.
The Node object is passed because in the call chain, it asks whether anything aggregated to the Node
has this type ID to instantiate a SocketFactory. One is able to set a different socket factory
for each node. The Node must have the IP stack installed on it.

### Creating a socket

```
m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
```

Which is established in the following procedure:

* **Socket::CreateSocket(Ptr<Node> node, TypeId tid)**

  https://www.nsnam.org/doxygen/socket_8cc_source.html#l00071
  ```
  Ptr<SocketFactory> socketFactory = node->GetObject<SocketFactory> (tid);
  s = socketFactory->CreateSocket ();
  ```
  
* The node has several SocketFactory's associated to it, because of
  the installation of the Internet stack.
  
  1. In the constructor of the InternetStackHelper, it sets
     m_tcpFactory to TcpL4Protocol, which inherits from TcpSocketFactory.
  
     **InternetStackHelper::Initialize()**
     
     https://www.nsnam.org/doxygen/internet-stack-helper_8cc_source.html#l00120
     ```
     SetTcp ("ns3::TcpL4Protocol");
     ```
  2. Then it adds to the Node by aggregating an instantiation of the TcpL4Protocol:
  
     **InternetStackHelper::Install(Ptr<Node> node)**
     https://www.nsnam.org/doxygen/internet-stack-helper_8cc_source.html#l00292
     ```
     node->AggregateObject (m_tcpFactory.Create<Object> ());
     ```
     
  3. As such, the call of `socketFactory->CreateSocket ()` is sent to:
  
     **TcpL4Protocol::CreateSocket()**
    
     https://www.nsnam.org/doxygen/tcp-l4-protocol_8cc_source.html#l00216
    
     Which then actually creates the socket.
     
### Changing the TCP type

Because of the above model, the TcpL4Protocol attached to the Node is used to decide what
type of TCP socket is returned. As such, one must modify the TcpL4Protocol attached
to a node. As such, the following code can be used to adapt the TCP protocol:

```
Ptr<Node> node = ...; // E.g., GetNode() in an application
node->GetObject<TcpL4Protocol>()->SetAttribute("SocketType",  TypeIdValue(TcpVegas::GetTypeId ()));
node->GetObject<TcpL4Protocol>()->SetAttribute("RecoveryType",  TypeIdValue(TcpPrrRecovery::GetTypeId ()));
node->GetObject<TcpL4Protocol>()->SetAttribute("RttEstimatorType",  TypeIdValue(RttMeanDeviation::GetTypeId ()));
m_socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
```

The official documentation also has an equivalent, which uses Config::, which is more confusing in my opinion:

https://www.nsnam.org/docs/models/html/tcp.html
```
TypeId tid = TypeId::LookupByName ("ns3::TcpNewReno");
std::stringstream nodeId;
nodeId << n0n1.Get (0)->GetId ();
std::string specificNode = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketType";
Config::Set (specificNode, TypeIdValue (tid));
Ptr<Socket> localSocket = Socket::CreateSocket (n0n1.Get (0), TcpSocketFactory::GetTypeId ());
```

Of course, if you just want a default TCP on all nodes
(before installing the Internet stack on nodes, as it is a default, and once initialized
is set!), you can set it in the following two ways:

```
Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVegas::GetTypeId ()));
```
