#include "DCCEXParser.h"
#include "CommManager.h"
#include "../Accessories/Turnouts.h"
#include "../Accessories/Sensors.h"
#include "../Accessories/Outputs.h"
#include "../Accessories/EEStore.h"
#include "../CommandStation.h"
#include <inttypes.h>

DCC* DCCEXParser::mainTrack;
DCC* DCCEXParser::progTrack;

int DCCEXParser::p[MAX_PARAMS];

void DCCEXParser::init(DCC* mainTrack_, DCC* progTrack_) {
    mainTrack = mainTrack_;
    progTrack = progTrack_;
} 

int DCCEXParser::stringParser(const char *com, int result[]) {
    byte state=1;
    byte parameterCount=0;
    int runningValue=0;
    const char * remainingCmd=com;  // skips the opcode
    bool signNegative=false;
    
    // clear all parameters in case not enough found
    for (int i=0;i<MAX_PARAMS;i++) result[i]=0;
    
    while(parameterCount<MAX_PARAMS) {
        char hot=*remainingCmd;
        switch (state) {
        case 1: // skipping spaces before a param
            if (hot==' ') break;
            if (hot == '\0' || hot=='>') return parameterCount;
            state=2;
            continue;
        case 2: // checking sign
            signNegative=false;
            runningValue=0;
            state=3; 
            if (hot!='-') continue; 
            signNegative=true;
            break; 
        case 3: // building a parameter   
            if (hot>='0' && hot<='9') {
                runningValue=10*runningValue+(hot-'0');
                break;
            }
            result[parameterCount] = runningValue * (signNegative ?-1:1);
            parameterCount++;
            state=1; 
            continue;
        }   
        remainingCmd++;
    }
    return parameterCount;
}

