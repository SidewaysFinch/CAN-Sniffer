/**
 * CAN MCP2515_nb
 * Copyright 2020 WitchCraftWorks Team, All Rights Reserved
 *
 * Licensed under Apache 2.0
 */

#include <MCP2515_nb.h>
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

MCP2515 MCP = MCP2515();

int ret = 0;

void setup() {
	Serial.begin(115200);
	while (!Serial) {
		;
	}

	Serial.println("CAN Receiver Callback");
    MCP.setClockFrequency(MCP_8MHZ);

	ret = MCP.begin(50E3);
	// start the CAN bus at 50 kbps
	if (ret) {
		Serial.println("Starting CAN failed!");
		Serial.println(ret);
		while (true);
	}
    MCP.setListenMode(false);

	// register the receive callback
	MCP.onReceivePacket(onReceive);
}

void loop() {
	// do nothing
}

void onReceive(CANPacket* packet) {
	// Serial.print("Received ");

	// // Serial.print(packet->getId(), HEX);
    // // Serial.print(" ");

    // for (int i = 0; i < packet->getDlc(); i++) {
	// 		Serial.print(packet->getData()[i]);
    // }

    // Serial.println();

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
      return;
    }
    sprintf(str, "Gear: %c,\t RPM2: %d,\t Speed %d", gear, rpm, speedKph);

    Serial.print(str);
    Serial.println();
}