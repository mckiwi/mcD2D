/*
 * MyUE.cc
 *
 *  Created on: 2018¦~6¤ë1¤é
 *      Author: GGININ
 */


#include <string.h>
#include <omnetpp.h>
#include <stdlib.h>
#include "MyMessage_m.h"
#include <math.h>
#include <queue>



using namespace omnetpp;



class Myue : public cSimpleModule
{
  public:
    Myue();
    ~Myue();
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual MyMessage *generateMessage(const char* s);
    virtual void RandomWPMobility();
    virtual void LinearMobility();
    virtual void MassMobility();
    virtual void RandomWalkMobility();
    virtual int ModeSelection();
    virtual int Myabs(int a);
    virtual int TokenPolicy();
    virtual int ModeSelection2();
    virtual int TokenPolicy2();
    cMessage *msg;
    cMessage *event;
  private:
    int pairUE;
    double pos_x;
    double pos_y;
    double pre_x;
    double pre_y;
    double ue_x[3000];
    double ue_y[3000];

    std::vector<double> x_watch;
    std::vector<double> y_watch;
    std::vector<double> ue_group;
    std::queue<unsigned int> q;
    int max_ue;
    int direction;
    double sinr;
    double targetSinr;
    double sinrGain;
    int token;
    unsigned int power_budget;
    double sinrGainMean;
    double sinrGainSum;
    double D2DTimeSlot;
    double speed;
    double massCount; // for MassMobility
    int type; // for MassMobility
    int minRingRadius;
    int maxRingRadius;
    int threshold;
//    cOutVector sinrGainOut;
    int failCount;
    bool mark[3000];
    bool target;
    int lifetime;
    int randomWPCount;

};

// The module class needs to be registered with OMNeT++
Define_Module(Myue);

Myue::Myue() {
    event = msg = nullptr;
}

Myue::~Myue() {
    cancelAndDelete(event);
    delete msg;
}

void Myue::initialize()
{
    pos_x = intuniform(0 , 10000-1);
    pos_y = intuniform(0 , 10000-1);
    pre_x = pos_x;
    pre_y = pos_y;
    for (int i = 0; i < 3000; ++i) {
        x_watch.push_back(0);
        y_watch.push_back(0);
        ue_group.push_back(0);
        mark[i] = true; // for trace users in ring
    }
    WATCH(pos_x);
    WATCH(pos_y);
    WATCH(token);
    WATCH_VECTOR(x_watch);
    WATCH_VECTOR(y_watch);
    WATCH_VECTOR(ue_group);
    event = generateMessage("Move");
    scheduleAt(0.1, event);
    direction = -1; //default
    token = this->getParentModule()->par("token");
    max_ue = 3000;
    sinr = 0; //-10~25
    targetSinr = 0; //-10~0 ,0~13, 13~25
    power_budget = 300;
//    sinrGainOut.setName("sinrGain");
    sinrGain = 0;
    sinrGainMean = 0;
    sinrGainSum = 0;
    D2DTimeSlot = 0;
    massCount = 0;

    if (intuniform(0,9) <= 2) type = 0;
    else type = 1;
//    if (intuniform(0,9) <= 2) speed = 0.16;
//    else speed = 1.6;

    threshold = this->getParentModule()->par("threshold");

    if (intuniform(0,9) <= threshold) power_budget = 100; //power
    else power_budget = 300;

    minRingRadius = this->getParentModule()->par("minRingRadius");
    maxRingRadius = this->getParentModule()->par("maxRingRadius");
    failCount = 0;
    target = false;
    lifetime = 0;
    randomWPCount = uniform(1,1000);
}

void Myue::finish() {

        if (D2DTimeSlot != 0)
            sinrGainMean = sinrGainSum/D2DTimeSlot;
        else
            sinrGainMean = 0;
        //recordScalar("#failCount", failCount);
        recordScalar("#sinrGainMean", sinrGainMean);

}

