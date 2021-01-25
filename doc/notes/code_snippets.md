# Code snippets

#### Retrieve the IP and port of a socket
- Fetch the raw `Address`
- Convert the `Address` to `InetSocketAddress` and print it

```
Address addr;
m_socket->GetSockName (addr);
InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom (addr);
std::cout << inetSocketAddress.GetIpv4() << ":" << inetSocketAddress.GetPort() << std::endl;
```
