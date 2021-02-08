/*
 *
 *
 *
 */

#include <string.h>


int sensorFSM(void *sensorMsg) {

    static int sensorTotal = 0, sensorCount = 0;
    static int fsmState = 0; // 0 for INIT_AVERAGE, 1 for UPDATE_AVERAGE

    /*switch(fsmState) {

        case 0:
            if(!strcmp(sensorMsg->msg, "TIMER500_MESSAGE"))
                fsmState = 1;
            else if(!strcmp(sensorMsg->msg, "TIMER70_MESSAGE"))
                fsmState = 0;
            else
                // handle error
            break;

        case 1:
            if(!strcmp(sensorMsg->msg, "TIMER500_MESSAGE"))
                //do something
                fsmState++;
            else if(!strcmp(sensorMsg->msg, "TIMER70_MESSAGE"))
                // do something
                fsmState++;
            else
                // handle error
            break;
    }*/

    return 0;

}
