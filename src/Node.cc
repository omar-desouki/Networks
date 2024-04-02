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

#include "Node.h"
#include "MyMessage_m.h"
Define_Module(Node);

//delays -> ythato fe el ini file ahsan

//var 3shan ashal el code lma ba3mel senddelayed

//sender vars
std::vector<int> sender_ack(100000,0); //array bastore feh el ack we el nack ely gatly
std::vector<int> sender_alreadysent(100000,0); //array bastore feh lw ana already ba3t msg 3shan mab3thash marten



std::vector<int>lost_before(100000,0);
std::vector<MyMessage_Base*>timeout_frames_resend; //timeout_vector
std::vector<MyMessage_Base*>timeout_frames; //timeout_vector
std::vector<MyMessage_Base*>re_sended_frames; //added da
std::vector<MyMessage_Base*>ack_base_vect; //added da
std::vector< std::string > msg_str_vect;
std::vector< std::string > flags_str_vect;
std::vector<MyMessage_Base*> msg_base_vect;
std::vector<MyMessage_Base*> msg_base_vect_dup; //dah 3shan lw hasal duplicate
std::vector<int>ack_array(100000,0);
std::vector<int>ack_array_sender(100000,0);



void Node::initialize() //no initialiser
{
    outputFile.open("output.txt",std::ios::out | std::ios::app);

    // Check if the file is open
    if (!outputFile.is_open())
    {
        std::cerr << "Error opening output file!" << std::endl;
    }
        pt_delay= getParentModule()->par("pt_delay");
        tt_delay= getParentModule()->par("tt_delay");
        tot_delay= getParentModule()->par("tot_delay");
        error_delay= getParentModule()->par("error_delay");
        dup_delay= getParentModule()->par("dup_delay");
}

