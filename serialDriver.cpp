//todo doxygen

#include "serialDriver.h"

/* Serial Driver Message IDs. */
enum SdFnIds {
    SERIAL_DRIVER_ID_ENGINE_DATA = 0x0,
    SERIAL_DRIVER_ID_STATUS = 0x1,
    SERIAL_DRIVER_ID_MAX,
};

/* Rx Callback. */
void onPacketReceived(const uint8_t* buffer, size_t size);
/* Define Rx functions. */
void rxHandleUnimplemented(const uint8_t* payload, size_t size);
void rxHandleEngineData(const uint8_t* payload, size_t size);

/* Define structure of Rx functions. */
typedef void SdRxFunc(const uint8_t* payload, size_t size);

/* Rx Function Lookup table entry. */
typedef struct SdRxFnEntry {
    SdFnIds id;         /* Function ID. */
    SdRxFunc *func;     /* Pointer to Rx Function to be called. */
} SdRxFnEntry;

/* Rx Function lookup table. */
const SdRxFnEntry rxFunctionLookupTable[SERIAL_DRIVER_ID_MAX] = {
    {SERIAL_DRIVER_ID_ENGINE_DATA, rxHandleEngineData},
    {SERIAL_DRIVER_ID_STATUS, rxHandleUnimplemented},
};


/*!
 * @brief Default Constructor.
 */
SerialDriver::SerialDriver(Stream *serialPort) {
    /* Validate serialPort object. */
    if (serialPort == nullptr) {
        _serialP->print("Error: Bad Serial pointer!");
        while (1);
    }

    _serialP = serialPort;

    /* Initialize PacketSerial. */
    _packetSerial.setStream(serialPort);
    _packetSerial.setPacketHandler(&onPacketReceived);
}

//todo doxygen
Errors SerialDriver::loopReadMsgs() {
    _packetSerial.update();
    
    if (_packetSerial.overflow()) {
        //todo handle
        _serialP->println("WARNING: Serial buffer overflowed!");
    }
}

Errors SerialDriver::sendMessage(uint8_t id, const uint8_t* payload, size_t size) {
    const uint8_t* message = nullptr;
    size_t msgSize = 0;

    /* Validate inputs. */
    if (payload == nullptr) {
        return;
    }
    /* Every message should include an ID and a payload of at least 1 byte. */
    if (size < 1) {
        return;
    }
    /* Check if the ID byte is valid. */
    if (id >= SERIAL_DRIVER_ID_MAX) {
        return;
    }

    /* Prepend the ID to our payload data by storing in a message buffer. */
    msgSize = ++size;
    message = (const uint8_t*)malloc(msgSize * sizeof(uint8_t));
    memcpy(message, &id, sizeof(id));
    memcpy((message + 1), payload, size*sizeof(uint8_t));

    /* Send the message. */
    _packetSerial.send(message, msgSize);
}



//todo doxygen
void onPacketReceived(const uint8_t* buffer, size_t size) {
    SdFnIds id = SERIAL_DRIVER_ID_MAX;
    const uint8_t* payload = nullptr;
    size_t payloadSize = 0;

    /* Validate inputs. */
    if (buffer == nullptr) {
        return;
    }
    /* Every message should include an ID and a payload of at least 1 byte. */
    if (size < 2) {
        return;
    }
    /* Check if the ID byte is valid. */
    id = (SdFnIds)buffer[0];
    if (id >= SERIAL_DRIVER_ID_MAX) {
        return;
    }

    /* ID is valid and payload should exist, extract payload from buffer. */
    payload = buffer + 1;
    payloadSize = --size;

    /* Look through our table for the corresponding function ID. */
    for (int i = 0; i < SERIAL_DRIVER_ID_MAX; i++) {
        /* Check if the IDs match. */
        if(rxFunctionLookupTable[i].id == id) {
            /* Call the function and return. */
            rxFunctionLookupTable[i].func(payload, payloadSize);
            return;
        }
    }

    return;
}


/* 
 * Serial Driver Rx Functions
 */

/*!
 * @brief Does nothing. Used for any actions that aren't implemented.
 *
 * @param payload   Data payload from the packet.
 * @param size      Length of the payload array.
 * 
 * @result  Does nothing.
 */
void rxHandleUnimplemented(const uint8_t* payload, size_t size) {
    return;
}

void rxHandleEngineData(const uint8_t* payload, size_t size) {
    return;
}