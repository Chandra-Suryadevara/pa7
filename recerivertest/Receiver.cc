#include "Receiver.h"
#include <iostream>
#include <math.h>
#include <vector>
#include <thread>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <zlib.h>
#include <string>
#include <queue>
#include <fstream>
#include <fcntl.h> 
#include <unistd.h>




unsigned int Receiver::calcrc1() {
    unsigned int crc1;

    crc1 = crc32(0L, NULL, 0);
    crc1 = crc32(crc1, (reinterpret_cast<const Bytef*>(buffer)), 8);
    return crc1;
}




std::vector<uint8_t> Receiver::extract_payload(uint8_t buf[]) {
    std::vector<uint8_t> sub_payload(datasize);
    for (int i = 0; i < datasize; i++) {
        sub_payload[i] = buf[12 + i];
    }
    return sub_payload;
}


unsigned int Receiver::calcrc2() {
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

void Receiver::set_crc1(unsigned int CRC1) {

    crc1 = CRC1;

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

int Receiver::getWinSize() {
    return Winsize;
}

void Receiver::set_winsize(int win) {
    Winsize = win;
}
std::queue<std::vector<uint8_t>> Receiver::get_data() {


    return data;
}


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

     Receiver receiver(argv[4], argv[5], argv[3]);
     std::thread receiving_thread(&Receiver::receive_data, &receiver);
     std::thread saving_thread(&Receiver::save_data_to_file, &receiver, data_file);
     
     receiving_thread.join();
     saving_thread.join();
    return 0;
}
void Receiver::save_data_to_file(const std::string& data_file) {
    // Open the file for writing
    std::ofstream of(data_file.c_str(), std::ios::binary | std::ios::out);
    if (of.fail()) {
        std::cout << "Cannot create file" << std::endl;
        return;
    }

    while (!done_receiving) {
        
            std::unique_lock<std::mutex> lock(mtx); // Correct placement

            // Wait until data is available for saving
            cv.wait(lock, [this] { return !data.empty() || done_receiving; });

            // Save data to file if available
            while (!data.empty()) {
                const auto& byteVector = data.front();
                of.write(reinterpret_cast<const char*>(byteVector.data()), byteVector.size());
                data.pop();
            }
        

        // Check if done receiving
        if (done_receiving) {
            break;
        }
    }
    if (done_receiving) {
        // Close the file outside the loop
        of.close();

        if (of.fail()) {
            std::cout << "Write failed" << std::endl;
        }
        else {
            std::cout << "Writing done" << std::endl;
        }
    }
}




void Receiver::Send_packet(uint8_t buf[]) {

    if ((buf[TP] >> 6 & 0b11) == 1) {
        if ((buf[TP] & 0x20) == 1) {

            if (sendto(sock, reinterpret_cast<const char*>(buf), headersize + datasize, 0, ptr->ai_addr, ptr->ai_addrlen) == -1) {
                std::cerr << "Error sending stripped paylaod  packet" << std::endl;
            }
            else {

                std::cout << "sent a stripped of packet for seq NUm " << static_cast<int>(buf[TP + 1]) << std::endl;
            }
        }
    }
}

void Receiver::Send_ack_packet(const std::array<uint8_t, maxSize>& buf) {
    if ((buf[TP] >> 6 & 0b11) == 2) {
        if (sendto(sock, reinterpret_cast<const char*>(buf.data()), maxSize, 0, ptr->ai_addr, ptr->ai_addrlen) == -1) {
            std::cerr << "Error sending negative ack packet" << std::endl;
        }
        else {
            std::cout << "sent an Ack packet for seq Num " << static_cast<int>(buf[TP + 1]) << std::endl;
        }
    }
}


bool Receiver::is_done() {

    return done_receiving;

}




void Receiver::Send_neg_ack_packet(const std::array<uint8_t, maxSize>& buf) {
    if ((buf[TP] >> 6 & 0b11) == 3) {
        if (sendto(sock, reinterpret_cast<const char*>(buf.data()), maxSize, 0, ptr->ai_addr, ptr->ai_addrlen) == -1) {
            std::cerr << "Error sending negative ack packet" << std::endl;
        }
        else {
            std::cout << "sent a negative Ack packet for seq Num " << static_cast<int>(buf[TP + 1]) << std::endl;
        }
    }
}


std::array<std::uint8_t, maxSize> Receiver::make_packet(unsigned int type, unsigned int TR, unsigned int Seqnum)
{
    uint8_t buf[maxSize];
    for (int i = 0; i < maxSize; i++) {
        buf[i] = buf[i] & 0x00;
    }
    if (type > 0 && type <= 3) {
        // Clear the existing value and set the lowest 2 bits of type
        buf[TP] &= 0x3f;
        buf[TP] |= (type << 6);

    }
    if (TR == 0 || TR == 1) {

        buf[TP] &= 0xDF;
        buf[TP] |= (TR << 5) >> 2;

    }


        buf[TP + 1] &= 0;
        buf[TP + 1] |= Seqnum;

        for (int i = 0; i < datasize; i++) {
            buf[Pay + i] &= 0x00;

        }
        std::array<uint8_t, maxSize> buf1;
        for (int i = 0; i < maxSize; i++) {
            buf1[i] = buf[i] & 0x00;
        }

        return buf1;
      
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
    done_receiving = false;
        
    if ((rval = getaddrinfo(destinationHost, destinationPort, &hints, &results)) != 0) {
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



void Receiver::receive_data(){
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 10; // 15-second timeout
    timeout.tv_usec = 0;
    while (!done_receiving) {
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
        }
        else if (localAddr.sa_family == AF_INET6) {
            sockaddr_in6* ipv6LocalAddr = (sockaddr_in6*)&localAddr;
            std::cout << "Bound to port: " << ntohs(ipv6LocalAddr->sin6_port) << std::endl;
            receivedBytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        }
     
        if (receivedBytes == -1) {
            std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
        }
        else if (receivedBytes == 0) {
            std::cout << "No data received" << std::endl;
        }
        else {
            inspect_packet();
            std::cout << "Packet received. Resetting timer." << std::endl;
            done_receiving = true;
        }
        
        int activity = select(sock + 1, &read_fds, nullptr, nullptr, &timeout);

        if (activity == -1) {
            std::cerr << "Error in select()" << std::endl;
            std::abort();
        }
        else if (activity == 0) {
            std::cout << "Timeout occurred. No data received within 60 seconds." << std::endl;
            done_receiving = true;
            break;
        }
        
    }


    }
        




bool Receiver::inspect_packet(){
    uint8_t y = nextseq;
    if((buffer[TP] >> 6) == 0x1){
        if((buffer[TP] & 0x20) == 0x0){
            if(acknowledge_check(buffer[TP+1]) && data.empty()){
                unsigned int local_crc1 = calcrc1();
                if(local_crc1 == extractCRC(buffer)){
                    datasize = extractLength(buffer);
                    unsigned int local_crc2 = calcrc2();
                    unsigned int given_crc2 = extractCRC2(buffer);
                    if(local_crc2 == given_crc2){
                        if (sizeof(data) == 0) {
                            set_winsize(buffer[TP] & 0x2B67);
                            set_crc1(local_crc1);
                        }
                         data.push(extract_payload(buffer));
                         nextseq++;
                         recent_ack = buffer[TP + 1];
                         Send_ack_packet(make_packet(2, 0, nextseq - 1));
                         cv.notify_one();
                         std::cout << "Sent a acknoledgement for " << nextseq - 1 << " packet"<<std::endl;
                        return true ;
                    }else {
                        std::cerr<<"Not matching CRC's computed CRC2:"<<local_crc2<<"found CRC1:"<<given_crc2;
                        return false;
                    }
                }else {
                    std::cerr<<"Not matching CRC's computed CRC1:"<<local_crc1<<"found CRC1:"<<extractCRC(buffer);
                    return false;
                }
            }
            else if (acknowledge_check(buffer[TP + 1])) {
                unsigned int local_crc1 = calcrc1();
                if ( local_crc1 == extractCRC(buffer)) {
                    datasize = extractLength(buffer);
                    unsigned int local_crc2 = calcrc2();
                    unsigned int given_crc2 = extractCRC2(buffer);
                    if (local_crc2 == given_crc2) {
                        data.push(extract_payload(buffer));
                        recent_ack = buffer[TP + 1];
                        Send_ack_packet(make_packet(2, 0, nextseq - 1));
                        std::cout << "Sent a acknoledgement for " << nextseq - 1 << "packet" << std::endl;
                        cv.notify_one();
                        return true;
                    }
                    else {
                        std::cerr << "Not matching CRC's computed CRC2:" << local_crc2 << "found CRC1:" << given_crc2;
                        return false;
                    }
                }
                else {
                    std::cerr << "Not matching CRC's computed CRC1 which was received with first packet:" << local_crc1 << "found CRC1:" << extractCRC(buffer);
                    return false;
                }
            
            }else {
                std::cerr << "Seq Num Not Right";
                return false;
            }    
        }
        else if ((buffer[TP] & 0x20) == 0x1) { // send a negative acknowledgment
            Send_neg_ack_packet(make_packet(3, 0, nextseq));
            std::cout << "Sent a begative acknoledgement for " << nextseq << "packet" << std::endl;
            return false;

        }
        else {
            std::cerr << "TR is wrong right";
            return false;
        }
    }
    else if ((buffer[TP] >> 6) == 0x2) {  //ack pacekt

        std::cout << "Receieved a acknowledge pacekt"<<std::endl;
        return false;

    }
    else if ((buffer[TP] >> 6) == 0x3) {  //negative ack pacekt

        std::cout << "Receieved a negative acknowledge pacekt" << std::endl;
        return false;

    }else { // if type is other
        std::cerr << "Type is differnt cannot get data";
        return false;
    }
}


bool Receiver::acknowledge_check(int n){// do from here see the logic and recent ack declaration

if(n == recent_ack + 1 || getWinSize() - (recent_ack + 1 ) == 0 ){
    return true;
} else {
    return false;
}
}