// See documentation on DCC class for info on this section
void DCCEXParser::parse(const char *com) {
    int numArgs = stringParser(com+1, p);
    
    switch(com[0]) {
    
/***** SET ENGINE THROTTLES USING 128-STEP SPEED CONTROL ****/

    case 't':       // <t REGISTER CAB SPEED DIRECTION>
        setThrottleResponse throttleResponse;

        mainTrack->setThrottle(p[0], p[1], p[2], p[3], throttleResponse);

        CommManager::printf(F("<T %d %d %d>"), throttleResponse.device, throttleResponse.speed, throttleResponse.direction);
        
        break;
    
/***** OPERATE ENGINE DECODER FUNCTIONS F0-F28 ****/

    case 'f':       // <f CAB BYTE1 [BYTE2]>
        setFunctionResponse functionResponse;
        
        if(numArgs == 2)
            mainTrack->setFunction(p[0], p[1], functionResponse);
        else 
            mainTrack->setFunction(p[0], p[1], p[2], functionResponse);
        
        // TODO use response?
        
        break;

/***** OPERATE STATIONARY ACCESSORY DECODERS  ****/

    case 'a':       // <a ADDRESS SUBADDRESS ACTIVATE>        
        setAccessoryResponse accessoryResponse;

        mainTrack->setAccessory(p[0], p[1], p[2], accessoryResponse);
        
        break;
    
/***** CREATE/EDIT/REMOVE/SHOW & OPERATE A TURN-OUT  ****/

    case 'T':       // <T ID THROW>      
        Turnout *t;

        switch(numArgs){

        case 2:                     // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
            t=Turnout::get(p[0]);
            if(t!=NULL)
                t->activate(p[1], (DCC*) mainTrack);
            else
                CommManager::printf(F("<X>"));
            break;

        case 3:                     // argument is string with id number of turnout followed by an address and subAddress
            Turnout::create(p[0],p[1],p[2],1);
            break;

        case 1:                     // argument is a string with id number only
            Turnout::remove(p[0]);
            break;

        case 0:                    // no arguments
            Turnout::show(1);                  // verbose show
            break;
        }
        
        break;
    
/***** CREATE/EDIT/REMOVE/SHOW & OPERATE AN OUTPUT PIN  ****/

    case 'Z':       // <Z ID ACTIVATE>
        Output* o;

        switch(numArgs){

        case 2:                     // argument is string with id number of output followed by zero (LOW) or one (HIGH)
            o=Output::get(p[0]);
            if(o!=NULL)
                o->activate(p[1]);
            else
                CommManager::printf(F("<X>"));
            break;

        case 3:                     // argument is string with id number of output followed by a pin number and invert flag
            Output::create(p[0],p[1],p[2],1);
            break;

        case 1:                     // argument is a string with id number only
            Output::remove(p[0]);
            break;

        case 0:                    // no arguments
            Output::show(1);                  // verbose show
            break;
        }
        
        break;
    
/***** CREATE/EDIT/REMOVE/SHOW A SENSOR  ****/

    case 'S':
        switch(numArgs){

        case 3:                     // argument is string with id number of sensor followed by a pin number and pullUp indicator (0=LOW/1=HIGH)
            Sensor::create(p[0],p[1],p[2],1);
            break;

        case 1:                     // argument is a string with id number only
            Sensor::remove(p[0]);
            break;

        case 0:                    // no arguments
            Sensor::show();
            break;

        case 2:                     // invalid number of arguments
            CommManager::printf(F("<X>"));
            break;
        }
    
        break;

/***** SHOW STATUS OF ALL SENSORS ****/

    case 'Q':         // <Q>
        /*
        *    returns: the status of each sensor ID in the form <Q ID> (active) or <q ID> (not active)
        */
        Sensor::status();
        break;


/***** WRITE CONFIGURATION VARIABLE BYTE TO ENGINE DECODER ON MAIN OPERATIONS TRACK  ****/

    case 'w':      // <w CAB CV VALUE>
        writeCVByteMainResponse wresponse;

        mainTrack->writeCVByteMain(p[0], p[1], p[2], wresponse);
        
        break;

/***** WRITE CONFIGURATION VARIABLE BIT TO ENGINE DECODER ON MAIN OPERATIONS TRACK  ****/

    case 'b':      // <b CAB CV BIT VALUE>
        writeCVBitMainResponse bresponse;

        mainTrack->writeCVBitMain(p[0], p[1], p[2], p[3], bresponse);
        
        break;

/***** WRITE CONFIGURATION VARIABLE BYTE TO ENGINE DECODER ON PROGRAMMING TRACK  ****/

    case 'W':      // <W CV VALUE CALLBACKNUM CALLBACKSUB>

        progTrack->writeCVByte(p[0], p[1], p[2], p[3]);

        break;

/***** WRITE CONFIGURATION VARIABLE BIT TO ENGINE DECODER ON PROGRAMMING TRACK  ****/

    case 'B':      // <B CV BIT VALUE CALLBACKNUM CALLBACKSUB>
        
        progTrack->writeCVBit(p[0], p[1], p[2], p[3], p[4]);
        
        break;

/***** READ CONFIGURATION VARIABLE BYTE FROM ENGINE DECODER ON PROGRAMMING TRACK  ****/

    case 'R':     // <R CV CALLBACKNUM CALLBACKSUB>        
        
        progTrack->readCV(p[0], p[1], p[2]);
    
        break;

/***** TURN ON POWER FROM MOTOR SHIELD TO TRACKS  ****/

    case '1':      // <1>
        mainTrack->hdw.setPower(true);
        progTrack->hdw.setPower(true);
        CommManager::printf(F("<p1>"));
        break;

/***** TURN OFF POWER FROM MOTOR SHIELD TO TRACKS  ****/

    case '0':     // <0>
        mainTrack->hdw.setPower(false);
        progTrack->hdw.setPower(false);
        CommManager::printf(F("<p0>"));
        break;

/***** READ MAIN OPERATIONS TRACK CURRENT  ****/

    case 'c':     // <c>

        // Todo: figure out the scale that JMRI is actually using and scale accordingly
        int currRead;
        currRead = mainTrack->hdw.getLastRead();
        CommManager::printf(F("<a %d>"), currRead);
        break;

/***** READ STATUS OF DCC++ BASE STATION  ****/

    case 's':      // <s>
        CommManager::printf(F("<p%d MAIN>"), mainTrack->hdw.getStatus());
        CommManager::printf(F("<p%d PROG>"), progTrack->hdw.getStatus());
        for(int i=1;i<=mainTrack->numDev;i++){
            if(mainTrack->speedTable[i].speed==0)
            continue;
            CommManager::printf("<T%d %d %d>", i, mainTrack->speedTable[i].speed, mainTrack->speedTable[i].forward);
        }
        CommManager::printf(F("<iDCC++ BASE STATION FOR ARDUINO %s / %s: V-%s / %s %s>"), "Command Station", BOARD_NAME, VERSION, __DATE__, __TIME__);
        CommManager::showInitInfo();
        Turnout::show();
        Output::show();

        break;

/***** STORE SETTINGS IN EEPROM  ****/

    case 'E':     // <E>
        EEStore::store();
        CommManager::printf(F("<e %d %d %d>"), EEStore::eeStore->data.nTurnouts, EEStore::eeStore->data.nSensors, EEStore::eeStore->data.nOutputs);
        break;

/***** CLEAR SETTINGS IN EEPROM  ****/

    case 'e':     // <e>

        EEStore::clear();
        CommManager::printf(F("<O>"));
        break;

/***** PRINT CARRIAGE RETURN IN SERIAL MONITOR WINDOW  ****/

    case ' ':     // < >
        CommManager::printf(F("\n"));
        break;
    }
}