void Myue::handleMessage(cMessage *msg)
{
    MyMessage *mymsg = check_and_cast<MyMessage *>(msg);
    MyMessage *newmsg;
    EV << "Received: " << mymsg->getName() << "\n";
//    EV << "token : " << token << "\n";


    if (strcmp("Move", mymsg->getName())==0) {
        MassMobility();
        cancelAndDelete(event);
        event = generateMessage("Move");
        scheduleAt(simTime()+0.1, event);
        EV << "x = " << pos_x << ", y = " << pos_y << "\n";

    }
    else if (strcmp("Schedule", mymsg->getName())==0) {
       EV << " x : " << pos_x << " y : " << pos_y << " token : " << token << " ";
       cModule * parent = this->getParentModule();
       pairUE = -1; //BS
       std::priority_queue<unsigned int> pq;
       if (ModeSelection()) {
           EV << "D2D MODE" << "\n";
           ++D2DTimeSlot;
           int x, y;
           if(pos_x < 2000) x = 0;
           if(2000 <= pos_x && pos_x < 4000) x = 1;
           if(4000 <= pos_x && pos_x < 6000) x = 2;
           if(6000 <= pos_x && pos_x < 8000) x = 3;
           if(8000 <= pos_x && pos_x <= 10000) x = 4;
           if(pos_y < 2000) y = 0;
           if(2000 <= pos_y && pos_y < 4000) y = 1;
           if(4000 <= pos_y && pos_y < 6000) y = 2;
           if(6000 <= pos_y && pos_y < 8000) y = 3;
           if(8000 <= pos_y && pos_y <= 10000) y = 4;
           int myGroup = x+y*10;
           int bs_x = (myGroup%10)*2000 + 1000;
           int bs_y = (myGroup/10)*2000 + 1000;
           double d_BS = std::sqrt( Myabs(std::pow(bs_x-pos_x , 2)) + std::pow(Myabs(bs_y-pos_y) , 2) ); // d between this UE and BS
           if (d_BS < minRingRadius) targetSinr = 9999; //big enough
           else if (minRingRadius <= d_BS && d_BS < maxRingRadius) {
               targetSinr =  uniform(13,20);

           }
           else targetSinr = uniform(0,13);

           for (unsigned int i = 0; i < max_ue; ++i) {

               int ueGroup = mymsg->getGroup(i);
               ue_group[i] = ueGroup;

               if (myGroup == ueGroup) {
                   double d = std::sqrt(std::pow(Myabs(pos_x - ue_x[i]) , 2) + std::pow(Myabs(pos_y - ue_y[i]) , 2)); //d between two UE
                   double d_targetToBS = std::sqrt(std::pow(Myabs(bs_x - ue_x[i]) , 2) + std::pow(Myabs(bs_y - ue_y[i]) , 2) );
                   if (d <= 3500 && d_BS >= d_targetToBS && i != parent->getIndex()) { //d2d range UE
                       pq.emplace(i);
                   }
               }

           }
           while (!pq.empty()) {
               q.push(pq.top());
               pq.pop();
           }
           if (q.empty() || token <= 0) {
               EV << "Qsize " << q.size() << " token = " << token << "\n";
               cModule * dest = getModuleByPath("myScheduler");
               cGate * destGateIn = dest->gate("in");
               cGate *thisGateOut = this->getParentModule()->gate("c_out");
               thisGateOut->connectTo(destGateIn); // forward direction
               newmsg = generateMessage("RR");
               send(newmsg, "c_out");
               thisGateOut->disconnect();
               EV << "pairUE = " << pairUE << "\n";
               while(!q.empty()) q.pop();
           }
           else {
               pairUE = q.front();
               q.pop();
               std::string s = std::string("myue[")  + std::to_string(pairUE) + std::string("]");
               const char * c = s.c_str();
               cModule * dest = getModuleByPath(c);
               cGate * destGateIn = dest->gate("d_in");
               cGate *thisGateOut = this->getParentModule()->gate("d_out");
               thisGateOut->connectTo(destGateIn); // forward direction
               newmsg = generateMessage("Req");
               newmsg->setSource(this->getParentModule()->getIndex());
               send(newmsg, "d_out");
               thisGateOut->disconnect();
           }


       }
       else {
           cModule * dest = getModuleByPath("myScheduler");
           cGate * destGateIn = dest->gate("in");
           cGate *thisGateOut = this->getParentModule()->gate("c_out");
           thisGateOut->connectTo(destGateIn); // forward direction
           newmsg = generateMessage("RR");
           send(newmsg, "c_out");
           thisGateOut->disconnect();
           EV << "CELLULAR MODE" << "\n";
       }
       delete(mymsg);
//       sinrGainOut.record(sinrGain);
       int id = this->getParentModule()->getIndex();
       if (sinrGain < 0.01 && sinrGain > 0) sinrGain = 0.01;
       if (sinrGain > 0 && mark[id] == true) {
           sinrGainSum += sinrGain;
       }
       sinrGain = 0;
    }
    else if (strcmp("Fail", mymsg->getName())==0) {
        delete(mymsg);
        if (q.empty() || token <= 0) {
            if (token > 0) --token;
            cModule * dest = getModuleByPath("myScheduler");
            cGate * destGateIn = dest->gate("in");
            cGate *thisGateOut = this->getParentModule()->gate("c_out");
            thisGateOut->connectTo(destGateIn); // forward direction
            newmsg = generateMessage("RR");
            send(newmsg, "c_out");
            thisGateOut->disconnect();
//            EV << "D2D FAIL"  << "\n";
//            EV << "Qsize " << q.size() << " token = " << token << "\n";
            int id = this->getParentModule()->getIndex();
            if (token > 0 && mark[id] == true)
                ++failCount;
        }
        else {
            pairUE = q.front();
            q.pop();
            std::string s = std::string("myue[")  + std::to_string(pairUE) + std::string("]");
            const char * c = s.c_str();
            cModule * dest = getModuleByPath(c);
            cGate * destGateIn = dest->gate("d_in");
            cGate *thisGateOut = this->getParentModule()->gate("d_out");
            thisGateOut->connectTo(destGateIn); // forward direction
            newmsg = generateMessage("Req");
            newmsg->setSource(this->getParentModule()->getIndex());
            send(newmsg, "d_out");
            thisGateOut->disconnect();
        }
    }
    else if (strcmp("Success", mymsg->getName())==0) {
        pairUE = mymsg->getSource();
        delete(mymsg);
        if (token > 0) --token;
        cModule * dest = getModuleByPath("myScheduler");
        cGate * destGateIn = dest->gate("in");
        cGate *thisGateOut = this->getParentModule()->gate("c_out");
        thisGateOut->connectTo(destGateIn); // forward direction
        newmsg = generateMessage("RR");
        send(newmsg, "c_out");
        thisGateOut->disconnect();
//        EV << "pairUE = " << pairUE << " newSinr : " << targetSinr << "\n";
        sinrGain = Myabs(targetSinr - sinr);

        while(!q.empty()) q.pop();

    }
    else if (strcmp("Req", mymsg->getName())==0) {
        EV << "Req : received from : " << mymsg->getSource() << "\n";
//      unsigned int chance = intuniform(0, 1);
        if (TokenPolicy()) {
            newmsg = generateMessage("Success");
            ++token;
        }
        else newmsg = generateMessage("Fail");

        newmsg->setSource(this->getParentModule()->getIndex());

        pairUE = mymsg->getSource();
        std::string s = std::string("myue[")  + std::to_string(pairUE) + std::string("]");
        const char * c = s.c_str();
        cModule *dest = getModuleByPath(c);
        cGate * destGateIn = dest->gate("d_in");
        cGate * thisGateOut = this->getParentModule()->gate("d_out");
        thisGateOut->connectTo(destGateIn); // reverse D2D direction
        delete(mymsg);
        send(newmsg, "d_out");
        thisGateOut->disconnect();

    }
    else if (strcmp("Pos", mymsg->getName())==0) { //position syn
        cModule * dest = getModuleByPath("positionHandler");
        cGate * destGateIn = dest->gate("in");
        cGate * thisGateOut = this->getParentModule()->gate("h_out");
        thisGateOut->connectTo(destGateIn); // reverse direction

        for (int i = 0; i < 3000; ++i) {
            ue_x[i] = mymsg->getUe_x(i);
            ue_y[i] = mymsg->getUe_y(i);
        }
        x_watch.assign(ue_x , ue_x + 3000);
        y_watch.assign(ue_y , ue_y + 3000);

        delete(mymsg);
        newmsg = generateMessage("Pos");
        newmsg->setPos_x(pos_x);
        newmsg->setPos_y(pos_y);
        cModule * parent = this->getParentModule();
        newmsg->setSource(parent->getIndex());
        send(newmsg , "h_out");
        thisGateOut->disconnect();
    }
    else {

    }

}

