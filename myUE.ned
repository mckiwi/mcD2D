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



import inet.mobility.contract.IMobility;

simple Myue
{
    parameters:
        @display("i=device/pocketpc_vs");
        @display("t=123");
        //# D2D capability
        bool d2dCapable = default(false);
        string nicType = d2dCapable ? "LteNicUeD2D" : "LteNicUe";
    gates:
        input c_in;
        input d_in;
        input h_in;
        output c_out;
        output d_out;
        output h_out;
        
}
//
module MobileHost
{
    parameters:
        string mobilityType = default("StationaryMobility");
        int token = default(11);
        int threshold = default(2);
        int minRingRadius = default(400);
        int maxRingRadius = default(700);
        @networkNode();
        @node;
        @display("i=device/pocketpc_s;bgb=518,197");
        @statistic[coordX](title="coord X"; source="xCoord(mobilityPos(mobilityStateChanged))"; record=vector; interpolationmode=linear);
        @statistic[coordY](title="coord Y"; source="yCoord(mobilityPos(mobilityStateChanged))"; record=vector; interpolationmode=linear);
    //@statistic[coordZ](title="coord Z";source="zCoord(mobilityPos(mobilityStateChanged))";record=vector; interpolationmode=linear);
    gates:
        input c_in @loose;
        input d_in @loose;
        input h_in @loose;
        output c_out @loose;
        output d_out @loose;
        output h_out @loose;
    submodules:
        mobility: <mobilityType> like IMobility;
        myue: Myue {

            @display("p=135,85");
        }
    connections:
        c_in --> myue.c_in;
        d_in --> myue.d_in;
        h_in --> myue.h_in;
        myue.c_out --> c_out;
        myue.d_out --> d_out;
		myue.h_out --> h_out;
}