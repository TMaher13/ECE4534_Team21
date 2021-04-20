#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
/* POSIX Header files */
#include <pthread.h>
#include <math.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <ti/drivers/GPIO.h>
#include <ti_drivers_config.h>
//#include <ti/drivers/utils/List.h>

#include <linkedlist.h>

#include <uart_term.h>
#include <queue_structs.h>

extern BaseType_t readQueue(QueueHandle_t handle, const void *data);
extern BaseType_t writeQueue(QueueHandle_t handle, const void *data);

extern QueueHandle_t nav_handle;
extern QueueHandle_t mqtt_handle;

enum navState {IDLE, SIMPLE_DIST, RAND_WALK, PATH_PLANNING};

/*typedef struct map_node {
    List_Elem elem;
    uint16_t index;
    float distance;
} mapNode;*/



// 10x10 map of the floor, each tile is 0.2x0.2 meters (200x200 millimeters)
float floorMap[100];
uint16_t scanData[LIDAR_DEGREES];

void rebuildMap(struct imageDetection obst, struct imageDetection targ) {
    struct mqttQueueStruct publish;
    //BaseType_t writeRet;
    snprintf(publish.topic, TOPIC_SIZE, "map");

    int placeIndex;
    int_least8_t delt_x, delt_y;
    uint16_t i; //, alt_i;

    float x=0.0, y=0.0;

    for(i=0; i<100; ++i) // Initialize map
        floorMap[i] = 0;

    for(i=0; i<360; ++i) {
        if(scanData[i] == 0 || scanData[i] > 2000)
            continue;

        placeIndex = 55;

        x = scanData[i] * cos(i * 3.14/180 + 3.14/2);
        y = scanData[i] * sin(i * 3.14/180 + 3.14/2);

        delt_x = (x / 200);
        delt_y = (int_least8_t)(y / 200) * 10; // smh

        placeIndex = placeIndex + delt_x - delt_y;

        if(i<36 || i>324) {
            // Check if a target or obstacle
            if( i == targ.angle && (targ.location == IN_FRONT || targ.location == ONLY_OBJ) )
                floorMap[placeIndex] = 5;
            else if( i == targ.angle && (targ.location == IN_BACK) ) {
                if(placeIndex-20 >= 0)
                    floorMap[placeIndex-20] = 5;
                else if(placeIndex-10 >= 0)
                    floorMap[placeIndex-10] = 5;
            }
            else if( i == obst.angle && (obst.location == IN_FRONT || obst.location == ONLY_OBJ) )
                floorMap[placeIndex] = -10;
            else if(floorMap[placeIndex] <= 0)
                floorMap[placeIndex] = -10;
        }
        else
            floorMap[placeIndex] = -10; // just a wall/object in the room
    }
    floorMap[55] = 0; // rover in the middle

    /*uint_least8_t counter = 0;
    for(i=0; i<100; ++i) {
        counter++;
        UART_PRINT("%d\t", floorMap[i]);
        if(counter == 10) {
            UART_PRINT("\r\n");
            counter = 0;
        }
    }*/
    /*for(i=0; i<100; ++i) {
        //UART_PRINT("Writing to mqtt\r\n");
        memset(publish.payload, 0, PAYLOAD_SIZE);
        snprintf(publish.payload, PAYLOAD_SIZE, "%f", floorMap[i]);

        //send to publish queue
        writeRet = writeQueue(mqtt_handle, &publish);
        if(writeRet == errQUEUE_FULL)
            UART_PRINT("MQTT queue full\r\n");
    }*/
}


// Convert from <x,y> to the index in our int[100] array
uint16_t convertToArray(uint16_t x, uint16_t y) {
    return y*10 + x;
}

// Convert from index of int[100] to <x,y>
void convertToXY(uint16_t index, uint16_t *x, uint16_t *y) {
    *y = index / 10;
    *x = index - (*y) * 10;
}

float EuclideanDist(uint16_t index1, uint16_t index2) {
    uint16_t x1, x2, y1, y2;

    convertToXY(index1, &x1, &y1);
    convertToXY(index2, &x2, &y2);

    //UART_PRINT("Euclidean distance p1: %d %d\r\n", x1, y1);
    //UART_PRINT("Euclidean distance p2: %d %d\r\n|-> %f\r\n", x2, y2, sqrt( pow((double)x1-(double)x2, 2) + pow((double)y1-(double)y2, 2) ) );
    return sqrt( pow((float)x1-(float)x2, 2) + pow((float)y1-(float)y2, 2) );
}