MyMessage *Myue::generateMessage(const char* s) {
    MyMessage *msg = new MyMessage(s);
    return msg;
}

void Myue::RandomWPMobility() {
    unsigned int way = 0;
    if (massCount == 0) way = intuniform(0, 12-1);
    pre_x = pos_x;
    pre_y = pos_y;
    direction = way;

    if (type == 0) {
        speed = uniform(0 , 0.2);
    }
    else {
        speed = uniform(1.6 , 2);
    }

    if (massCount > randomWPCount) {
        way = intuniform(0, 12-1);
        randomWPCount = uniform(0, 1000);
        massCount = 1;
    }


    switch(way) {
        case 0: pos_y += speed; break;
        case 1: pos_y += sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 2: pos_y += sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 3: pos_x += speed; break;
        case 4: pos_y -= sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 5: pos_y -= sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 6: pos_y -= speed; break;
        case 7: pos_y -= sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 8: pos_y -= sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
        case 9: pos_x -= speed; break;
        case 10: pos_y += sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 11: pos_y += sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
    }
    if (pos_y >= 10000) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_y <= 0) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_x >= 10000) {pos_x = 0; direction = intuniform(0, 12-1);}
    if (pos_x <= 0) {pos_x = 10000; direction = intuniform(0, 12-1);}

}
void Myue::RandomWalkMobility() {
    unsigned int way = intuniform(0, 12-1);
    pre_x = pos_x;
    pre_y = pos_y;
    direction = way;

    if (type == 0) {
        speed = uniform(0 , 0.2);
    }
    else {
        speed = uniform(1.6 , 2);
    }



    switch(way) {
        case 0: pos_y += speed; break;
        case 1: pos_y += sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 2: pos_y += sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 3: pos_x += speed; break;
        case 4: pos_y -= sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 5: pos_y -= sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 6: pos_y -= speed; break;
        case 7: pos_y -= sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 8: pos_y -= sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
        case 9: pos_x -= speed; break;
        case 10: pos_y += sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 11: pos_y += sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
    }
    if (pos_y >= 10000) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_y <= 0) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_x >= 10000) {pos_x = 0; direction = intuniform(0, 12-1);}
    if (pos_x <= 0) {pos_x = 10000; direction = intuniform(0, 12-1);}

}

