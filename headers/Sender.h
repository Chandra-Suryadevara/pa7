#ifndef SENDER_H
#define SENDER_H

#include <iostream>
#include <thread>
#include <netdb.h>
#include <vector>
const int maxSize = 528;

struct Packet {
    uint8_t data[maxSize];
    bool isSent;
    bool isAcknowledged;
};

class Sender {

public:
      Sender(const char* destinationHost, const char* destinationPort, int win_size, int Seqnum);
      ~Sender();
      void setWinSize(int n);
      Packet DefineType(Packet buf, int type);
      Packet DefineTR(Packet buf, int TR);
      Packet setCRC1(Packet buf, unsigned int crc1);
      Packet setCRC2(Packet buf, unsigned int crc2);
      Packet AddWin(Packet buf);
      Packet AddSeq(Packet buf);
      Packet AddLength(Packet buf, unsigned int length);
      Packet AddTime(Packet buf, unsigned int time);
      void make_packet(const std::vector<char>& payload);
      void Send_packet(Packet temppacket);
      void Receive_packet();
      bool available_space(); // Fixed the function name, should be available_space()
      void inspect_packet(Packet temppacket);
      bool acknowledge(int s);
      bool canAddNew();
      void RecievePacket(); // Fixed the function name, should be ReceivePacket()
      int addNew();
      void setSeqSize(int n);
      int getSeqSize();
      const char* getdesHost();
      const char* getdesport();
      int getWinSize();


private:
int recentAck;
std::vector<Packet> sentpackets;
std::vector<Packet> all_packets;
std::vector<Packet> received_packets;
int size;
int nextSeqNum;
int base;
int sock, rval;
int timestep = 0;
int TRL;
int Winsize;
int bitSeqNum;
int data_size;
struct addrinfo hints, * results, * ptr;
const int L = 2;
const int T = 4;
const int TP = 0;
const int CRC = 8;
const int headersize = 16;
const int maxsize = 528;
const int Pay = 12;
int space;
};


#endif