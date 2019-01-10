/*
 * myScheduler.cc
 *
 *  Created on: 2018¦~6¤ë4¤é
 *      Author: GGININ
 */

#include <string.h>
#include <stdio.h>
#include <omnetpp.h>
#include "MyMessage_m.h"

using namespace omnetpp;

class MyScheduler : public cSimpleModule
{
  public:
    MyScheduler();
    ~MyScheduler();
  protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual MyMessage *generateMessage(const char *);


//    cMessage *msg;
    MyMessage *mymsg;
    cMessage *event;
  private:
    int max_ue;
    int schedule_ue;
    double pos_x[3000];
    double pos_y[3000];
    double group[3000];

    std::vector<double> ue_x; //for watch
    std::vector<double> ue_y;
    std::vector<double> ue_group;
};

// The module class needs to be registered with OMNeT++
Define_Module(MyScheduler);

MyScheduler::MyScheduler() {
    event = nullptr;

    schedule_ue = 0;
}

MyScheduler::~MyScheduler() {
    cancelAndDelete(event);
}

void MyScheduler::initialize()
{
    max_ue = this->par("maxUE");
    event = new cMessage("event");
    scheduleAt(0.1, event);

    for (int i = 0; i < max_ue; ++i) {
        pos_x[i] = 0;
        pos_y[i] = 0;
        group[i] = 0;
        ue_x.push_back(0);
        ue_y.push_back(0);
        ue_group.push_back(0);
    }

    WATCH_VECTOR(ue_x);
    WATCH_VECTOR(ue_y);
    WATCH_VECTOR(ue_group);
}

void MyScheduler::handleMessage(cMessage *msg)
{
    MyMessage *newmsg;
    if (msg == event) {
        EV << "MyScheduler starts\n";
        mymsg = generateMessage("RR");
        scheduleAt(simTime()+0.2, mymsg);
    }
    else if (strcmp("Pos", msg->getName())==0) {
        MyMessage *mymsg = check_and_cast<MyMessage *>(msg);
        for (int i = 0; i < max_ue; ++i) {
            pos_x[i] = mymsg->getUe_x(i);
            pos_y[i] = mymsg->getUe_y(i);
            ue_x[i] = mymsg->getUe_x(i);
            ue_y[i] = mymsg->getUe_y(i);
        }

        delete(mymsg);
        //group
        for (int i = 0; i < max_ue; ++i) {
            int x, y;
            if(pos_x[i] < 2000) x = 0;
            if(2000 <= pos_x[i] && pos_x[i] < 4000) x = 1;
            if(4000 <= pos_x[i] && pos_x[i] < 6000) x = 2;
            if(6000 <= pos_x[i] && pos_x[i] < 8000) x = 3;
            if(8000 <= pos_x[i] && pos_x[i] <= 10000) x = 4;
            if(pos_y[i] < 2000) y = 0;
            if(2000 <= pos_y[i] && pos_y[i]  < 4000) y = 1;
            if(4000 <= pos_y[i] && pos_y[i] < 6000) y = 2;
            if(6000 <= pos_y[i] && pos_y[i] < 8000) y = 3;
            if(8000 <= pos_y[i] && pos_y[i] <= 10000) y = 4;
            group[i] = x+y*10;
            ue_group[i] = group[i];
        }

    }
    else if (strcmp("RR", msg->getName())==0) {
        delete(msg);

        if (schedule_ue > max_ue-1) {// > 2999
            schedule_ue = 0;
            newmsg = generateMessage("RR");
            scheduleAt(simTime()+0.1, newmsg);
        }
        else {
            int type = intuniform(0, 2);
            while (type != 0) { // uplink or not scheduled
                type = intuniform(0, 2);
                ++schedule_ue;
            }
            if (schedule_ue < max_ue) { // 0~2999
                std::string s = std::string("myue[")  + std::to_string(schedule_ue) + std::string("]");
                const char * c = s.c_str();
                cModule * dest = getModuleByPath(c);
                cGate * destGateIn = dest->gate("c_in");
                cGate *thisGateOut = gate("out");
                thisGateOut->connectTo(destGateIn); // forward direction
                newmsg = generateMessage("Schedule");
                for (int j = 0; j < max_ue; ++j) { //send all ue's position
                    newmsg->setGroup(j , group[j]);
                }
                send(newmsg, "out");
                thisGateOut->disconnect();
                ++schedule_ue;
            }
            else { //>2999
                schedule_ue = 0;
                newmsg = generateMessage("RR");
                scheduleAt(simTime()+0.1, newmsg);
            }

        }

    }


}

MyMessage *MyScheduler::generateMessage(const char *s) {
    MyMessage *msg = new MyMessage(s);
    msg->setSource(-1);
    return msg;
}