void Myue::LinearMobility() {
    unsigned int way = intuniform(0, 8-1);
    pre_x = pos_x;
    pre_y = pos_y;
    if (direction == -1) {
        direction = way;
    }
    else
        way = direction;
    if (type == 0) {
        speed = uniform(0 , 0.2);
    }
    else {
        speed = uniform(1.6 , 2);
    }
    switch(way) {
        case 0: pos_y += speed; break;
        case 1: pos_y += sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 2: pos_y += sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 3: pos_x += speed; break;
        case 4: pos_y -= sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 5: pos_y -= sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 6: pos_y -= speed; break;
        case 7: pos_y -= sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 8: pos_y -= sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
        case 9: pos_x -= speed; break;
        case 10: pos_y += sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 11: pos_y += sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
    }
    if (pos_y >= 10000) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_y <= 0) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_x >= 10000) {pos_x = 0; direction = intuniform(0, 12-1);}
    if (pos_x <= 0) {pos_x = 10000; direction = intuniform(0, 12-1);}

}

void Myue::MassMobility() {
    unsigned int way = 0;
    if (massCount == 0) way = intuniform(0, 12-1);
    pre_x = pos_x;
    pre_y = pos_y;
    direction = way;

    if (type == 0) {
        speed = uniform(0 , 0.2);
    }
    else {
        speed = uniform(1.6 , 2);
    }

    if (massCount > 50) {
        int tmp = intuniform(0 , 1);
        if (tmp == 0) --way;
        else ++way;
        massCount = 1;
    }

    switch(way) {
        case 0: pos_y += speed; break;
        case 1: pos_y += sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 2: pos_y += sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 3: pos_x += speed; break;
        case 4: pos_y -= sin(60*PI/180)*speed; pos_x += cos(60*PI/180)*speed; break;
        case 5: pos_y -= sin(30*PI/180)*speed; pos_x += cos(30*PI/180)*speed; break;
        case 6: pos_y -= speed; break;
        case 7: pos_y -= sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 8: pos_y -= sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
        case 9: pos_x -= speed; break;
        case 10: pos_y += sin(30*PI/180)*speed; pos_x -= cos(30*PI/180)*speed; break;
        case 11: pos_y += sin(60*PI/180)*speed; pos_x -= cos(60*PI/180)*speed; break;
    }

    if (pos_y >= 10000) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_y <= 0) {pos_y = 10000; direction = intuniform(0, 12-1);}
    if (pos_x >= 10000) {pos_x = 0; direction = intuniform(0, 12-1);}
    if (pos_x <= 0) {pos_x = 10000; direction = intuniform(0, 12-1);}



    ++massCount;
}

