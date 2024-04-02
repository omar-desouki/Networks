//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __PROJECT_NODE_H_
#define __PROJECT_NODE_H_

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <bitset>
#include <vector>
#include <cmath> // Include the cmath library for std::round
#include <fstream>


//function btround to one decimal place
double roundToOneDecimal(double value)
{
    return std::round(value * 10.0) / 10.0;
}



std::bitset<8> calculateChecksum(const std::string& curr_msg) {
    // Initialise XOR with zero.
    std::bitset<8> checksum(0);

    // Concatenate the binary representation of characters to the correct message.
    // Apply the effect of the characters to the message XOR.
    for(int i = 0; i < curr_msg.size(); i++) {
        std::bitset<8> newChar(curr_msg[i]);
        checksum = checksum ^ newChar;
    }

    return checksum;
}



std::string byteStuffing(const std::string& curr_msg) {
    std::vector<char> msg_byte_stuffed_vect;

    // Flags of ‘#’ and the Esc is ‘/’
    msg_byte_stuffed_vect.push_back('#');

    for (int i = 0; i < curr_msg.size(); i++) {
        if (curr_msg[i] == '#' || curr_msg[i] == '/') {
            msg_byte_stuffed_vect.push_back('/');
            msg_byte_stuffed_vect.push_back(curr_msg[i]);
        } else {
            msg_byte_stuffed_vect.push_back(curr_msg[i]);
        }
    }

    msg_byte_stuffed_vect.push_back('#'); // hna m3ana vect feh el msg bs ma3molo byte stuffing

    std::string msg_byte_stuffed_str(msg_byte_stuffed_vect.begin(), msg_byte_stuffed_vect.end());
    return msg_byte_stuffed_str;
}







using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
public:
    std::string sender;
    //reciever
    int sequenceNumber = 0;
    int expectedSequenceNumber = 0;
    std::string message ="";
    std::string receivedMessage ="";
    std::ofstream outputFile;
    std::ifstream inputFile;
    double temp_time = 0;
    double pt_delay ;
    double tt_delay;
    double tot_delay;
    double error_delay;
    double dup_delay;
    //error vars
    int duplication =0; //var lw hyhsl duplication
    int delay =0; //var lw be 1 hyhsal delay
    int loss =0; //var lw be 1 yb2a lost we msh ha send


    //sender window initialising
    int sender_window_start =0;
    int sender_window_end = 2;

    int dup_index = 0; //3shan akon 3aref dah el duplicate rakm kam
    int ack_index=0;
    int msg_base_vect_index =0;
    int re_sended_frames_index=0;
    int lost_index=0;
    //index bahot feh akher message fen 3shan a3rf akhry we ana baslide el window
    int close_index =0;
    int timeout_frames_resend_index =0;





protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

};

#endif