/*
 * Get the neighbors adjacent to the given index in our 10x10 map
 *
 */
void getNeighbors(uint16_t index, int16_t neighbors[8]) {
    uint_least8_t i;

    // Initialize to no neighbors
    for(i=0; i<8; i++)
        neighbors[i] = -1;

    // Neighbors in front of index
    if( (index - 11) >= 0)
        neighbors[0] = index-11;
    if( (index-10) >= 0)
        neighbors[1] = index-10;
    if( (index-9) >= 0)
        neighbors[2] = index-9;

    // Neighbors to the sides of index
    if( (index-1) >= 0)
        neighbors[3] = index-1;
    if( (index+1) < 100)
        neighbors[4] = index+1;

    // Neighbors behind index
    if( (index+9) < 100)
        neighbors[5] = index+9;
    if( (index+10) < 100)
        neighbors[6] = index+10;
    if( (index+11) < 100)
        neighbors[7] = index+11;
}


void WavePropagationSearch( int16_t path[20] ) {
    List *nodes, *newNodes;
    //List_Elem *tempElem;
    mapNode node, *tempNode;
    uint16_t i, j, targetIndex;
    int16_t neighbors[8];
    int_least8_t currNewNodes[100], lenNewNodes = 0;

    struct mqttQueueStruct publish;
    BaseType_t writeRet;
    snprintf(publish.topic, TOPIC_SIZE, "map");

    for(i=0; i<100; i++)
        if(floorMap[i] == 5)
            node.index = i;
    targetIndex = node.index;
    UART_PRINT("Target index: %d\r\n", targetIndex);
    //node.distance = 0.9;

    // Initialize list of nodes
    //List_clearList(&newNodes);
    //List_clearList(&nodes);
    //List_put(&nodes, (List_Elem *)&node);
    nodes = List_makeList();
    newNodes = List_makeList();
    List_add_back(targetIndex, 0.8, nodes);

    UART_PRINT("Generating map values\r\n");
    while(!List_is_empty(nodes)) {

        UART_PRINT("Resetting new nodes\r\n");
        // Initialize list of newNodes
        /*for (tempElem = List_tail(&newNodes); tempElem != NULL; tempElem = List_tail(&newNodes)) {
            List_remove(&newNodes, tempElem);
        }
        List_clearList(&newNodes);*/
        List_destroy(newNodes);

        lenNewNodes = 0;
        for(i=0; i<100; ++i)
            currNewNodes[i] = -1;

        // Iterate over discovered nodes
        tempNode = nodes->head;
        while(tempNode != NULL) {
            UART_PRINT("Looking at node %d\r\n", tempNode->index);
        //for (tempElem = List_head(&nodes); tempElem != NULL; tempElem = List_next(tempElem)) {
             //tempNode = (mapNode *)tempElem;
             if(tempNode->index > 99)
                 break;

             floorMap[tempNode->index] = tempNode->distance;

             getNeighbors(tempNode->index, neighbors);
             //mapNode neighborNode[8];
             uint_least8_t repeat = 0;
             for(i=0; i<8; ++i) {
                 if(neighbors[i] != -1) {
                     if(floorMap[neighbors[i]] < 0.5 && floorMap[neighbors[i]] > -0.5) { // undiscovered node
                         // Check if already in list
                         for(j=0; j<lenNewNodes; ++j) {
                             if(currNewNodes[j] == neighbors[i]) {
                                 //continue; // This just continues in current for loop
                                 UART_PRINT("Found a repeat: %d\r\n", neighbors[i]);
                                 repeat = 1;
                             }
                         }
                         if(repeat == 1) {
                             repeat = 0;
                             continue;
                         }
                         UART_PRINT("Adding new node: %d - %f\r\n", neighbors[i], EuclideanDist(neighbors[i], targetIndex));
                         List_add_back(neighbors[i], EuclideanDist(neighbors[i], targetIndex), newNodes);
                         currNewNodes[lenNewNodes] = neighbors[i];
                         lenNewNodes++;
                     }
                 }
             }
             tempNode = tempNode->next;
        }

        UART_PRINT("Deleting list and adding new nodes\r\n");
        // clear the nodes and replace with our current newNodes list
        List_destroy(nodes);
        tempNode = newNodes->head;
        while(tempNode != NULL) {
            UART_PRINT("Adding to node  list: %d , %f\r\n", tempNode->index, tempNode->distance);
            List_add_back(tempNode->index, tempNode->distance, nodes);
            tempNode = tempNode->next;
        }
        //UART_PRINT("Fail 3\r\n");
    }

    floorMap[targetIndex] = 0.8;

    UART_PRINT("Building path\r\n");
    // Traversing map for best path forward
    uint_least8_t currLoc, noPath=0, pathIndex=0;
    int_least8_t bestNeighbor;
    float bestNeighborVal;
    for(i=0; i<20; ++i)
        path[i] = -1;
    path[0] = 55; pathIndex++;
    currLoc = 55;

    while(currLoc != targetIndex && !noPath) {
        bestNeighborVal = 100.0;
        getNeighbors(currLoc, neighbors);

        //UART_PRINT("Neighbors: \r\n");
        for(i=0; i<8; ++i) {
            //UART_PRINT("%d: %d\r\n", neighbors[i], floorMap[neighbors[i]]);
            if(floorMap[neighbors[i]] < bestNeighborVal && floorMap[neighbors[i]] > 0) {
                bestNeighbor = neighbors[i];
                bestNeighborVal = floorMap[neighbors[i]];
            }
        }
        //UART_PRINT("Best neighbor for %d: %d\r\n", currLoc, bestNeighbor);

        if(bestNeighbor == -1 || pathIndex == 20) // Fail to find path
            noPath = 1;

        currLoc = bestNeighbor;
        path[pathIndex] = bestNeighbor;
        pathIndex++;
    }

    /*uint_least8_t counter = 0;
    for(i=0; i<100; ++i) {
        counter++;
        UART_PRINT("%.2f\t", floorMap[i]);
        if(counter == 10) {
            UART_PRINT("\r\n");
            counter = 0;
        }
    }*/
    for(i=0; i<100; ++i) {
        //UART_PRINT("Writing to mqtt\r\n");
        memset(publish.payload, 0, PAYLOAD_SIZE);
        snprintf(publish.payload, PAYLOAD_SIZE, "%.2f", floorMap[i]);

        //send to publish queue
        writeRet = writeQueue(mqtt_handle, &publish);
        if(writeRet == errQUEUE_FULL)
            UART_PRINT("MQTT queue full\r\n");
    }
    UART_PRINT("PATH:\r\n");
    for(i=0; i<20; ++i)
        UART_PRINT("%d ", path[i]);
    UART_PRINT("\r\nCompleted path planning\r\n");
}