int Myue::ModeSelection() { //0 Cellular >1 D2D
    cModule * parent = this->getParentModule();
    int x, y;
    if(pos_x < 2000) x = 0;
    if(2000 <= pos_x && pos_x < 4000) x = 1;
    if(4000 <= pos_x && pos_x < 6000) x = 2;
    if(6000 <= pos_x && pos_x < 8000) x = 3;
    if(8000 <= pos_x && pos_x <= 10000) x = 4;
    if(pos_y < 2000) y = 0;
    if(2000 <= pos_y && pos_y < 4000) y = 1;
    if(4000 <= pos_y && pos_y < 6000) y = 2;
    if(6000 <= pos_y && pos_y < 8000) y = 3;
    if(8000 <= pos_y && pos_y <= 10000) y = 4;
    int myGroup = x+y*10;

    int bs_x = (myGroup%10)*2000 + 1000;
    int bs_y = (myGroup/10)*2000 + 1000;
    int a = bs_x-pos_x; if (a < 0) a = -a;
    int b = bs_y-pos_y; if (b < 0) b = -b;
    double d_BS = std::sqrt( std::pow(a , 2) + std::pow(b , 2) );
    sinr = (1500-d_BS) /  1500 * 35 - 10;
    EV << "d_BS : " << d_BS  << " bsx : " << bs_x << " bsy : " << bs_y << " SINR : " << sinr << " ";
    int id = this->getParentModule()->getIndex();

    if (d_BS <= minRingRadius && mark[id] == true) mark[id] = true;
    else mark[id] = false;
    if (token <= 0) return 0;
    if (d_BS <= minRingRadius) return 0;
    mark[id] = true;
    target = true;
    if (d_BS > 1500) mark[id] = false;
    if (d_BS > maxRingRadius) return 1;





    int pre_d_BS = std::sqrt( std::pow(Myabs(bs_x-pre_x) , 2) + std::pow(Myabs(bs_y-pre_y) , 2) );

    x = pos_x;
    y = pos_y;
    int way = direction;
    int count = 0;
    int speed = 350;//d2d range

    for (unsigned int i = 0; i < max_ue; ++i) {
        double d = std::sqrt(std::pow(Myabs(x - ue_x[i]) , 2) + std::pow(Myabs(y - ue_y[i]) , 2)); //d between two UE
        if (d <= 1000  && i != parent->getIndex()) { //d2d range UE
            ++count;
        }
    }


    if (d_BS >= pre_d_BS) { //go away from BS
        while (d_BS < 1000+350) {
            d_BS = std::sqrt( std::pow(Myabs(bs_x-x) , 2) + std::pow(Myabs(bs_y-y) , 2) );
            switch(way) {
                case 0: y += speed; break;
                case 1: y += sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                case 2: y += sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                case 3: x += speed; break;
                case 4: y -= sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                case 5: y -= sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                case 6: y -= speed; break;
                case 7: y -= sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                case 8: y -= sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
                case 9: x -= speed; break;
                case 10: y += sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                case 11: y += sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
            }

            for (unsigned int i = 0; i < max_ue; ++i) {
                double d = std::sqrt(std::pow(Myabs(x - ue_x[i]) , 2) + std::pow(Myabs(x - ue_y[i]) , 2)); //d between two UE
                if (d <= 500  && i != parent->getIndex()) { //d2d range UE
                    ++count;
                }
            }
        }
    }
    else { //get closer to BS
        way = (way+6) % 12;
        while (d_BS < 1000+350) {
            d_BS = std::sqrt( std::pow(Myabs(bs_x-x) , 2) + std::pow(Myabs(bs_y-y) , 2) );
            switch(way) {
                case 0: y += speed; break;
                case 1: y += sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                case 2: y += sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                case 3: x += speed; break;
                case 4: y -= sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                case 5: y -= sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                case 6: y -= speed; break;
                case 7: y -= sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                case 8: y -= sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
                case 9: x -= speed; break;
                case 10: y += sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                case 11: y += sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
            }
            for (unsigned int i = 0; i < max_ue; ++i) {
                double d = std::sqrt(std::pow(Myabs(x - ue_x[i]) , 2) + std::pow(Myabs(pos_y - ue_y[i]) , 2)); //d between two UE
                if (d <= 500  && i != parent->getIndex()) { //d2d range UE
                    ++count;
                }
            }
        }
    }
    if (count >= 1) return count;
    else return 0;

}

