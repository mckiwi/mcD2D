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



simple MyEnodeB
{
    parameters:
        @display("i=device/antennatower_s");
    gates:
        input in @loose;
        output out @loose;

}

simple PositionHandler
{
    parameters:
        @display("i=device/device");
        int maxUE = default(3000);
    gates:
        input in @loose;
        output out @loose;
}

simple MyScheduler
{
    parameters:
        @display("i=device/device");
        int maxUE = default(3000);
    gates:
        input in @loose;
        input d_in @loose;
        output out @loose;

}


network Network
{
    @display("bgb=13165.265,10984.393");


    int n @prompt("Number of nodes") = default(3000);

    volatile int posX = intuniform (0,10000);
    volatile int posY = intuniform (0,10000);



    submodules:
        myue[n]: MobileHost {
            parameters:
                @display("p=$posX,$posY");

        }
        positionHandler: PositionHandler {
            parameters:
                @display("p=10000, 10000");
        }
        myScheduler: MyScheduler {
            parameters:
                @display("p=10000, 0");
        }

        myEnodeB0: MyEnodeB {
            parameters:
                @display("p=1000,1000");
        }
        myEnodeB1: MyEnodeB {
            parameters:
                @display("p=3000,1000");
        }
        myEnodeB2: MyEnodeB {
            parameters:
                @display("p=5000,1000");
        }
        myEnodeB3: MyEnodeB {
            parameters:
                @display("p=7000,1000");
        }
        myEnodeB4: MyEnodeB {
            parameters:
                @display("p=9000,1000");
        }

        myEnodeB5: MyEnodeB {
            parameters:
                @display("p=1000,3000");
        }
        myEnodeB6: MyEnodeB {
            parameters:
                @display("p=3000,3000");
        }
        myEnodeB7: MyEnodeB {
            parameters:
                @display("p=5000,3000");
        }
        myEnodeB8: MyEnodeB {
            parameters:
                @display("p=7000,3000");
        }
        myEnodeB9: MyEnodeB {
            parameters:
                @display("p=9000,3000");
        }

        myEnodeB10: MyEnodeB {
            parameters:
                @display("p=1000,5000");
        }
        myEnodeB11: MyEnodeB {
            parameters:
                @display("p=3000,5000");
        }
        myEnodeB12: MyEnodeB {
            parameters:
                @display("p=5000,5000");
        }
        myEnodeB13: MyEnodeB {
            parameters:
                @display("p=7000,5000");
        }
        myEnodeB14: MyEnodeB {
            parameters:
                @display("p=9000,5000");
        }

        myEnodeB15: MyEnodeB {
            parameters:
                @display("p=1000,7000");
        }
        myEnodeB16: MyEnodeB {
            parameters:
                @display("p=3000,7000");
        }
        myEnodeB17: MyEnodeB {
            parameters:
                @display("p=5000,7000");
        }
        myEnodeB18: MyEnodeB {
            parameters:
                @display("p=7000,7000");
        }
        myEnodeB19: MyEnodeB {
            parameters:
                @display("p=9000,7000");
        }

        myEnodeB20: MyEnodeB {
            parameters:
                @display("p=1000,9000");
        }
        myEnodeB21: MyEnodeB {
            parameters:
                @display("p=3000,9000");
        }
        myEnodeB22: MyEnodeB {
            parameters:
                @display("p=5000,9000");
        }
        myEnodeB23: MyEnodeB {
            parameters:
                @display("p=7000,9000");
        }
        myEnodeB24: MyEnodeB {
            parameters:
                @display("p=9000,9000");
        }



        myEnodeB: MyEnodeB {
            @display("p=1428.4401,7471.84");
        }
}
