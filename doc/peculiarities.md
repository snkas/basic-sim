# Peculiarities

## TCP/UDP

- A TCP socket when initially Bind()'d to a particular source IP address
  will loose that IP address upon Connect().

- UDP sockets which did not get Bind()'d to a particular source IP address
  will not have a routing call with the full IP/UDP header, it will only 
  have a call with the IP destination.

- Converting an `Address` to `InetSocketAddress` and printing it:
  ```
  Address addr;
  InetSocketAddress transport(Ipv4Address("0.0.0.0"), 0);
  m_socket->GetSockName (addr);
  std::cout << addr << std::endl;
  transport = InetSocketAddress::ConvertFrom (addr);
  std::cout << transport.GetIpv4() << ":" << transport.GetPort() << std::endl;
  ```
  
