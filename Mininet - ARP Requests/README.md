## Raw Ethernet Packets
Simple program that can send ARP request and process ARP replies. Run/validated in Mininet.

### Run:
You will need Mininet to run the program. 

#### Compile:
```sh mk```

#### Run Mininet:
```sudo mn```

Enter your ```sudo``` password when prompted. Mininet will automatically create two virtual hosts for you. 

#### In Mininet:
Make sure you are in the directory of the executable file generated from compiling.


Two commands are required to verify this program:
1. ```h2 wireshark &```
   - Open wireshark and observe h2.
2. ```h1 ./455_proj2 h1-eth0 10.0.0.2``` 
   - Send ARP request using the interface name (h1-eth0) and destination IP (10.0.0.2). 

Observe wireshark activity when the second command is run.
