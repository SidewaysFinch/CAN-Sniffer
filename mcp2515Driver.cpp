//todo doxygen

#include "mcp2515Driver.h"

/* MCP2515 Definitions. */
#define MCP2515_BAUDRATE    CAN_500KBPS     /* Baudrate of the MCP2515 chip. */
#define MCP2515_CLOCK_HZ    MCP_8MHZ        /* Run Chip at 8MHz as the onboard source is 8MHz. */
#define MCP2515_MASK        MCP_STDEXT      /* Mask for Standard and Extended IDs. */
#define MCP2515_MODE        MCP_LISTENONLY  /* Listen on the bus, do not transmit any messages or ACK. */
#define MCP2515_CS          10              /* Chip Select connected to the MCP2515 chip. */
#define MCP2515_INT         2               /* Interrupt pin connected to the MCP2515 chip. */
char str[128];

#define CAN_MSG_ENGINE_DATA 0x158
#define CAN_MSG_POWERTRAIN_DATA 0x17C
#define CAN_SIG_RPM_OFFSET 16         //bits
#define CAN_SIG_RPM_LEN 16         //bits

#define CAN_MSG_CAR_SPEED 0x309
#define CAN_SIG_CAR_SPEED_OFFSET 0         //bits
#define CAN_SIG_CAR_SPEED_LEN 16         //bits

#define CAN_MSG_GEARBOX 0x191

#define CAN_MSG_SCM_FEEDBACK 0x326 // has r/l blinker

//Todo, bad practice will move
int rpm         = 0;
int speedKph    = 0;
char gear       = 'X';

void Isr_readMsg(CANPacket* packet) {
    Serial.print("Recieved 0x");

	Serial.print(packet->getId(), HEX);
    Serial.print(" ");

    for (int i = 0; i < packet->getDlc(); i++) {
			Serial.print(packet->getData()[i]);
    }

    Serial.println();

    switch (packet->getId())
    {
    case CAN_MSG_ENGINE_DATA:
       speedKph = (packet->getData()[0] << 8) + packet->getData()[1];   // XMISSION_SPEED
    //   rpm = (packet->getData()[2] << 8) + packet->getData()[3];     // ENGINE_RPM
      
      break;

    case CAN_MSG_POWERTRAIN_DATA:
      rpm = (packet->getData()[2] << 8) + packet->getData()[3];    // ENGINE_RPM
      
      break;

    case CAN_MSG_GEARBOX:
      gear = packet->getData()[0];
      gear = gear & 0b00111111;             // GEAR_SHIFTER
      switch (gear)
      {
      case 0x20:
        gear = 'L';                         // Low Gear
        break;
      
      case 0x10:
        gear = 'S';                         // Sport
        break;

      case 0x8:
        gear = 'D';                         // Drive
        break;

      case 0x4:
        gear = 'N';                         // Neutral
        break;

      case 0x2:
        gear = 'R';                         // Reverse
        break;

      case 0x1:
        gear = 'P';                         // Park
        break;
      
      default:
        break;
      }

      break;
    
    default:
      break;
    }
    sprintf(str, "Gear: %c,\t RPM2: %d,\t Speed %d,\t %d,\t %d,\t %d", gear, rpm, speedKph);

    Serial.print(str);
    Serial.println();
}

/*!
 * @brief Default Constructor.
 */
Mcp2515Driver::Mcp2515Driver(CarData *data) {
    Errors driverRet = ERROR_FAIL;
    int mcpRet = OK;

    /* Validate CarData struct. */
    if (data == NULL) {
        Serial.print("Error: Bad CarData pointer!");
        return;
    }
    this->data = data;

    /* Create canNode object. */
    if (canNode != NULL) {
        Serial.print("Error: canNode already exists!");
        goto err;
    }
    canNode = new MCP2515();

    /* Initialize MCP2515 with parameters. */
    canNode->setClockFrequency(MCP2515_CLOCK_HZ);
    canNode->setPins(MCP2515_CS, MCP2515_INT);
    //todo possibly set masks

    mcpRet = canNode->begin(MCP2515_BAUDRATE);
    if (mcpRet != OK) {
      Serial.print("Error: ");
      Serial.println(mcpRet);

      goto err;
    }

    /* Set listen mode, and do not allow invalid packets. */
    canNode->setListenMode(false);

    /* Register the message receive callback. */
    canNode->onReceivePacket(Isr_readMsg);

err:
    /* Best effort. */
    canNode->end();
    delete canNode;
}


// /*!
//  * @brief Handles message checking.
//  */
// Errors Mcp2515Driver::readMsg(Mcp2515DriverCanMessage *canMsg) {
//     Errors driverRet = ERROR_FAIL;
//     INT8U mcpRet = CAN_FAIL;

//     /* Clear the CAN message container. */
//     canMsg = {};

//     if(digitalRead(MCP2515_INT) != LOW) {
//         /* If interrupt pin wasn't triggered, do nothing. */
//         return ERROR_OK;
//     }

//     /* Read a message. */
//     mcpRet = canNode->readMsgBuf(&canMsg->rxId, &canMsg->len, canMsg->data);
    
//     sprintf(str, "RxID %d Len %d, Buf %d", canMsg->rxId, canMsg->len, canMsg->data[0]);
//     Serial.print(str); //todo temp
//     if (mcpRet != CAN_OK) {
//         return ERROR_FAIL;
//     }

//     /* If message read successfully. */
//     return ERROR_OK;  
// }

// /*!
//  * @brief Print message.
//  */
// Errors Mcp2515Driver::printMsg(Mcp2515DriverCanMessage canMsg) {
//     Serial.print(canMsg.can_id, HEX);
//     Serial.print(","); 
//     Serial.print(canMsg.can_dlc, HEX);
//     Serial.print(",");
    
//     for (int i = 0; i<canMsg.can_dlc; i++)  {
//       Serial.print(packet->getData()[i],HEX);
//       Serial.print(",");
//     }

//     Serial.println();
// }