void buildPath(int16_t path[20]) {
    struct mqttQueueStruct publish;
    snprintf(publish.topic, TOPIC_SIZE, "rover_directions");

    uint16_t distToMove=0, currPathAngle=0; // Distance moved will be in
    int16_t angleToMove=0;                                  // increments of 200 or 282 (sqrt(2) * 200)
    uint_least8_t i=0, complete=0;

    while(path[i] != -1 && i < 19) {
        if(path[i+2] == -1) {
            complete = 1;
        }
        if(path[i+1] - path[i] == -11) { // Moving in direction: -45 degrees
            if(currPathAngle == 315) {
                distToMove += 282;
            }
            else if(currPathAngle == 270) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 282;
            }
            else if(currPathAngle == 225) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 282;
            }
            else if(currPathAngle == 45) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 282;
            }
            else if(currPathAngle == 0) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 282;
            }
            currPathAngle = 315;
        }
        else if(path[i+1] - path[i] == -10) { // Moving in direction: 0 degrees
            if(currPathAngle == 0) {
                distToMove += 200;
            }
            else if(currPathAngle == 315) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 200;
            }
            else if(currPathAngle == 270) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 200;
            }
            else if(currPathAngle == 90) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 200;
            }
            else if(currPathAngle == 45) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 200;
            }
            currPathAngle = 0;
        }
        else if(path[i+1] - path[i] == -9) { // Moving in direction: 45 degrees
            if(currPathAngle == 45) {
                distToMove += 282;
            }
            else if(currPathAngle == 315) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 282;
            }
            else if(currPathAngle == 135) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 282;
            }
            else if(currPathAngle == 90) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 282;
            }
            else if(currPathAngle == 0) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 282;
            }
            currPathAngle = 45;
        }
        else if(path[i+1] - path[i] == -1) { // Moving in direction: -90 degrees
            if(currPathAngle == 270) {
                distToMove += 200;
            }
            else if(currPathAngle == 315) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 200;
            }
            else if(currPathAngle == 225) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 200;
            }
            else if(currPathAngle == 180) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 200;
            }
            else if(currPathAngle == 0) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 200;
            }
            currPathAngle = 270;
        }
        else if(path[i+1] - path[i] == 1) { // Moving in direction: 90 degrees
            if(currPathAngle == 90) {
                distToMove += 200;
            }
            else if(currPathAngle == 180) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 200;
            }
            else if(currPathAngle == 135) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 200;
            }
            else if(currPathAngle == 45) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 200;
            }
            else if(currPathAngle == 0) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 200;
            }
            currPathAngle = 90;
        }
        else if(path[i+1] - path[i] == 9) { // Moving in direction: -135 degrees
            if(currPathAngle == 225) {
                distToMove += 282;
            }
            else if(currPathAngle == 315) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 282;
            }
            else if(currPathAngle == 270) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 282;
            }
            else if(currPathAngle == 180) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 282;
            }
            else if(currPathAngle == 135) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 282;
            }
            currPathAngle = 225;
        }
        else if(path[i+1] - path[i] == 10) { // Moving in direction: 180 degrees
            if(currPathAngle == 180) {
                distToMove += 200;
            }
            else if(currPathAngle == 270) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 200;
            }
            else if(currPathAngle == 225) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 200;
            }
            else if(currPathAngle == 135) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 200;
            }
            else if(currPathAngle == 90) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 200;
            }
            currPathAngle = 180;
        }
        else if(path[i+1] - path[i] == 11) { // Moving in direction: 135 degrees
            if(currPathAngle == 135) {
                distToMove += 282;
            }
            else if(currPathAngle == 225) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -90;
                distToMove = 282;
            }
            else if(currPathAngle == 180) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = -45;
                distToMove = 282;
            }
            else if(currPathAngle == 90) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 45;
                distToMove = 282;
            }
            else if(currPathAngle == 45) {
                memset(publish.payload, 0, PAYLOAD_SIZE);
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"0\"}",
                         angleToMove,
                         distToMove);
                if(distToMove != 0)
                    while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

                angleToMove = 90;
                distToMove = 282;
            }
            currPathAngle = 135;
        }

        if(complete) {
            memset(publish.payload, 0, PAYLOAD_SIZE);
            snprintf(publish.payload, PAYLOAD_SIZE,
                     "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"1\"}",
                     angleToMove,
                     distToMove);
            while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;

            return;
        }

        i++;
    }
}