int Myue::TokenPolicy() {
    //if (token > exp(power_budget)*3/(exp(power_budget)+1)) return 0;
    if (token > sqrt(power_budget)*4) return 0;
    int x, y;
    if(pos_x < 2000) x = 0;
    if(2000 <= pos_x && pos_x < 4000) x = 1;
    if(4000 <= pos_x && pos_x < 6000) x = 2;
    if(6000 <= pos_x && pos_x < 8000) x = 3;
    if(8000 <= pos_x && pos_x <= 10000) x = 4;
    if(pos_y < 2000) y = 0;
    if(2000 <= pos_y && pos_y < 4000) y = 1;
    if(4000 <= pos_y && pos_y < 6000) y = 2;
    if(6000 <= pos_y && pos_y < 8000) y = 3;
    if(8000 <= pos_y && pos_y <= 10000) y = 4;
    int myGroup = x+y*10;
    int bs_x = (myGroup%10)*2000 + 1000;
    int bs_y = (myGroup/10)*2000 + 1000;

    double d_BS = std::sqrt( std::pow(Myabs(bs_x-pos_x) , 2) + std::pow(Myabs(bs_y-pos_y) , 2) );
    if (d_BS <= minRingRadius) return token > exp(power_budget/3)/(exp(power_budget/3)+1) ? 0 : 1;
    if (d_BS <= maxRingRadius) {
        int count = ModeSelection();
        return count;
    }
    return 0;

}

