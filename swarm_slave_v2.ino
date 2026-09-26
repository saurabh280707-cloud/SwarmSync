#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN

// CHANGE THIS for each drone: 1, 2, or 3
#define THIS_NODE_ID 1

const byte slaveAddresses[3][6] = {
  "NODE1",
  "NODE2",
  "NODE3"
};

const byte masterAddress[6] = "MSTR0";

struct Command {
  char cmd[20];
  int nodeID;
  int value;
};

struct Response {
  int nodeID;
  char status[20];
  float battery;
};

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(5, 15);

  // Listen on this node's address
  radio.openReadingPipe(1, slaveAddresses[THIS_NODE_ID - 1]);
  radio.startListening();

  Serial.print("SwarmSync Slave v2 Ready | Node ID: ");
  Serial.println(THIS_NODE_ID);
}

void sendResponse(const char* status) {
  Response reply;
  reply.nodeID = THIS_NODE_ID;
  strcpy(reply.status, status);
  reply.battery = readBattery();

  radio.openWritingPipe(masterAddress);
  radio.stopListening();
  radio.write(&reply, sizeof(reply));

  // Resume listening
  radio.openReadingPipe(1, slaveAddresses[THIS_NODE_ID - 1]);
  radio.startListening();
}

float readBattery() {
  // Reads battery voltage from analog pin
  int raw = analogRead(34); // GPIO34
  float voltage = (raw / 4095.0) * 3.3 * 2; // voltage divider
  return voltage;
}

void executeCommand(Command* pkt) {
  if (pkt->nodeID != THIS_NODE_ID) return; // Ignore other nodes

  Serial.print("Command: ");
  Serial.println(pkt->cmd);

  if (strcmp(pkt->cmd, "MOVE_FORWARD") == 0) {
    Serial.println("Moving forward");
    // Add ESC/motor code here
    sendResponse("MOVING");
  }
  else if (strcmp(pkt->cmd, "HOVER") == 0) {
    Serial.println("Hovering");
    // Add hover stabilization here
    sendResponse("HOVERING");
  }
  else if (strcmp(pkt->cmd, "LAND") == 0) {
    Serial.println("Landing");
    // Add landing sequence here
    sendResponse("LANDING");
  }
  else {
    sendResponse("UNKNOWN_CMD");
  }
}

void loop() {
  if (radio.available()) {
    Command pkt;
    radio.read(&pkt, sizeof(pkt));
    executeCommand(&pkt);
  }
}