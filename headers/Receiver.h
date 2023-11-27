#ifndef RECEIVER_H_
#define RECEIVER_H_

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <netdb.h>

const int maxSize = 528;

class Receiver{

public:
Receiver(const char* destinationHost, const char* destinationPort, const char* IPTYPE);

/*Checks if the acknowledge one is the next seq number or last seq number */
bool acknowledge_check (int n);
void receive_data();
/*Checks if teh seq number passed is the consecutive number if true checks 
is buffer is empty or the buffer has previous  seq numbers or it adds the seq number to buffer*/
void data_received(int seq);
int getSeqSize();
/*concats buffer with recent_ack*/
int get_recent_ack();
bool save_packet(std::string file_name);
bool inspect_packet();
unsigned int calcrc1();
unsigned int extractCRC(uint8_t buf[]);
unsigned int extractCRC2(uint8_t buf[]);

unsigned int calcrc2();
/*setter function for recent ack*/
void set_recent_ack(int ack);
std::vector<int> get_ack();
unsigned int extractLength(uint8_t buf[]);
std::queue<std::vector<uint8_t>> get_data();
std::vector<uint8_t> extract_payload(uint8_t buf[]);
int getdatasize();
private:
const int L = 2;
const int T = 4;
const int TP = 0;
const int CRC = 8;
const int headersize = 16;
const int Pay = 12;
int sock, rval;
int datasize;
int nextseq = 0;
struct addrinfo hints, * results, * ptr;
int SeqnumSize;
uint8_t buffer[maxSize];
std::queue<std::vector<uint8_t>> data;
int recent_ack;
};

#endif