int Myue::ModeSelection2() { //0 Cellular >1 D2D
    cModule * parent = this->getParentModule();
        int x, y;
        if(pos_x < 2000) x = 0;
        if(2000 <= pos_x && pos_x < 4000) x = 1;
        if(4000 <= pos_x && pos_x < 6000) x = 2;
        if(6000 <= pos_x && pos_x < 8000) x = 3;
        if(8000 <= pos_x && pos_x <= 10000) x = 4;
        if(pos_y < 2000) y = 0;
        if(2000 <= pos_y && pos_y < 4000) y = 1;
        if(4000 <= pos_y && pos_y < 6000) y = 2;
        if(6000 <= pos_y && pos_y < 8000) y = 3;
        if(8000 <= pos_y && pos_y <= 10000) y = 4;
        int myGroup = x+y*10;

        int bs_x = (myGroup%10)*2000 + 1000;
        int bs_y = (myGroup/10)*2000 + 1000;
        int a = bs_x-pos_x; if (a < 0) a = -a;
        int b = bs_y-pos_y; if (b < 0) b = -b;
        double d_BS = std::sqrt( std::pow(a , 2) + std::pow(b , 2) );
        sinr = (1500-d_BS) /  1500 * 35 - 10;
        EV << "d_BS : " << d_BS  << " bsx : " << bs_x << " bsy : " << bs_y << " SINR : " << sinr << " ";
        int id = this->getParentModule()->getIndex();

        if (d_BS <= minRingRadius && mark[id] == true) mark[id] = true;
        else mark[id] = false;
        if (token <= 0) return 0;
        if (d_BS <= minRingRadius) return 0;
        mark[id] = true;
        target = true;
        if (d_BS > 1500) mark[id] = false;
        if (d_BS > maxRingRadius) return 1;





        int pre_d_BS = std::sqrt( std::pow(Myabs(bs_x-pre_x) , 2) + std::pow(Myabs(bs_y-pre_y) , 2) );

        x = pos_x;
        y = pos_y;
        int way = direction;
        int count = 0;
        int speed = 350;//d2d range

        for (unsigned int i = 0; i < max_ue; ++i) {
            double d = std::sqrt(std::pow(Myabs(x - ue_x[i]) , 2) + std::pow(Myabs(y - ue_y[i]) , 2)); //d between two UE
            if (d <= 1000  && i != parent->getIndex()) { //d2d range UE
                ++count;
            }
        }


        if (d_BS >= pre_d_BS) { //go away from BS
            while (d_BS < 1000+350) {
                d_BS = std::sqrt( std::pow(Myabs(bs_x-x) , 2) + std::pow(Myabs(bs_y-y) , 2) );
                switch(way) {
                    case 0: y += speed; break;
                    case 1: y += sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                    case 2: y += sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                    case 3: x += speed; break;
                    case 4: y -= sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                    case 5: y -= sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                    case 6: y -= speed; break;
                    case 7: y -= sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                    case 8: y -= sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
                    case 9: x -= speed; break;
                    case 10: y += sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                    case 11: y += sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
                }

                for (unsigned int i = 0; i < max_ue; ++i) {
                    double d = std::sqrt(std::pow(Myabs(x - ue_x[i]) , 2) + std::pow(Myabs(x - ue_y[i]) , 2)); //d between two UE
                    if (d <= 500  && i != parent->getIndex()) { //d2d range UE
                        ++count;
                    }
                }
            }
        }
        else { //get closer to BS
            way = (way+6) % 12;
            while (d_BS < 1000+350) {
                d_BS = std::sqrt( std::pow(Myabs(bs_x-x) , 2) + std::pow(Myabs(bs_y-y) , 2) );
                switch(way) {
                    case 0: y += speed; break;
                    case 1: y += sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                    case 2: y += sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                    case 3: x += speed; break;
                    case 4: y -= sin(60*PI/180)*speed; x += cos(60*PI/180)*speed; break;
                    case 5: y -= sin(30*PI/180)*speed; x += cos(30*PI/180)*speed; break;
                    case 6: y -= speed; break;
                    case 7: y -= sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                    case 8: y -= sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
                    case 9: x -= speed; break;
                    case 10: y += sin(30*PI/180)*speed; x -= cos(30*PI/180)*speed; break;
                    case 11: y += sin(60*PI/180)*speed; x -= cos(60*PI/180)*speed; break;
                }
                for (unsigned int i = 0; i < max_ue; ++i) {
                    double d = std::sqrt(std::pow(Myabs(x - ue_x[i]) , 2) + std::pow(Myabs(pos_y - ue_y[i]) , 2)); //d between two UE
                    if (d <= 500  && i != parent->getIndex()) { //d2d range UE
                        ++count;
                    }
                }
            }
        }
        if (count >= 1) return count;
        else return 0;
}

int Myue::TokenPolicy2() {
    if (token <= 7) {
        if (power_budget <= 100) return false;
        else return true;
    }
    else if (token <= 12) {
        if (power_budget <= 200) return false;
        else return true;
    }
    else if (token <= 13) {
        if (power_budget <= 500) return false;
        else return true;
    }
    else if (token <= 14) {
        if (power_budget <= 600) return false;
        else return true;
    }
    return false;
}


int Myue::Myabs(int a) {
    if (a < 0) return -a;
    return a;
}
