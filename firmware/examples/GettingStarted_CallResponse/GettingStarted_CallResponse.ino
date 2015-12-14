/*
   Dec 2014 - TMRh20 - Updated
   Derived from examples by J. Coliz <maniacbug@ymail.com>
*/
/**
 * Example for efficient call-response using ack-payloads
 *
 * This example continues to make use of all the normal functionality of the radios including
 * the auto-ack and auto-retry features, but allows ack-payloads to be written optionlly as well.
 * This allows very fast call-response communication, with the responding radio never having to
 * switch out of Primary Receiver mode to send back a payload, but having the option to switch to
 * primary transmitter if wanting to initiate communication instead of respond to a commmunication.
 */

//
// Hardware configuration
// from https://github.com/mshoemaker/SparkCore-RF24

/*
  PINOUTS
  http://docs.spark.io/#/firmware/communication-spi
  http://maniacbug.wordpress.com/2011/11/02/getting-started-rf24/

  SPARK CORE    SHIELD SHIELD    NRF24L01+
  GND           GND              1 (GND)
  3V3 (3.3V)    3.3V             2 (3V3)
  D6 (CSN)      9  (D6)          3 (CE)
  A2 (SS)       10 (SS)          4 (CSN)
  A3 (SCK)      13 (SCK)         5 (SCK)
  A5 (MOSI)     11 (MOSI)        6 (MOSI)
  A4 (MISO)     12 (MISO)        7 (MISO)

  NOTE: Also place a 10-100uF cap across the power inputs of
        the NRF24L01+.  I/O o fthe NRF24 is 5V tolerant, but
        do NOT connect more than 3.3V to pin 2(3V3)!!!
 */

 #include "particle-rf24/particle-rf24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins D6 & A2 */
RF24 radio( D0, D1, D2, D3, D4 );
/**********************************************************/
                                                                           // Topology
byte addresses[][5] = { 0xE7E7E7E7E7, 0xE7E7E7E7E7 };                      // Radio pipe addresses for the 2 nodes to communicate.

// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_pong_back;                                              // The role of the current running sketch

byte counter = 1;                                                          // A single byte to keep track of the data being sent back and forth


void setup(){

  Serial.begin(115200);
  Serial.println(F("Particle-RF24/firmware/examples/GettingStarted_CallResponse"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  // Setup and configure radio

  radio.begin();

  radio.enableAckPayload();                     // Allow optional ack payloads
  radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads

  if(radioNumber){
    radio.openWritingPipe(addresses[1]);        // Both radios listen on the same pipes by default, but opposite addresses
    radio.openReadingPipe(1,addresses[0]);      // Open a reading pipe on address 0, pipe 1
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  radio.startListening();                       // Start listening

  radio.writeAckPayload(1,&counter,1);          // Pre-load an ack-paylod into the FIFO buffer for pipe 1

  //Print debug info
  radio.printDetails();
}

void loop(void) {


/****************** Ping Out Role ***************************/

  if (role == role_ping_out){                               // Radio is in ping mode

    byte gotByte;                                           // Initialize a variable for the incoming response

    radio.stopListening();                                  // First, stop listening so we can talk.
    Serial.print(F("Now sending "));                         // Use a simple byte counter as payload
    Serial.println(counter);

    unsigned long time = micros();                          // Record the current microsecond count

    if ( radio.write(&counter,1) ){                         // Send the counter variable to the other radio
        if(!radio.available()){                             // If nothing in the buffer, we got an ack but it is blank
            Serial.print(F("Got blank response. round-trip delay: "));
            Serial.print(micros()-time);
            Serial.println(F(" microseconds"));
        }else{
            while(radio.available() ){                      // If an ack with payload was received
                radio.read( &gotByte, 1 );                  // Read it, and display the response time
                unsigned long timer = micros();

                Serial.print(F("Got response "));
                Serial.print(gotByte);
                Serial.print(F(" round-trip delay: "));
                Serial.print(timer-time);
                Serial.println(F(" microseconds"));
                counter++;                                  // Increment the counter variable
            }
        }

    }else{        Serial.println(F("Sending failed.")); }          // If no ack response, sending failed

    delay(1000);  // Try again later
  }


/****************** Pong Back Role ***************************/

  if ( role == role_pong_back ) {
    byte pipeNo, gotByte;                          // Declare variables for the pipe and the byte received
    while( radio.available(&pipeNo)){              // Read all available payloads
      radio.read( &gotByte, 1 );
                                                   // Since this is a call-response. Respond directly with an ack payload.
      gotByte += 1;                                // Ack payloads are much more efficient than switching to transmit mode to respond to a call
      radio.writeAckPayload(pipeNo,&gotByte, 1 );  // This can be commented out to send empty payloads.
      Serial.print(F("Loaded next response "));
      Serial.println(gotByte);
   }
 }



/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back ){
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = role_ping_out;  // Become the primary transmitter (ping out)
      counter = 1;
   }else
    if ( c == 'R' && role == role_ping_out ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
       role = role_pong_back; // Become the primary receiver (pong back)
       radio.startListening();
       counter = 1;
       radio.writeAckPayload(1,&counter,1);

    }
  }
}
