File 1 — Master Node (Multi-Node + Bidirectional)

#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN

// Unique address for each slave node
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
  radio.setDataRate(RF24_250KBPS); // Better range
  radio.setRetries(5, 15);         // Retry on fail
  Serial.println("SwarmSync Master v2 Ready");
  Serial.println("Nodes: 3");
}

void sendCommand(int nodeID, const char* cmd, int value) {
  Command packet;
  strcpy(packet.cmd, cmd);
  packet.nodeID = nodeID;
  packet.value = value;

  // Open writing pipe for target node
  radio.openWritingPipe(slaveAddresses[nodeID - 1]);
  radio.stopListening();

  bool sent = radio.write(&packet, sizeof(packet));

  if (sent) {
    Serial.print("Sent to NODE");
    Serial.print(nodeID);
    Serial.print(": ");
    Serial.println(cmd);
  } else {
    Serial.print("NODE");
    Serial.print(nodeID);
    Serial.println(" not responding");
  }

  // Now listen for response
  radio.openReadingPipe(1, masterAddress);
  radio.startListening();
  delay(200);

  if (radio.available()) {
    Response reply;
    radio.read(&reply, sizeof(reply));
    Serial.print("NODE");
    Serial.print(reply.nodeID);
    Serial.print(" Status: ");
    Serial.print(reply.status);
    Serial.print(" | Battery: ");
    Serial.print(reply.battery);
    Serial.println("V");
  }
}

void loop() {
  // Send to all 3 nodes
  sendCommand(1, "MOVE_FORWARD", 50);
  delay(500);
  sendCommand(2, "MOVE_FORWARD", 50);
  delay(500);
  sendCommand(3, "MOVE_FORWARD", 50);
  delay(2000);

  // Hover all
  sendCommand(1, "HOVER", 0);
  sendCommand(2, "HOVER", 0);
  sendCommand(3, "HOVER", 0);
  delay(2000);

  // Land all
  sendCommand(1, "LAND", 0);
  sendCommand(2, "LAND", 0);
  sendCommand(3, "LAND", 0);
  delay(3000);
}

