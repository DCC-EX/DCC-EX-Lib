#include "JMRIParser.h"
#include "CommManager.h"
#include "../Accessories/Turnouts.h"
#include "../Accessories/Sensors.h"
#include "../Accessories/Outputs.h"
#include "../Accessories/EEStore.h"
#include "../CommandStation.h"
#include <inttypes.h>

DCC* JMRIParser::mainTrack;
DCC* JMRIParser::progTrack;

int JMRIParser::p[MAX_PARAMS];

void JMRIParser::init(DCC* mainTrack_, DCC* progTrack_) {
    mainTrack = mainTrack_;
    progTrack = progTrack_;
} 

int JMRIParser::stringParser(const char *com, int result[]) {
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
void JMRIParser::parse(const char *com) {
    int numArgs = stringParser(com+1, p);
    
    switch(com[0]) {
    
/***** SET ENGINE THROTTLES USING 128-STEP SPEED CONTROL ****/

    case 't':       // <t REGISTER CAB SPEED DIRECTION>
        setThrottleResponse throttleResponse;

        mainTrack->setThrottle(p[0], p[1], p[2], p[3], throttleResponse);

        CommManager::printf("<T %d %d %d>", throttleResponse.device, throttleResponse.speed, throttleResponse.direction);
        
        break;
    
/***** OPERATE ENGINE DECODER FUNCTIONS F0-F28 ****/

    case 'f':       // <f CAB BYTE1 [BYTE2]>
        setFunctionResponse functionResponse;
        
        if(numArgs == 2)
            mainTrack->setFunction(p[0], p[1], functionResponse);
        else 
            mainTrack->setFunction(p[0], p[1], p[2], functionResponse);
        
        break;

/***** OPERATE STATIONARY ACCESSORY DECODERS  ****/

    case 'a':       // <a ADDRESS SUBADDRESS ACTIVATE>
        /*
        *    turns an accessory (stationary) decoder on or off
        *
        *    ADDRESS:  the primary address of the decoder (0-511)
        *    SUBADDRESS: the subaddress of the decoder (0-3)
        *    ACTIVATE: 1=on (set), 0=off (clear)
        *
        *    Note that many decoders and controllers combine the ADDRESS and SUBADDRESS into a single number, N,
        *    from  1 through a max of 2044, where
        *
        *    N = (ADDRESS - 1) * 4 + SUBADDRESS + 1, for all ADDRESS>0
        *
        *    OR
        *
        *    ADDRESS = INT((N - 1) / 4) + 1
        *    SUBADDRESS = (N - 1) % 4
        *
        *    returns: NONE
        */
        
        setAccessoryResponse accessoryResponse;

        mainTrack->setAccessory(p[0], p[1], p[2], accessoryResponse);
        
        break;
    
/***** CREATE/EDIT/REMOVE/SHOW & OPERATE A TURN-OUT  ****/

    case 'T':       // <T ID THROW>
        /*
        *   <T ID THROW>:                sets turnout ID to either the "thrown" or "unthrown" position
        *
        *   ID: the numeric ID (0-32767) of the turnout to control
        *   THROW: 0 (unthrown) or 1 (thrown)
        *
        *   returns: <H ID THROW> or <X> if turnout ID does not exist
        *
        *   *** SEE ACCESSORIES.CPP FOR COMPLETE INFO ON THE DIFFERENT VARIATIONS OF THE "T" COMMAND
        *   USED TO CREATE/EDIT/REMOVE/SHOW TURNOUT DEFINITIONS
        */
        
        Turnout *t;

        switch(numArgs){

        case 2:                     // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
            t=Turnout::get(p[0]);
            if(t!=NULL)
                t->activate(p[1], (DCC*) mainTrack);
            else
                CommManager::printf("<X>");
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
        /*
        *   <Z ID ACTIVATE>:          sets output ID to either the "active" or "inactive" state
        *
        *   ID: the numeric ID (0-32767) of the output to control
        *   ACTIVATE: 0 (active) or 1 (inactive)
        *
        *   returns: <Y ID ACTIVATE> or <X> if output ID does not exist
        *
        *   *** SEE OUTPUTS.CPP FOR COMPLETE INFO ON THE DIFFERENT VARIATIONS OF THE "O" COMMAND
        *   USED TO CREATE/EDIT/REMOVE/SHOW TURNOUT DEFINITIONS
        */
        
        Output* o;

        switch(numArgs){

        case 2:                     // argument is string with id number of output followed by zero (LOW) or one (HIGH)
            o=Output::get(p[0]);
            if(o!=NULL)
                o->activate(p[1]);
            else
                CommManager::printf("<X>");
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
            CommManager::printf("<X>");
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
        /*
        *    writes, without any verification, a Configuration Variable to the decoder of an engine on the main operations track
        *
        *    CAB:  the short (1-127) or long (128-10293) address of the engine decoder
        *    CV: the number of the Configuration Variable memory location in the decoder to write to (1-1024)
        *    VALUE: the value to be written to the Configuration Variable memory location (0-255)
        *
        *    returns: NONE
        */
       
        writeCVByteMainResponse wresponse;

        mainTrack->writeCVByteMain(p[0], p[1], p[2], wresponse);
        
        break;

/***** WRITE CONFIGURATION VARIABLE BIT TO ENGINE DECODER ON MAIN OPERATIONS TRACK  ****/

    case 'b':      // <b CAB CV BIT VALUE>
        /*
        *    writes, without any verification, a single bit within a Configuration Variable to the decoder of an engine on the main operations track
        *
        *    CAB:  the short (1-127) or long (128-10293) address of the engine decoder
        *    CV: the number of the Configuration Variable memory location in the decoder to write to (1-1024)
        *    BIT: the bit number of the Configurarion Variable regsiter to write (0-7)
        *    VALUE: the value of the bit to be written (0-1)
        *
        *    returns: NONE
        */
        writeCVBitMainResponse bresponse;

        mainTrack->writeCVBitMain(p[0], p[1], p[2], p[3], bresponse);
        
        break;

/***** WRITE CONFIGURATION VARIABLE BYTE TO ENGINE DECODER ON PROGRAMMING TRACK  ****/

    case 'W':      // <W CV VALUE CALLBACKNUM CALLBACKSUB>
        /*
        *    writes, and then verifies, a Configuration Variable to the decoder of an engine on the programming track
        *
        *    CV: the number of the Configuration Variable memory location in the decoder to write to (1-1024)
        *    VALUE: the value to be written to the Configuration Variable memory location (0-255)
        *    CALLBACKNUM: an arbitrary integer (0-32767) that is ignored by the Base Station and is simply echoed back in the output - useful for external programs that call this function
        *    CALLBACKSUB: a second arbitrary integer (0-32767) that is ignored by the Base Station and is simply echoed back in the output - useful for external programs (e.g. DCC++ Interface) that call this function
        *
        *    returns: <r CALLBACKNUM|CALLBACKSUB|CV Value)
        *    where VALUE is a number from 0-255 as read from the requested CV, or -1 if verificaiton read fails
        */

        writeCVByteResponse wcvresponse;

        progTrack->writeCVByte(p[0], p[1], p[2], p[3], wcvresponse);

        CommManager::printf("<r%d|%d|%d %d>", wcvresponse.callback, wcvresponse.callbackSub, wcvresponse.cv, wcvresponse.bValue);

        break;

/***** WRITE CONFIGURATION VARIABLE BIT TO ENGINE DECODER ON PROGRAMMING TRACK  ****/

    case 'B':      // <B CV BIT VALUE CALLBACKNUM CALLBACKSUB>
        /*
        *    writes, and then verifies, a single bit within a Configuration Variable to the decoder of an engine on the programming track
        *
        *    CV: the number of the Configuration Variable memory location in the decoder to write to (1-1024)
        *    BIT: the bit number of the Configurarion Variable memory location to write (0-7)
        *    VALUE: the value of the bit to be written (0-1)
        *    CALLBACKNUM: an arbitrary integer (0-32767) that is ignored by the Base Station and is simply echoed back in the output - useful for external programs that call this function
        *    CALLBACKSUB: a second arbitrary integer (0-32767) that is ignored by the Base Station and is simply echoed back in the output - useful for external programs (e.g. DCC++ Interface) that call this function
        *
        *    returns: <r CALLBACKNUM|CALLBACKSUB|CV BIT VALUE)
        *    where VALUE is a number from 0-1 as read from the requested CV bit, or -1 if verificaiton read fails
        */
        
        writeCVBitResponse Bresponse;

        progTrack->writeCVBit(p[0], p[1], p[2], p[3], p[4], Bresponse);

        CommManager::printf("<r%d|%d|%d %d %d>", Bresponse.callback, Bresponse.callbackSub, Bresponse.cv, Bresponse.bNum, Bresponse.bValue);
        
        break;

/***** READ CONFIGURATION VARIABLE BYTE FROM ENGINE DECODER ON PROGRAMMING TRACK  ****/

    case 'R':     // <R CV CALLBACKNUM CALLBACKSUB>
        /*
        *    reads a Configuration Variable from the decoder of an engine on the programming track
        *
        *    CV: the number of the Configuration Variable memory location in the decoder to read from (1-1024)
        *    CALLBACKNUM: an arbitrary integer (0-32767) that is ignored by the Base Station and is simply echoed back in the output - useful for external programs that call this function
        *    CALLBACKSUB: a second arbitrary integer (0-32767) that is ignored by the Base Station and is simply echoed back in the output - useful for external programs (e.g. DCC++ Interface) that call this function
        *
        *    returns: <r CALLBACKNUM|CALLBACKSUB|CV VALUE)
        *    where VALUE is a number from 0-255 as read from the requested CV, or -1 if read could not be verified
        */
        
        readCVResponse rcvresponse;
        
        progTrack->readCV(p[0], p[1], p[2], rcvresponse);

        CommManager::printf("<r%d|%d|%d %d>", rcvresponse.callback, rcvresponse.callbackSub, rcvresponse.cv, rcvresponse.bValue);
    
        break;

/***** TURN ON POWER FROM MOTOR SHIELD TO TRACKS  ****/

    case '1':      // <1>
        /*
        *    enables power from the motor shield to the main operations and programming tracks
        *
        *    returns: <p1>
        */
        mainTrack->hdw.setPower(true);
        progTrack->hdw.setPower(true);
        CommManager::printf("<p1>");
        break;

/***** TURN OFF POWER FROM MOTOR SHIELD TO TRACKS  ****/

    case '0':     // <0>
        /*
        *    disables power from the motor shield to the main operations and programming tracks
        *
        *    returns: <p0>
        */
        mainTrack->hdw.setPower(false);
        progTrack->hdw.setPower(false);
        CommManager::printf("<p0>");
        break;

/***** READ MAIN OPERATIONS TRACK CURRENT  ****/

    case 'c':     // <c>
        /*
        *    reads current being drawn on main operations track
        *
        *    returns: <a CURRENT>
        *    where CURRENT = 0-1024, based on exponentially-smoothed weighting scheme
        */

        // Todo: figure out the scale that JMRI is actually using and scale accordingly
        int currRead;
        currRead = mainTrack->hdw.getMilliamps();
        CommManager::printf("<a %d>", currRead);
        break;

/***** READ STATUS OF DCC++ BASE STATION  ****/

    case 's':      // <s>
        /*
        *    returns status messages containing track power status, throttle status, turn-out status, and a version number
        *    NOTE: this is very useful as a first command for an interface to send to this sketch in order to verify connectivity and update any GUI to reflect actual throttle and turn-out settings
        *
        *    returns: series of status messages that can be read by an interface to determine status of DCC++ Base Station and important settings
        */
        // mainTrack->showStatus();
        for(int i=1;i<=mainTrack->numDev;i++){
            if(mainTrack->speedTable[i].speed==0)
            continue;
            CommManager::printf("<T%d %d %d>", i, mainTrack->speedTable[i].speed, mainTrack->speedTable[i].forward);
        }
        CommManager::printf("<iDCC++ BASE STATION FOR ARDUINO %s / %s: V-%s / %s %s>", "Command Station", BOARD_NAME, VERSION, __DATE__, __TIME__);
        CommManager::showInitInfo();
        Turnout::show();
        Output::show();

        break;

/***** STORE SETTINGS IN EEPROM  ****/

    case 'E':     // <E>
        /*
        *    stores settings for turnouts and sensors EEPROM
        *
        *    returns: <e nTurnouts nSensors>
        */

        EEStore::store();
        CommManager::printf("<e %d %d %d>", EEStore::eeStore->data.nTurnouts, EEStore::eeStore->data.nSensors, EEStore::eeStore->data.nOutputs);
        break;

/***** CLEAR SETTINGS IN EEPROM  ****/

    case 'e':     // <e>
        /*
        *    clears settings for Turnouts in EEPROM
        *
        *    returns: <O>
        */

        EEStore::clear();
        CommManager::printf("<O>");
        break;

/***** PRINT CARRIAGE RETURN IN SERIAL MONITOR WINDOW  ****/

    case ' ':     // < >
        /*
        *    simply prints a carriage return - useful when interacting with Ardiuno through serial monitor window
        *
        *    returns: a carriage return
        */
        CommManager::printf("\n");
        break;
    }
}