void *navigationThread(void *arg0) {
    struct mqttQueueStruct publish;
    struct navQueueStruct navData;
    BaseType_t readRet, writeRet;

    //TickType_t lastTarget;

    snprintf(publish.topic, TOPIC_SIZE, "map");

    uint16_t i, j, direction, distTravel, furthestDir, currDir, numPoints; // For rand walk
    int16_t path[20];

    struct imageDetection currObstacle = {-1, 0, 0};
    struct imageDetection currTarget = {-1, 0, 0};

    enum navState roverState = IDLE;


    for(;;) {

        readRet = readQueue(nav_handle, &navData);

        if(readRet == pdTRUE) {
            switch(navData.messageType) {
                case LIDAR_MESSAGE:
                    //lastLidar = navData.tickTime;
                    memcpy(scanData, navData.scanData, sizeof(uint16_t)*LIDAR_DEGREES);
                    break;
                case CAMERA_MESSAGE:
                    //lastCamera = navData.tickTime;
                    if(navData.imgDet.object == IMG_TARGET) {
                        //lastTarget = xTaskGetTickCount();
                        currTarget.object = navData.imgDet.object;
                        if(navData.imgDet.angle >= 0)
                            currTarget.angle = navData.imgDet.angle;
                        else {
                            UART_PRINT("%d - %d\r\n", LIDAR_DEGREES, navData.imgDet.angle);
                            currTarget.angle = 360 + navData.imgDet.angle; // Plus since it's a negative
                        }

                        currTarget.location = navData.imgDet.location;
                        //roverState = PATH_PLANNING;
                        UART_PRINT("Target received: %d %d %d\r\n", currTarget.object,
                                                                    currTarget.angle,
                                                                    currTarget.location);
                    }
                    else if(navData.imgDet.object == IMG_OBSTACLE) {
                        currObstacle.object = navData.imgDet.object;
                        currObstacle.angle = navData.imgDet.angle;
                        currObstacle.location = navData.imgDet.location;

                        UART_PRINT("Obstacle received: %d %d %d\r\n", currObstacle.object,
                                                                       currObstacle.angle,
                                                                       currObstacle.location);
                    }
                    break;
                case REQUEST_MESSAGE:
                    if(navData.request == SIMPLE_DIST_REQUEST) {
                        roverState = SIMPLE_DIST;
                    }
                    else if(navData.request == ROVER_PATH_REQUEST) {
                        //UART_PRINT("Received path request %d\r\n", currTarget.object);
                        if(currTarget.object == IMG_TARGET) {
                            roverState = PATH_PLANNING;
                            //UART_PRINT("Moving to path planning\r\n");
                        }
                        else {
                            roverState = RAND_WALK;
                            //UART_PRINT("Moving to random walk\r\n");
                        }
                    }
                    else {
                        // Unexpected request
                    }
            }

        }

        switch(roverState) {
            case IDLE:
                //currObstacle.object = -1;
                //currTarget.object = -1;
                break;
            case SIMPLE_DIST:
                i=0;
                while(scanData[i] == 0)
                    i++;
                UART_PRINT("Getting distance in front at angle %d degrees", i);

                snprintf(publish.topic, TOPIC_SIZE, "roverMove");
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"0\",\"distance\":\"%d\",\"complete\":\"1\"}",
                         scanData[0] - 50);

                writeRet = writeQueue(mqtt_handle, &publish);
                if (writeRet == errQUEUE_FULL){
                    //DEBUG_EVENT
                }

                roverState = IDLE; // Switch back to IDLE once distance required is returned
                break;
            case RAND_WALK:
                UART_PRINT("Entered random walk\r\n");

                furthestDir = 0;
                for(i=0; i<360; i+=20) {

                    numPoints = 0;
                    currDir = 0;
                    for(j=i; j<i+20; j++) {
                        if(scanData[j] != 0 && scanData[j] < 4000) {
                            numPoints++;
                            currDir += scanData[j];
                        }
                    }
                    currDir = currDir / numPoints;
                    if(currDir > furthestDir) {
                        furthestDir = currDir;
                        direction = i + 10;
                        UART_PRINT("New furthest area at angle %d: %d %d: %d\r\n", direction, currDir, numPoints, furthestDir);
                     }
                }
                distTravel = furthestDir / 4;
                UART_PRINT("Direction to walk: %d degrees for %dmm\n", direction, distTravel);

                snprintf(publish.topic, TOPIC_SIZE, "roverMove");
                snprintf(publish.payload, PAYLOAD_SIZE,
                         "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"1\"}",
                         direction,
                         distTravel);

                writeRet = writeQueue(mqtt_handle, &publish);
                if (writeRet == errQUEUE_FULL){
                    //DEBUG_EVENT
                }
                roverState = IDLE;
                break;
            case PATH_PLANNING:
                //UART_PRINT("Path planning: %d %d\r\n", currTarget.object, currTarget.location);
                if(currTarget.object == IMG_TARGET) {
                    if(currTarget.location == IN_FRONT || currTarget.location == ONLY_OBJ) {
                        UART_PRINT("Creating direct path to target\r\n");

                        i=currTarget.angle;
                        while(scanData[i] == 0)
                            i++;

                        memset(publish.topic, 0, TOPIC_SIZE);
                        memset(publish.payload, 0, PAYLOAD_SIZE);
                        snprintf(publish.topic, TOPIC_SIZE, "roverMove");
                        snprintf(publish.payload, PAYLOAD_SIZE,
                                 "{\"angle\":\"%d\",\"distance\":\"%d\",\"complete\":\"1\"}",
                                 currTarget.angle,
                                 scanData[i] - 50); // scanData[currTarget.angle]

                        while(writeQueue(mqtt_handle, &publish) == errQUEUE_FULL) ;
                    }
                    else {
                        rebuildMap(currObstacle, currTarget);

                        UART_PRINT("Calculating optimal path\r\n");
                        WavePropagationSearch(path);

                        UART_PRINT("Building path\r\n");
                        if(path[19] != -1)
                            continue;

                        buildPath(path);
                    }

                    roverState = IDLE;
                }
                else {
                    roverState = RAND_WALK;
                }

                break;
        }
    }
}


int createNavigationThread(int threadStackSize, int prio) {

    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;


    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = prio;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, threadStackSize);
    if (retc != 0) {
        //fatalError(SENSOR_STACK_FATAL_ERROR);
        return -1; // Stack initialization failed
    }

    retc = pthread_create(&thread, &attrs, navigationThread, NULL);
    if (retc != 0) {
        //fatalError(SENSOR_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}
