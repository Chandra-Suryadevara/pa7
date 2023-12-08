#include "../headers/Sender.h"
#include <iostream>
#include <math.h>
#include <vector>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <zlib.h>
#include <string>
#include <fstream>



int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: sender [-f data_file] host port" << std::endl;
        return 1;
    }
    
    std::string data_file;
    const char* host; // Initialize host and port pointers
    const char* port;
  
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f") {
            if (i + 1 < argc) {
                data_file = argv[i + 1];
                i++;
                
            }
            else {
                std::cerr << "Error: -f option requires a file name." << std::endl;
                return 1;
            }
        }
        else {
            if (!host)
                host = argv[i];
            else
                port = argv[i];
        }
    }
    std::cout << data_file;
        // Read file in binary mode
    std::ifstream file(data_file, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    char buffer[512];
    Sender sender(argv[3], argv[4], 2, 4); // Define win_size and Seqnum
    std::vector<char> payload;

    while (file.read(buffer, 512)) {
        payload.insert(payload.end(), buffer, buffer + 512);
        sender.make_packet(payload);
        payload.clear();
    }


    if (file.gcount() > 0) {
        payload.insert(payload.end(), buffer, buffer + file.gcount());
        sender.make_packet(payload);

    }
    
    file.close();

    return 0;
}



Sender::Sender(const char* destinationHost, const char* destinationPort, int win_size, int Seqnum)

{
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rval = getaddrinfo(destinationHost, destinationPort, &hints, &results)) != 0) {
        std::cerr << "Error getting the destination address: " << gai_strerror(rval) << std::endl;
        std::abort();
    }
    for (ptr = results; ptr != NULL; ptr = ptr->ai_next) {
        if ((sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) != -1) {
            break;
        }
    }
    if (ptr == NULL) {
        std::cerr << "Failed to open socket" << std::endl;
        std::abort();
    }
    setSeqSize(Seqnum);
    setWinSize(win_size);
    nextSeqNum = 0;
}


Packet Sender::DefineType(Packet buf, int type) {
    if ( type > 0 && type <= 3) {
        // Clear the existing value and set the lowest 2 bits of type
        buf.data[TP] &= 0x3f;
        buf.data[TP] |= (type << 6);

    }
    return buf;
}

Packet Sender::DefineTR(Packet buf, int TR) {
    if (TR == 0 || TR == 1) {
       
        buf.data[TP] &= 0xDF;
        buf.data[TP] |= (TR << 5) >> 2;
        TRL = TR;
        
    }
    return buf;
}

Packet Sender::setCRC1(Packet buf,unsigned int crc1) {
  
        for (int i = 8; i < 12; i++) {
            buf.data[i] &= 0x00;
        }
        buf.data[CRC] = (crc1 & 0xff000000) >> 24;
        buf.data[CRC + 1] = ((crc1 & 0x00ff0000) >> 16);
        buf.data[CRC + 2] = ((crc1 & 0x0000ff00) >> 8);
        buf.data[CRC + 3] = (crc1 & 0x000000ff);
        return buf;
}

Packet Sender::setCRC2(Packet buf, unsigned int crc2) {
    
    
    int CRC2 = 12 + data_size;
    for (int i = 524; i < 528; i++) {
        buf.data[i] &= 0x00;
    }
    buf.data[CRC2] = (crc2 & 0xff000000) >> 24;
    buf.data[CRC2 + 1] = ((crc2 & 0x00ff0000) >> 16);
    buf.data[CRC2 + 2] = ((crc2 & 0x0000ff00) >> 8);
    buf.data[CRC2 + 3] = (crc2 & 0x000000ff);
    return buf;
}


Packet Sender::AddWin(Packet buf) {


    buf, data[TP] & = 0xA95F60;
    buf.data[TP] |= (Winsize >> 3);
    return buf;
}

Packet Sender::AddSeq(Packet buf) {


    buf.data[TP + 1] &= 0;
    buf.data[TP + 1] |= nextSeqNum;
    return buf;
}

Packet Sender::AddLength(Packet buf,unsigned int length) {
        
        // Set the length bytes in the packet header
        buf.data[L] = (length >> 8);  // Set the MSB
        buf.data[L + 1] = (length & 255);  // Set the LSB
    
        return buf;
}

Packet Sender::AddTime(Packet buf, unsigned int time) {

    //yet to implement;
    return buf;
}

