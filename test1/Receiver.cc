#include "../headers/Receiver.h"
#include <iostream>
#include <math.h>
#include <vector>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <zlib.h>
#include <string>
#include <queue>
#include <fstream>



int main(int argc, char* argv[]) {
    if (argc < 6) {
        std::cerr << "Usage: receiver [-f data_file] [-IPTYPE] host port" << std::endl;
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
        
    }



     std::ofstream of;

  // open file in binary mode
  of.open(data_file.c_str(), std::ios::binary | std::ios::out);
    if (of.fail()) {
        std::cout << "Cannot create file" << std::endl;
        return 1;
    }

     Receiver Receiver(argv[4], argv[5], argv[3]);
     Receiver.receive_data();
     if (Receiver.inspect_packet()){
     std::queue<std::vector<uint8_t>> data = Receiver.get_data();
    while (!data.empty()) {
        const auto& byteVector = data.front();
        of.write(reinterpret_cast<const char*>(byteVector.data()), byteVector.size());
        data.pop();
    }

    if (of.fail()) {
        std::cout << "Write failed" << std::endl;
        of.close();
        return 2;
    }else{
        std::cout<<"writing done";
    }
     }else{

        std::cerr<<"something is wrong with the packet";
     }
    of.close();
    return 0;
}


Receiver::Receiver(const char* destinationHost, const char* destinationPort, const char* IPTYPE){
    int ai_family = AF_UNSPEC; // Default to both IPv4 and IPv6
    if (strcmp(IPTYPE, "-4") == 0) {
        ai_family = AF_INET; // IPv4
    } else if (strcmp(IPTYPE, "-6") == 0) {
        ai_family = AF_INET6; // IPv6
    } else {
        std::cerr << "Invalid argument, please use -4 or -6" << std::endl;
        std::abort();
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rval = getaddrinfo(NULL, "0", &hints, &results)) != 0) {
        std::cerr << "Error getting address info: " << gai_strerror(rval) << std::endl;
        std:abort();
    }

    for (ptr = results; ptr != NULL; ptr = ptr->ai_next) {
        if ((sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) != -1) {
            if (bind(sock, ptr->ai_addr, ptr->ai_addrlen) != -1) {
                break;
            }
            close(sock);
        }
    }
     if (ptr == NULL) {
        std::cerr << "Failed to bind the socket" << std::endl;
        std::abort();
    }
}

int Receiver::getSeqSize() {
    return SeqnumSize;
}

std::queue<std::vector<uint8_t>> Receiver::get_data(){


    return data;
}

void Receiver::receive_data(){
    sockaddr localAddr;
    sockaddr_storage clientAddr;
    socklen_t localAddrLen = sizeof(localAddr);
    socklen_t clientAddrLen = sizeof(clientAddr);

    getsockname(sock, &localAddr, &localAddrLen);
    ssize_t receivedBytes = 0;
    if (localAddr.sa_family == AF_INET) {
        sockaddr_in* ipv4LocalAddr = (sockaddr_in*)&localAddr;
        std::cout << "Bound to port: " << ntohs(ipv4LocalAddr->sin_port) << std::endl;

         receivedBytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
    } else if (localAddr.sa_family == AF_INET6) {
        sockaddr_in6* ipv6LocalAddr = (sockaddr_in6*)&localAddr;
        std::cout << "Bound to port: " << ntohs(ipv6LocalAddr->sin6_port) << std::endl;
    receivedBytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
    }
if (receivedBytes > 0) {
        std::cout << "Received: " << std::endl;
    } else {
        std::cerr << "Error receiving data" << std::endl;
    }
}



unsigned int Receiver::calcrc1(){
     unsigned int crc1;
            
    crc1 = crc32(0L, NULL, 0);
    crc1 = crc32(crc1, (reinterpret_cast<const Bytef*>(buffer)),8 );
    return crc1;
}




std::vector<uint8_t> Receiver::extract_payload(uint8_t buf[]){
    std::vector<uint8_t> sub_payload(datasize); 
        for (int i = 0; i < datasize; i++) {
                sub_payload[i] = buf[12 + i];
            }
    return sub_payload;
}


unsigned int Receiver::calcrc2(){
      uint8_t sub_payload[datasize];
    for (int i = 0; i < datasize; i++) {
                sub_payload[i] = buffer[12 + i];
            }
            unsigned long crc2;

            crc2 = crc32(0L, NULL, 0);
            crc2 = crc32(crc2, (reinterpret_cast<const Bytef*>(sub_payload)), datasize);
    return crc2;
}

unsigned int Receiver::extractCRC(uint8_t buf[]) {
    unsigned int crc = 0;
    crc |= buf[CRC] << 24;
    crc |= buf[CRC + 1] << 16;
    crc |= buf[CRC + 2] << 8;
    crc |= buf[CRC + 3];
    return crc;
}

unsigned int Receiver::extractCRC2(uint8_t buf[]) {
    int CRC2 = 12 + datasize;
    unsigned int crc = 0;
    crc |= buf[CRC2] << 24;
    crc |= buf[CRC2 + 1] << 16;
    crc |= buf[CRC2 + 2] << 8;
    crc |= buf[CRC2 + 3];
    return crc;
}

unsigned int Receiver::extractLength(uint8_t buf[]) {
    unsigned int length = (buf[L] << 8) | buf[L + 1];
    return length;
}

int Receiver::getdatasize() {
    return datasize;   
}




bool Receiver::inspect_packet(){
    uint8_t y = nextseq;
    if((buffer[TP] >> 6) == 0x1){
        if(buffer[TP] & 0x20 == 0x0){
            if(buffer[TP+1]  == y){
                unsigned int local_crc1 = calcrc1();
                
                if(local_crc1 == extractCRC(buffer)){
                    datasize = extractLength(buffer);
                    unsigned int local_crc2 = calcrc2();
                    unsigned int given_crc2 = extractCRC2(buffer);
                    if(local_crc2 == given_crc2){
                         data.push(extract_payload(buffer));
                        return true ;
                    }else {
                        std::cerr<<"Not matching CRC's computed CRC2:"<<local_crc2<<"found CRC1:"<<given_crc2;
                    }
                }else {
                    std::cerr<<"Not matching CRC's computed CRC1:"<<local_crc1<<"found CRC1:"<<extractCRC(buffer);
            
                }
            } else{
                std::cerr << "Seq Num Not Right";
                return false;
            }    
        } else{
            std::cerr << "TR is not right";
            return false;
        }
    }else{
        std::cerr << "Type is differnt cannot get data";
        return false;
    }
}



/*


bool Receiver::acknowledge_check(int n){

if(n == recent_ack + 1 || getSeqSize() - (recent_ack + 1 ) == 0 ){
    return true;
} else {
    return false;
}
}

void Receiver::data_received() {
    if (acknowledge_check(seq)) {
        set_recent_ack(seq);

        // Check if the buffer is not empty
        if (!buffer.empty() && acknowledge_check(buffer.front())) {
            int front = buffer.front();
            buffer.erase(buffer.begin());
            data_received(front);
        }

    }
    else {
        buffer.push_back(seq);
    }
}


std::vector<int> Receiver::get_ack() {
    std::vector<int> ack = buffer;
    ack.insert(ack.end(), recent_ack);
    return ack;
}


int Receiver::get_recent_ack() {
    return recent_ack;
}

void Receiver::set_recent_ack(int ack) {
    recent_ack = ack;
}

*/

