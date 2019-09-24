## Raw Ethernet Packets
Uses basic C sockets to send raw Ethernet packages across a network (Mininet).

### Run:
You will need Mininet to run the program. 

#### Compile ```main.c``` file:
```gcc main.c```

#### Run Mininet:
```sudo mn```

Enter your ```sudo``` password when prompted. Mininet will automatically create two virtual hosts for you. We will use ```h1``` to send and ```h2``` to receive.
#### In Mininet:
Make sure you are in the directory of the ```a.out``` file generated from compiling ```main.c```.


Three commands are required to verify this program:
1. ```h2 ./a.out Recv h2-eth0 &```
   - Set ```h2``` to receive a message. The program will print its MAC to use with the next step.
      - You can optionally verify this MAC with ```h2 ifconfig``` and check the ```ether``` line.
2. ```h1 ./a.out Send h1-eth0 <MAC from previous step> "some message here"``` 
   - Send a message from ```h1``` to ```h2``` using the MAC given from the previous command. 
3. ```h2 bg``` 
    - Verify the correct message was sent. You will see your messaged echoed to the screen.


### Known Bugs
1. Sometimes, it takes three or four tries for ```h2 bg``` to print your message. If it does not, start again from the first step in the In Mininet section above.