void Sender::make_packet(const std::vector<char>& payload)
{
    Packet temppacket;
    data_size= payload.size() * sizeof(char);
   for (int i = 0; i < maxsize; i++) {
    temppacket.data[i] = temppacket.data[i] & 0x00;
    }
    temppacket.isSent = false;
    temppacket.isAcknowledged = false;

    if (data_size < 513 && data_size > 0) {

        temppacket = DefineType(temppacket,1);
        temppacket = DefineTR(temppacket, 0);
        temppacket = AddWin(temppacket);
        temppacket = AddSeq(temppacket);
        temppacket = AddLength(temppacket, data_size);
        temppacket = AddTime(temppacket, timestep);

       

        for (int i = 0; i < data_size; i++) {
          temppacket.data[Pay + i] &= 0x00;

       }

        for (int i = 0; i < data_size; i++) {
            temppacket.data[Pay + i] = static_cast<uint8_t>(payload[i]);
        }
        if (TRL == 0) {
            unsigned long crc1;
            
            crc1 = crc32(0L, NULL, 0);
            crc1 = crc32(crc1, (reinterpret_cast<const Bytef*>(temppacket.data)),8 );
         //fake implementation to imitate crc's
            temppacket = setCRC1(temppacket, crc1);

            uint8_t sub_payload[data_size];


            for (int i = 0; i < data_size; i++) {
                sub_payload[i] = temppacket.data[12 + i];
            }
            unsigned long crc2;

            crc2 = crc32(0L, NULL, 0);
            crc2 = crc32(crc2, (reinterpret_cast<const Bytef*>(sub_payload)), data_size);


            temppacket = setCRC2(temppacket, crc2);
        }
    }



    all_packets.push_back(temppacket);

}

void Sender::Send_packet(Packet temppacket)
{

    if ((temppacket.data[TP] >> 6 & 0b11) == 1) {
        if (Window.size() < 4) {
            if (sendto(sock, reinterpret_cast<const char*>(temppacket.data), headersize + data_size, 0, ptr->ai_addr, ptr->ai_addrlen) == -1) {
                std::cerr << "Error sending message" << std::endl;
            }
            else {
                sentpackets.push(temppacket);
                std::cout << " " << sizeof(temppacket.data) << std::endl;
                temppacket.isSent = true;
                nextSeqNum++;
                timestep++;
                std::cout << "sent";
            }
        }
        else {
            std::cout << "waiting for acknowledgment" << std::endl;
        }
    }
}

void Sender::setWinSize(int n) {
    int maxWinSize = floor((getSeqSize() + 1) / 2);

    if (n <= maxWinSize) {
        Winsize = n;
    }
    else {

        throw std::invalid_argument("Incorrect Window size. Please enter a value less than or equal to half the sequence number in SR protocol.\n");
    }
}


int Sender::getWinSize() {

    return Winsize;

}



void Sender::Receive_packet() {
    Packet tempPacket;
    struct sockaddr senderAddr;
    uint8_t receiveBuffer[maxsize];
    socklen_t senderAddrLen = sizeof(senderAddr);

    ssize_t receivedpackets = recvfrom(sock, receiveBuffer, sizeof(receiveBuffer), 0, &senderAddr, &senderAddrLen);

    if (receivedpackets == -1) {
        std::cerr << "Error receiving response: " << strerror(errno) << std::endl;
    }
    else if (receivedpackets > 0) {
        
        std::cout << "Received: " << receivedpackets << " bytes" << std::endl;
        memcpy(tempPacket.data, receiveBuffer, receivedpackets);
    }
    else {
        std::cerr << "No data received" << std::endl;
    }
    close(sock);
    
    received_packets.push_back(tempPacket);
    if (((Window.front).data)[TP+1] == (tempPacket.data)[TP+1]) {
        std::cout << "Ack knowledgement found " << std::endl;
        recentAck = static_cast<int>(tempPaclet.data[TP + 1]);
        Window.pop();
        Send_packet(all_packets[nextindex]);
    }
   
    
}




bool Sender::available_space() {
    if (sentpackets.size() < Winsize) {
        return true;
    }
    else {
        return false;
    }
}

void Sender::inspect_packet(Packet temppacket){
    if ((temppacket.data[TP] >> 6 & 0b11) == 1 && available_space()) {
        Send_packet(temppacket);
    }
    else {
        
    }

}

bool Sender::acknowledge(int s)
{
    if (s < base || s >= base + getWinSize())
    {
        return false;
    }
    base = (s + 1) % getWinSize(); // Advance the base.
    recentAck = s;
    return true;
}



bool Sender::canAddNew()
{
    int maxSeqNum = (base + getWinSize() - 1) % getWinSize();
    return nextSeqNum != recentAck && nextSeqNum != maxSeqNum;
}


int Sender::addNew()
{
    if (canAddNew())
    {
        int newSeqNum = nextSeqNum;
        nextSeqNum = (nextSeqNum + 1) % getWinSize();
        return newSeqNum;
    }
    return -1;
}

void Sender::setSeqSize(int n) {


    bitSeqNum = n;

}

int Sender::getSeqSize() {

    return bitSeqNum;

}
/*

const char* Sender::getdesHost() {

    return destinationHostLocal; 

}

const char* Sender::getdesport() {

    return destinationPortLocal;

}
*/

Sender::~Sender() {
   
}