void Node::handleMessage(cMessage *msg)
{
    MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);

    /******************************************************
     *        node receiving from coordinator             *
     ******************************************************/

    if (strcmp(getName() , msg->getName()) == 0 )
    {
        sender = msg->getName();

        //////////////////////////////////////Read the input01.txt file//////////////////////////////////////
        if(strcmp(getName() , "node0") == 0)
        {
            inputFile.open("input0.txt");
        }
        else
        {
            inputFile.open("input1.txt");
        }
        // Check if the file is open
        if (!inputFile.is_open())
        {
            std::cout << "Error opening file!" << std::endl;
        }

        int index =0;
        std::string message;

        std::string flags;

        while (inputFile >> flags)
        {
            flags_str_vect.push_back(flags);

            // Read the rest of the line into a string
            std::getline(inputFile >> std::ws, message);

            msg_str_vect.push_back(message);

            index++;
        }
        inputFile.close();

        close_index = msg_str_vect.size();

        //////////////////////////////////////Done Read the input01.txt file//////////////////////////////////////

        for(int loop_index=sender_window_start; loop_index <= sender_window_end ;loop_index++)
        {


            //ha create my message object
            MyMessage_Base *mmsg = new MyMessage_Base("channel");
            mmsg->setM_Type(0); //0 -> Sender
            mmsg->setSeq_Num(sequenceNumber);
            msg_base_vect.push_back(mmsg->dup());

            std::string curr_msg = msg_str_vect[loop_index];
            std::string curr_flag = flags_str_vect[loop_index];

            //////////////////////////////////////check-sum//////////////////////////////////////

            std::bitset<8> checksum = calculateChecksum(curr_msg);
            msg_base_vect[msg_base_vect_index]->setMycheckbits(checksum);

            //////////////////////////////////////Done check-sum///////////////////////////////////

            ////////////////////////////////////error-flags////////////////////////////////////

            //Modification[0] - loss[1] - duplicate[2] - delay[3]

            if(curr_flag[1] == '1')//loss
            {
                lost_before[loop_index] = 1;
                //don't send the frame
                loss = 1;
            }

            if(curr_flag[0] == '1') //modification
            {
                int index = int(uniform (0, curr_msg.size()));
                curr_msg[index] = curr_msg[index] + 2;

            }

            if(curr_flag[2] == '1') //duplication
            {
                duplication = 1;
            }

            if(curr_flag[3] == '1') //delay
            {
                delay = 1 ;
            }

            ////////////////////////////////////Done error-flags////////////////////////////////////

            /////////////////////////////////////byte-stufing/////////////////////////////////////
            std:: string msg_byte_stuffed_str = byteStuffing(curr_msg);
            msg_base_vect[msg_base_vect_index]->setM_Payload(msg_byte_stuffed_str.c_str());

            /////////////////////////////////////Done byte-stufing/////////////////////////////////////

            std:: string sentmsg = msg_base_vect[msg_base_vect_index] -> getM_Payload();
            if(loss == 0 )//if loss flag is 1, we will not send
            {
                simtime_t curr = simTime();
                sender_alreadysent[loop_index] = 1; //To know whether frame was sent before or not


                if(delay == 1)
                {
                    if(duplication == 1) //delay + duplication
                    {
                        temp_time = tt_delay+error_delay+pt_delay*(loop_index+1);
                        temp_time = roundToOneDecimal(temp_time);
                        sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");

                        msg_base_vect_dup.push_back( msg_base_vect[msg_base_vect_index]->dup());

                        //send duplicate
                        temp_time += dup_delay;
                        temp_time = roundToOneDecimal(temp_time);
                        sendDelayed( msg_base_vect_dup[dup_index],temp_time,"node_out");


                        dup_index++; //increment to consider next duplicate

                    }
                    else //delay only
                    {
                        temp_time = tt_delay + error_delay+pt_delay*(loop_index+1);
                        temp_time = roundToOneDecimal(temp_time);
                        sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");

                    }
                }
                else //no delay
                {
                    if(duplication == 1)//duplication only
                    {
                        temp_time =tt_delay+pt_delay*(loop_index+1);
                        temp_time = roundToOneDecimal(temp_time);
                        sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");

                        msg_base_vect_dup.push_back( msg_base_vect[msg_base_vect_index]->dup());

                        //send duplicate
                        temp_time +=dup_delay;
                        temp_time = roundToOneDecimal(temp_time);
                        sendDelayed( msg_base_vect_dup[dup_index],temp_time,"node_out");

                        dup_index++; //increment to consider next duplicate
                    }
                    else //no delay, no duplication
                    {
                        temp_time = tt_delay+pt_delay*(loop_index+1);
                        temp_time = roundToOneDecimal(temp_time);
                        sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");
                    }
                }
            }

            outputFile << "At time "<<simTime().dbl()+pt_delay*(loop_index)<<" "<<getName()<<", Introducing channel error with code " << flags_str_vect[loop_index]<<" and message = "<<msg_str_vect[loop_index]<<endl;

            timeout_frames.push_back(msg_base_vect[msg_base_vect_index] -> dup());
            scheduleAt(simTime()+10.5, timeout_frames[msg_base_vect_index]);
            std::cout << "Flags " <<curr_flag<< endl;
            outputFile << "At time " << simTime().dbl() + pt_delay * (loop_index + 1)
                                               << " Sender " << getName()
                                               << " Sent Frame with Seq_Num = " << msg_base_vect[msg_base_vect_index]->getSeq_Num()
                                               << " And Payload = " << msg_base_vect[msg_base_vect_index]->getM_Payload()
                                               << " and trailer = " << msg_base_vect[msg_base_vect_index]->getMycheckbits()
                                               << " and Modified " << curr_flag[0]
                                                                                << " ,Lost " << (loss == 1 ? "Yes" : "No")
                                                                                << " ,Duplicate " << duplication
                                                                                << " ,Delay " << delay << endl;
            //resetting to zero just in case
            loss =0;
            duplication =0;
            delay = 0;
            msg_base_vect_index++;
            sequenceNumber++;



        }

    }
    else //node receiving from a node
    {

        /******************************************************
         *   sender receiving either from receiver or self    *
         ******************************************************/

        if(strcmp(getName() ,sender.c_str()) ==0)
        {
            MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);

            /////////////////////////////////////Timer Handling at sender///////////////////////////////////

            if(msg->isSelfMessage())
            {
                outputFile << "Time out event at time " << simTime().dbl()
                                                                           << " for " << getName()
                                                                           << " on frame with seq_number " << mmsg->getSeq_Num() << endl;
                int seq_to_resend = mmsg->getSeq_Num();

                //create my message object
                MyMessage_Base *mmsg = new MyMessage_Base("channel");
                mmsg->setM_Type(0); // Sender -> 0
                mmsg->setSeq_Num(seq_to_resend);

                timeout_frames_resend.push_back(mmsg->dup());

                std::string curr_msg = msg_str_vect[seq_to_resend];

                //////////////////////////////////////check-sum//////////////////////////////////////
                std::bitset<8> checksum = calculateChecksum(curr_msg);
                timeout_frames_resend[timeout_frames_resend_index]->setMycheckbits(checksum);
                //////////////////////////////////////Done check-sum///////////////////////////////////


                /////////////////////////////////////byte-stufing/////////////////////////////////////
                std:: string msg_byte_stuffed_str = byteStuffing(curr_msg);
                timeout_frames_resend[timeout_frames_resend_index]->setM_Payload(msg_byte_stuffed_str.c_str());

                /////////////////////////////////////Done byte-stufing/////////////////////////////////////

                std:: string sentmsg = timeout_frames_resend[timeout_frames_resend_index] -> getM_Payload();
                outputFile << "At time " << simTime().dbl() + pt_delay
                        << " Sender " << getName()
                        << " Sent Frame with Seq_Num = " << timeout_frames_resend[timeout_frames_resend_index]->getSeq_Num()
                        << " And Payload = " << timeout_frames_resend[timeout_frames_resend_index]->getM_Payload()
                        << " and trailer = " << timeout_frames_resend[timeout_frames_resend_index]->getMycheckbits()
                        << endl;

                temp_time = tot_delay;
                temp_time = roundToOneDecimal(temp_time);
                sendDelayed( timeout_frames_resend[timeout_frames_resend_index],temp_time,"node_out");

                //Resetting to zero
                loss =0;
                duplication =0;
                delay = 0;
                timeout_frames_resend_index++;

            }

            /////////////////////////////////////Done timer using scheduleAt at sender///////////////////////////////////


            /******************************************************
             *        Sender receiving from receiver              *
             ******************************************************/

            /////////////////////////////////////Sender handling ACKs and NACKs///////////////////////////////////
            else //Sender receiving from receiver
            {
                outputFile << "At time " << simTime().dbl()
                                                   << " Sender " << getName()
                                                   << " Received  = " << mmsg->getM_Payload()
                                                   << " with number " << mmsg->getSeq_Num()
                                                   << endl;
                if (strcmp(mmsg->getM_Payload(), "ACK") == 0)
                {
                    if(mmsg->getSeq_Num()==close_index-1)
                    {
                        outputFile << "At time "<<simTime().dbl()<<" End of Simulation " <<endl;
                        endSimulation();

                    }

                    //cancel timeout event
                    //coutaya
                    std::cout << "cancel time out for frame : " << mmsg->getSeq_Num() << "  at time : " << simTime().dbl() <<endl;
                    cancelEvent(timeout_frames[mmsg->getSeq_Num()]);

                    sender_ack[mmsg->getSeq_Num()] = 1;

                    int window_start = 0;
                    int index =0;
                    //Determining how far to slide the window
                    while(sender_ack[index] == 1)
                    {
                        window_start++;
                        index++;
                    }
                    sender_window_start = window_start;
                    sender_window_end = sender_window_start+2;

                    //Making sure window doesn't slide more than permitted
                    if(sender_window_end >= close_index)
                    {
                        sender_window_end = close_index-1;
                    }
                    //Check for unsent frames in new window

                    for(int loop_index=sender_window_start; loop_index <= sender_window_end ;loop_index++)
                    {
                        //Handled Below if received NACK
                        if(sender_ack[loop_index] == 2)
                        {
                            continue;
                        }
                        //Skip sending if sent before, if NACK is received -> retransmit
                        else if(sender_alreadysent[loop_index] == 1)
                        {
                            continue;
                        }
                        else if(sender_ack[loop_index] == 1)
                        {
                            continue;
                        }
                        //If frame is sent and lost, do not send again
                        else if(lost_before[loop_index] == 1)
                        {
                            continue;
                        }

                        sequenceNumber = loop_index;

                        MyMessage_Base *mmsg = new MyMessage_Base("channel");
                        mmsg->setM_Type(0);
                        mmsg->setSeq_Num(sequenceNumber);

                        msg_base_vect.push_back(mmsg->dup());

                        std::string curr_msg = msg_str_vect[loop_index];
                        std::string curr_flag = flags_str_vect[loop_index];


                        //////////////////////////////////////check-sum//////////////////////////////////////
                        std::bitset<8> checksum = calculateChecksum(curr_msg);
                        msg_base_vect[msg_base_vect_index]->setMycheckbits(checksum);
                        //////////////////////////////////////Done check-sum///////////////////////////////////

                        ////////////////////////////////////error-flags////////////////////////////////////

                        //Modification[0] - loss[1] - duplicate[2] - delay[3]

                        if(curr_flag[1] == '1')//loss
                        {
                            lost_before[loop_index] = 1;
                            //don't send the frame
                            loss = 1;
                        }

                        if(curr_flag[0] == '1') //modification
                        {
                            int index = int(uniform (0, message.size()));

                            curr_msg[index] = curr_msg[index] + 2;

                        }

                        if(curr_flag[2] == '1') //duplication
                        {
                            duplication = 1;
                        }

                        if(curr_flag[3] == '1') //delay
                        {
                            delay = 1 ;
                        }

                        ////////////////////////////////////Done error-flags////////////////////////////////////

                        /////////////////////////////////////byte-stufing/////////////////////////////////////
                        std:: string msg_byte_stuffed_str = byteStuffing(curr_msg);

                        //mmsg->setM_Payload(msg_byte_stuffed_str.c_str());
                        msg_base_vect[msg_base_vect_index]->setM_Payload(msg_byte_stuffed_str.c_str());

                        /////////////////////////////////////Done byte-stufing/////////////////////////////////////

                        std:: string sentmsg = msg_base_vect[msg_base_vect_index] -> getM_Payload();
                        if(loss == 0 ) // Sender after Sliding window
                        {
                            simtime_t curr = simTime();
                            sender_alreadysent[loop_index] = 1;



                            if(delay == 1)
                            {
                                if(duplication == 1)
                                {
                                    temp_time = tt_delay+error_delay+pt_delay;
                                    temp_time = roundToOneDecimal(temp_time);
                                    sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");

                                    msg_base_vect_dup.push_back( msg_base_vect[msg_base_vect_index]->dup());

                                    temp_time += dup_delay;
                                    temp_time = roundToOneDecimal(temp_time);
                                    sendDelayed( msg_base_vect_dup[dup_index],temp_time,"node_out");


                                    dup_index++;

                                }
                                else
                                {
                                    temp_time = tt_delay + error_delay+pt_delay;
                                    temp_time = roundToOneDecimal(temp_time);
                                    sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");

                                }
                            }
                            else
                            {
                                if(duplication == 1)
                                {
                                    temp_time =tt_delay+pt_delay;
                                    temp_time = roundToOneDecimal(temp_time);
                                    sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");

                                    msg_base_vect_dup.push_back( msg_base_vect[msg_base_vect_index]->dup());

                                    temp_time +=dup_delay;
                                    temp_time = roundToOneDecimal(temp_time);
                                    sendDelayed( msg_base_vect_dup[dup_index],temp_time,"node_out");

                                    dup_index++;
                                }
                                else //no delay, no duplication
                                {
                                    temp_time = tt_delay+pt_delay;
                                    temp_time = roundToOneDecimal(temp_time);
                                    sendDelayed( msg_base_vect[msg_base_vect_index],temp_time,"node_out");

                                }
                            }
                        }


                        outputFile << "At time "<<simTime().dbl()<<" "<<getName()<<", Introducing channel error with code " << flags_str_vect[loop_index]<<" and message = "<<msg_str_vect[loop_index]<<endl;

                        timeout_frames.push_back(msg_base_vect[msg_base_vect_index] -> dup());
                        scheduleAt(simTime()+10.5, timeout_frames[msg_base_vect_index]);

                        outputFile << "At time " << simTime().dbl()+pt_delay
                                << " Sender " << getName()
                                << " Sent Frame with Seq_Num = " << msg_base_vect[msg_base_vect_index]->getSeq_Num()
                                << " And Payload = " << msg_base_vect[msg_base_vect_index]->getM_Payload()
                                << " and trailer = " << msg_base_vect[msg_base_vect_index]->getMycheckbits()
                                << " and Modified " << curr_flag[0]
                                                                 << " ,Lost " << (loss == 1 ? "Yes" : "No")
                                                                 << " ,Duplicate " << duplication
                                                                 << " ,Delay " << delay << endl;
                        loss =0;
                        duplication =0;
                        delay = 0;
                        msg_base_vect_index++;
                    }


                }
                //If NACK received, retransmit without any errors
                else
                {
                    if(ack_array_sender[mmsg->getSeq_Num()]==0)
                    {
                        if(sender_ack[mmsg->getSeq_Num()]!=1) //two received at the same time -> ACK and NACK
                        {

                            int seq_to_resend = mmsg->getSeq_Num();
                            ack_array_sender[mmsg->getSeq_Num()]=1;

                            sender_ack[seq_to_resend] = 2;

                            MyMessage_Base *mmsg = new MyMessage_Base("channel");
                            mmsg->setM_Type(0);
                            mmsg->setSeq_Num(seq_to_resend);

                            re_sended_frames.push_back(mmsg->dup());
                            std::string curr_msg = msg_str_vect[seq_to_resend];

                            //////////////////////////////////////check-sum//////////////////////////////////////
                            std::bitset<8> checksum = calculateChecksum(curr_msg);
                            re_sended_frames[re_sended_frames_index]->setMycheckbits(checksum);
                            //////////////////////////////////////Done check-sum///////////////////////////////////


                            /////////////////////////////////////byte-stufing/////////////////////////////////////
                            std:: string msg_byte_stuffed_str = byteStuffing(curr_msg);
                            //mmsg->setM_Payload(msg_byte_stuffed_str.c_str());
                            re_sended_frames[re_sended_frames_index]->setM_Payload(msg_byte_stuffed_str.c_str());

                            /////////////////////////////////////Done byte-stufing/////////////////////////////////////

                            sender_alreadysent[seq_to_resend] = 1;
                            std:: string sentmsg = re_sended_frames[re_sended_frames_index] -> getM_Payload();
                            //coutaya
                            std:: cout << "i am the sender 3 and the curr time of re_sending is  : " << simTime().dbl() +pt_delay<< " and i am sending this : " << curr_msg << endl;

                            temp_time = pt_delay+tt_delay;
                            temp_time = roundToOneDecimal(temp_time);
                            sendDelayed( re_sended_frames[re_sended_frames_index],temp_time,"node_out");
                            outputFile << "At time " << simTime().dbl() + pt_delay
                                    << " Sender " << getName()
                                    << " Sent Frame with Seq_Num = " << re_sended_frames[re_sended_frames_index]->getSeq_Num()
                                    << " And Payload = " << re_sended_frames[re_sended_frames_index]->getM_Payload()
                                    << " and trailer = " << re_sended_frames[re_sended_frames_index]->getMycheckbits()
                                    << endl;
                            loss =0;
                            duplication =0;
                            delay = 0;
                            re_sended_frames_index++;

                        }
                    }
                }
            }
        }
        /////////////////////////////////////Done sender dealing with ACK and NACK///////////////////////////////////

        /************************************************************************************************
         *receiver receiving from sender (no scheduleAt at receiver as it only receives from sender)    *
         ************************************************************************************************/

        else
        {
            receivedMessage=mmsg->getM_Payload();
            outputFile << "At time " << simTime().dbl()
                                               << " Receiver " << getName()
                                               << " Received Frame with Seq_Num = " << mmsg->getSeq_Num()
                                               << " And Payload = " << mmsg->getM_Payload()
                                               << " and trailer = " << mmsg->getMycheckbits()
                                               << endl;
            MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);


            message="";
            receivedMessage=mmsg->getM_Payload();
            int flag=0;
            //Check Sequence Number
            //Start Destuffing
            for(int i=1;i<receivedMessage.length()-1;i++)
            {
                if( (receivedMessage[i]== '/' )&& flag==0)
                {
                    flag=1;
                    continue;
                }
                else
                {
                    message+=receivedMessage[i];
                    flag=0;
                }
            }

            // Initialize XOR with zero.
            std::bitset<8> XOR(0);

            //Checksum checking
            for(int i=0; i<message.length(); i++)
            {
                std::bitset<8> newChar (message[i]);
                XOR = XOR ^ newChar;
            }

            XOR = XOR ^ mmsg->getMycheckbits();
            if (XOR==0)
            {
                ack_array[mmsg->getSeq_Num()]=1;
            }
            else
            {
                ack_array[mmsg->getSeq_Num()]=2;
            }

            if (expectedSequenceNumber == mmsg->getSeq_Num()) //Correct Order
            {
                int loop_index=0;
                for(int i = mmsg->getSeq_Num();i<ack_array.size();i++)
                {

                    if(ack_array[i]==1) //while loop to check successive ACKS
                    {
                        mmsg->setM_Type(1);
                        mmsg->setSeq_Num(i);
                        mmsg->setM_Payload("ACK");
                        ack_base_vect.push_back(mmsg->dup());
                        temp_time = tot_delay+pt_delay*loop_index;
                        sendDelayed( ack_base_vect[ack_index],temp_time,"node_out");
                        ack_index++;
                        loop_index++;
                        outputFile << "At time " << simTime().dbl() + temp_time - tt_delay
                                << " Receiver " << getName()
                                << " Sent ACK with number " << i << endl;
                    }
                    else if(ack_array[i]==2)
                    {
                        mmsg->setM_Type(2);
                        mmsg->setSeq_Num(i);
                        mmsg->setM_Payload("NACK");
                        ack_base_vect.push_back(mmsg->dup());
                        temp_time = tot_delay+pt_delay*loop_index;
                        sendDelayed( ack_base_vect[ack_index],temp_time,"node_out");
                        ack_index++;
                        loop_index++;
                        expectedSequenceNumber=i;
                        outputFile << "At time " << simTime().dbl() + temp_time - tt_delay
                                << " Receiver " << getName()
                                << " Sent NACK with number " << i << endl;
                        break;
                    }
                    else
                    {

                        expectedSequenceNumber=i;
                        break;

                    }

                }

            }
            else //Out of Order
            {

                std::cout << "Sequence Number " <<mmsg->getSeq_Num()<< endl;
                std::cout << "Expected Sequence Number " <<expectedSequenceNumber<< endl;
                std::cout << "Time " <<simTime().dbl()<< endl;
                if(mmsg->getSeq_Num()>expectedSequenceNumber) //Delay or Loss
                {
                    if (XOR==0)
                    {
                        ack_array[mmsg->getSeq_Num()]=1;
                    }
                    else
                    {
                        ack_array[mmsg->getSeq_Num()]=2;
                    }
                    mmsg->setM_Type(2);
                    mmsg->setSeq_Num(expectedSequenceNumber);
                    mmsg->setM_Payload("NACK");
                    ack_base_vect.push_back(mmsg->dup());
                    temp_time = tt_delay+pt_delay;
                    sendDelayed(ack_base_vect[ack_index],temp_time,"node_out");
                    ack_index++;
                    outputFile << "At time " << simTime().dbl() + pt_delay
                            << " Receiver " << getName()
                            << " Sent NACK with number " << expectedSequenceNumber << endl;
                }
            }
        }
    }
}
/*assumptions
 * 1-I have a linear buffer
 * 2-I send nack multiple times and resend each one but the reciever ignore the duplicates
 */
