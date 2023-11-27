Team Members: Angadpal Singh  |  Chandra Shekar Suryadevara | Prottasha D'Cruze







RECIEVER

Open a new terminal window use the following commmands:


-To run the program in IPv4: ./receiver -f data-received.txt 127.0.0.1 64341


-To run the program in IPv6: ./receiver -f data-received.txt ::1 64341



SENDER 

Open a new terminal window use the following commmands: 

To compile the program:  g++ Sender.cc -o sender -lz

-To run the program in IPv4: ./sender -f short.pcap 127.0.0.1 64341

-To run the program in IPv6: ./sender -f short.pcap ::1 64341

 