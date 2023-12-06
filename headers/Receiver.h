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

int getWinSize();

/*concats buffer with recent_ack*/
int get_recent_ack();
bool save_packet(std::string file_name);
bool inspect_packet();
unsigned int calcrc1();
unsigned int extractCRC(uint8_t buf[]);
unsigned int extractCRC2(uint8_t buf[]);
void Send_neg_ack_packet(uint8_t buf[]);
void Send_ack_packet(uint8_t buf[]);
std::array<std::uint8_t, maxSize> make_packet(unsigned int Type, unsigned int TR, unsigned int Seqnum);
unsigned int calcrc2();
/*setter function for recent ack*/
void set_winsize(int win);
void set_crc1(unsigned int CRC1);
unsigned int extractLength(uint8_t buf[]);
std::queue<std::vector<uint8_t>> get_data();
std::vector<uint8_t> extract_payload(uint8_t buf[]);
int getdatasize();
private:
std::mutex mtx;  // Mutex for synchronization
std::condition_variable cv;  // Condition variable for signaling
std::atomic<bool> done_receiving{ false };
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
int Winsize;
unsigned int crc1;
uint8_t buffer[maxSize];
std::queue<std::vector<uint8_t>>sentpackets;
std::queue<std::vector<uint8_t>> data; 
int recent_ack = 04;
};

#endif