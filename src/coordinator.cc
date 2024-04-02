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

#include "coordinator.h"
#include "MyMessage_m.h"


Define_Module(Coordinator);



void Coordinator::initialize()
{

    //initialize be eny ashof men el sender we men el reciever
    std::ifstream inputFile("coordinator.txt");

    //////////////////////////////////////Read coordinator.txt//////////////////////////////////////
    // Check if the file is open
    if (!inputFile.is_open())
    {
        std::cout << "Error opening file!" << std::endl;
    }

    // Read the first integer from the file
    if (!(inputFile >> starting_node))
    {
        std::cout << "Error reading starting_node from file!" << std::endl;
    }


    // Read the second integer from the file
    if (!(inputFile >> starting_time))
    {
        std::cout << "Error reading starting_time from file!" << std::endl;
    }

    // Close the file
    inputFile.close();
    //////////////////////////////////////Done Read coordinator.txt//////////////////////////////////////

    //    simtime_t currentTime = simTime();
    //    std::cout <<"before incrementing "<< currentTime  <<endl;

    MyMessage_Base *msg = new MyMessage_Base(("node" + std::to_string(starting_node)).c_str());

    //scheduleAt(simTime()+starting_time,msg); //bab3t 3nd el start time 3shan abd2 ab3t el msg 3nd el time dah

    if(starting_node == 0) //bacheck anhy node ely hab3tlha el msg
    {
        sendDelayed( msg,starting_time,"out_node_0");
    }
    else
    {
        sendDelayed( msg,starting_time,"out_node_1");
    }

}

void Coordinator::handleMessage(cMessage *msg) //no handle msg
{

    //    if (msg->isSelfMessage())
    //    {
    //        MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
    //
    //        if(starting_node == 0) //bacheck anhy node ely hab3tlha el msg
    //        {
    //            send(msg,"out_node_0");//bab3t le node_0
    //        }
    //        else
    //        {
    //            send(msg,"out_node_1");//bab3t le node_1
    //        }
    //    }

}
