/*
 * PositionHandler.cc
 *
 *  Created on: 2018¦~6¤ë1¤é
 *      Author: GGININ
 */


#include <string.h>
#include <stdio.h>
#include <omnetpp.h>
#include "MyMessage_m.h"
#include <vector>

using namespace omnetpp;


class PositionHandler : public cSimpleModule
{
  public:
    PositionHandler();
    ~PositionHandler();
  protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual MyMessage *generateMessage(const char *);

    MyMessage *mymsg;
    cMessage *event;
  private:
    unsigned int max_ue;
    double pos_x[3000];
    double pos_y[3000];

    std::vector<double> ue_x; //for watch
    std::vector<double> ue_y;
};

// The module class needs to be registered with OMNeT++
Define_Module(PositionHandler);

PositionHandler::PositionHandler() {
    event = nullptr;
    max_ue = 3000;
}

PositionHandler::~PositionHandler() {
    cancelAndDelete(event);

}

void PositionHandler::initialize()
{
    event = new cMessage("event");
    scheduleAt(0.1, event);

    for (int i = 0; i < 3000; ++i) {
        pos_x[i] = 0;
        pos_y[i] = 0;
        ue_x.push_back(0);
        ue_y.push_back(0);
    }
    WATCH_VECTOR(ue_x);
    WATCH_VECTOR(ue_y);

}
void PositionHandler::handleMessage(cMessage *msg)
{
    if (msg == event) {
        EV << "PositionHandler starts\n";
        //send pos to myScheduler
        cModule * dest = getModuleByPath("myScheduler");
        cGate * destGateIn = dest->gate("in");
        cGate * thisGateOut = gate("out");
        thisGateOut->connectTo(destGateIn); // forward direction
        mymsg = generateMessage("Pos");
        for (int j = 0; j < 3000; ++j) { //send all ue's position
            mymsg->setUe_x(j , pos_x[j]);
            mymsg->setUe_y(j , pos_y[j]);
        }
        send(mymsg, "out");
        thisGateOut->disconnect();

        for (int i = 0; i < max_ue; ++i) {
            std::string s = std::string("myue[")  + std::to_string(i) + std::string("]");
            const char * c = s.c_str();
            dest = getModuleByPath(c);
            destGateIn = dest->gate("h_in");
            thisGateOut = gate("out");
            thisGateOut->connectTo(destGateIn); // forward direction
            mymsg = generateMessage("Pos");
            for (int j = 0; j < 3000; ++j) { //send all ue's position
                mymsg->setUe_x(j , pos_x[j]);
                mymsg->setUe_y(j , pos_y[j]);
            }
            send(mymsg, "out");
            thisGateOut->disconnect();
        }

        ue_x.assign(pos_x , pos_x + max_ue);    //for watching
        ue_y.assign(pos_y , pos_y + max_ue);
        scheduleAt(simTime()+0.5, event);
    }
    else if (strcmp("Pos", msg->getName())==0)
    {
        mymsg = check_and_cast<MyMessage *>(msg);
        int UeID = mymsg->getSource();
        pos_x[UeID] = mymsg->getPos_x();
        pos_y[UeID] = mymsg->getPos_y();
        delete(mymsg);

    }

}

MyMessage *PositionHandler::generateMessage(const char *s) {
    MyMessage *msg = new MyMessage(s);
    msg->setSource(-1);
    return msg;
}

