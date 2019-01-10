/*
 * MyEnodeB.cc
 *
 *  Created on: 2018¦~6¤ë1¤é
 *      Author: GGININ
 */


#include <string.h>
#include <stdio.h>
#include <omnetpp.h>
#include "MyMessage_m.h"

using namespace omnetpp;

class MyEnodeB : public cSimpleModule
{
  public:
    MyEnodeB();
    ~MyEnodeB();
  protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual MyMessage *generateMessage(const char *);


//    cMessage *msg;
    MyMessage *mymsg;
    cMessage *event;
  private:
    unsigned int ue_scheduled;
    unsigned int max_ue;
};

// The module class needs to be registered with OMNeT++
Define_Module(MyEnodeB);

MyEnodeB::MyEnodeB() {
    event = nullptr;
    ue_scheduled = 0;
    max_ue = 3000;
}

MyEnodeB::~MyEnodeB() {
    cancelAndDelete(event);

}

void MyEnodeB::initialize()
{
    event = new cMessage("event");
//    scheduleAt(0.1, event);

}

void MyEnodeB::handleMessage(cMessage *msg)
{

    if (msg == event) {
        EV << "EnodeB starts scheduling\n";
        mymsg = generateMessage("REP");
        scheduleAt(simTime()+1, mymsg);

    }
    else {
        mymsg = check_and_cast<MyMessage *>(msg);
        if (strcmp("REP", msg->getName())==0) {
            EV << "Received: " << msg->getName() << "\n";
            cGate * thisGateOut = gate("out");
            delete(mymsg);
            thisGateOut->disconnect(); // close forward direction
            EV << "Connection Closed" << "\n";

            if (ue_scheduled < max_ue) {
                std::string s = std::string("myue[")  + std::to_string(ue_scheduled) + std::string("]");
                const char * c = s.c_str();
                cModule * dest = getModuleByPath(c);
                cGate * destGateIn = dest->gate("c_in");
                cGate * thisGateOut = gate("out");
                thisGateOut->connectTo(destGateIn); // forward direction
                mymsg = generateMessage("Schedule");
                send(mymsg, "out");
                ++ue_scheduled;
            }

        }
        else {

        }
    }

}

MyMessage *MyEnodeB::generateMessage(const char *s) {
    MyMessage *msg = new MyMessage(s);
    msg->setSource(-1);
    return msg;
}
