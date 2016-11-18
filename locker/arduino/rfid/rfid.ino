#include <SPI.h>
#include "RFID.h"
#include <EEPROM.h>
#include <Keyboard.h>

#define SS_PIN 10
#define RST_PIN 9
#define RED_LED 2
#define GREEN_LED 3

#define KEY_LEFT_CTRL  0x80
#define KEY_LEFT_SHIFT 0x81
#define KEY_LEFT_ALT   0x82
#define KEY_LEFT_GUI   0x83
#define KEY_RIGHT_CTRL 0x84
#define KEY_RIGHT_SHIFT    0x85
#define KEY_RIGHT_ALT  0x86
#define KEY_RIGHT_GUI  0x87

#define KEY_UP_ARROW   0xDA
#define KEY_DOWN_ARROW 0xD9
#define KEY_LEFT_ARROW 0xD8
#define KEY_RIGHT_ARROW    0xD7
#define KEY_BACKSPACE  0xB2
#define KEY_TAB        0xB3
#define KEY_RETURN 0xB0
#define KEY_ESC        0xB1
#define KEY_INSERT 0xD1
#define KEY_DELETE 0xD4
#define KEY_PAGE_UP    0xD3
#define KEY_PAGE_DOWN  0xD6
#define KEY_HOME   0xD2
#define KEY_END        0xD5
#define KEY_CAPS_LOCK  0xC1
#define KEY_F1     0xC2
#define KEY_F2     0xC3
#define KEY_F3     0xC4
#define KEY_F4     0xC5
#define KEY_F5     0xC6
#define KEY_F6     0xC7
#define KEY_F7     0xC8
#define KEY_F8     0xC9
#define KEY_F9     0xCA
#define KEY_F10        0xCB
#define KEY_F11        0xCC
#define KEY_F12        0xCD

RFID rfid(SS_PIN, RST_PIN);

unsigned long previousMillis, previousMillisIsBlockedBlink, k = 0;
unsigned long blink_delay = 1000;
unsigned long add_blink_delay = 1000;
unsigned long added_blink_delay = 125;
unsigned long failed_blink_delay = 50;
unsigned long isBlocked_blink_interval = 10000;
unsigned long isBlocked_blink_delay = 25;
const long interval_to_store_card = 1000;
unsigned char reading_card[5], prev_card[5]; //for reading card
unsigned char master[5] = {118, 111, 43, 20, 38}; // allowed card
unsigned char all_cards[6][5];
unsigned char card_was_read;
boolean isBlocked, isMasterCardPresentInEEPROM;
unsigned char i;
String strPassword;
boolean traceOn;

/*boolean isMasterCard();
void add_card();
void reset_card_eeprom();
void resetPrevCard();
boolean isTheSameCard();
boolean verify();
boolean read_master_card();
void write_master_card();
void read_all_cards();
void reset_all_cards();
boolean add_card_to_EEPROM();
void allow();
void denied();
void simple_indication(int led, int blink_delay, int count);
void double_indication(int led1, int led2, int blink_delay, int count);
void block();
void login();*/

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  rfid.init();
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  k = card_was_read = 0;
  isBlocked = false;
  strPassword = "ghjcnjq231289";
  delay(5000);
  double_indication(RED_LED, GREEN_LED, blink_delay, 1);
  isMasterCardPresentInEEPROM = read_master_card();
  traceOn = true;
  if (traceOn) {Serial.println("TRACE: setup: before read all cards;");}
  read_all_cards();
}

void loop() {
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval_to_store_card) {
        previousMillis = currentMillis;
        resetPrevCard();
      }
      if ((!isTheSameCard())&&(k == 0)) {
        card_was_read = 1;
        Serial.println(" ");
        Serial.println("Card found");
        Serial.println("Cardnumber:");
        for (i = 0; i < 5; i++) {
          Serial.print(rfid.serNum[i]);
          Serial.print(" ");
          reading_card[i] = rfid.serNum[i];
          prev_card[i] = rfid.serNum[i];
        }
        Serial.println();
        
      }
      else {
        k++;
        Serial.print("."); Serial.print(k);
      }
    }
  }
  else {
    if ((card_was_read != 1) && (isBlocked)) {
      unsigned long currentMillisIsBlockedBlink = millis();
      if (currentMillisIsBlockedBlink - previousMillisIsBlockedBlink >= isBlocked_blink_interval) {
        previousMillisIsBlockedBlink = currentMillisIsBlockedBlink;
        double_indication(RED_LED, GREEN_LED, isBlocked_blink_delay, 3);
      }
    }
    if (!isMasterCardPresentInEEPROM) {
      double_indication(RED_LED, GREEN_LED, isBlocked_blink_delay, 5);
      delay(2000);
      if (card_was_read == 1) {
        if (traceOn) {Serial.println("TRACE: add_card: before write master card;");}
        write_master_card();
        if (traceOn) {Serial.println("TRACE: add_card: after write master card;");}
        isMasterCardPresentInEEPROM = read_master_card(); 
        k = 0;
      }
    }
    if ((k<=80) && (card_was_read == 1)) {
      card_was_read = 0;
      if (verify()) {
        allow();
      }
      else {
        denied();
      }
      k = 0;    
    }
    else {
      if ((k>=80) && (k<300)&& (card_was_read == 1)) {
        card_was_read = 0;
        if (isMasterCard()) {
          Serial.println("add card");
          add_card();
        }
        else {
          denied();
        }
        k = 0;
      }
      else
      {
        if ((k>300)&& (card_was_read == 1)) { read_all_cards(); k=0; card_was_read = 0;};
      }
    }
  }
  rfid.halt();
}

boolean isMasterCard() {
  for (i = 0; i < 5; i++) {
    if (reading_card[i] != master[i]) {
      break;
    }
  }
  if (i == 5) {
    return true;
  }
  else {
    return false;
  }
}

void add_card() {
  int l = 0;
  double_indication(RED_LED, GREEN_LED, add_blink_delay, 3);
  Serial.println();
  Serial.println("Please put card on cardreader!");
  delay(1000);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, HIGH);
  delay(2000);
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      for (i = 0; i < 5; i++) {
        reading_card[i] = rfid.serNum[i];
      }
      if (isMasterCard()) {
        if (traceOn) {Serial.println("TRACE: add_card: before reset card eeprom;");}
        reset_card_eeprom();
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        delay(2000);
        double_indication(RED_LED, GREEN_LED, failed_blink_delay, 5);
        return;
      }
      else {
        if (!add_card_to_EEPROM()) {
          digitalWrite(RED_LED, LOW);
          digitalWrite(GREEN_LED, LOW);
          delay(2000);
          double_indication(RED_LED, GREEN_LED, failed_blink_delay, 5);
        }
        else {
          Serial.println();
          Serial.println("Card was added to EEPROM:");
          Serial.println("Cardnumber:");
          for (i = 0; i < 5; i++) {
            Serial.print(rfid.serNum[i]);
            Serial.print(" ");
          }
          digitalWrite(RED_LED, LOW);
          digitalWrite(GREEN_LED, LOW);
          delay(2000);
          double_indication(RED_LED, GREEN_LED, added_blink_delay, 5);
        }
      }
    }
  }
  else {
    double_indication(RED_LED, GREEN_LED, failed_blink_delay, 5);
  }
}

void reset_card_eeprom() {
  for (int i =0; i < 30; i++) {
    EEPROM.write(i, 0);    
  }
  Serial.println("Cards EEPROM was erased!");
  isMasterCardPresentInEEPROM = read_master_card();
  reset_all_cards();
}

void resetPrevCard() {
  for (i = 0; i < 5; i++) {
    prev_card[i] = 0;
  }
}

boolean isTheSameCard() {
  for (i = 0; i < 5; i++) {
    if (prev_card[i] != rfid.serNum[i]) {
      return false;
    }
  }
  return true;
}

boolean verify() {
  for (int i=0; i<6; i++) {
    int j=0;
    for (j=0; j<5; j++) {
      if (reading_card[j]!=all_cards[i][j]) {
        break;
      }
    }
    if (j == 5) {
      return true;
    }
  }
  return false;

 /*
  for (i = 0; i < 5; i++) {
    if (reading_card[i] != master[i]) {
      break;
    }
  }
  if (i == 5) {
    return true;
  }
  else  {
    for (i = 0; i < 5; i++)
    {
      if (reading_card[i] != card_to_add[i]) { 
        break; 
      }
    }
    if (i == 5) {
      return true;  
    }
    else {
      return false; 
    }
  }
  */
}

boolean read_master_card()
{
  Serial.println("Read master card from EEPROM:");
  unsigned long sum = 0;
  for (int i=0; i < 5; i++) {
    master[i] = EEPROM.read(i);
    sum += master[i];
  }
  if (sum != 0 ){
    read_all_cards();
    return true;
  }
  else {
    return false;  
  }
  
}

void write_master_card()
{
  card_was_read = 0;
  Serial.println("Master card was writen to EEPROM:");
  for (int i=0; i < 5; i++) {
    EEPROM.write(i, reading_card[i]);
    Serial.print(reading_card[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void reset_all_cards()
{
  Serial.println("All cards were resetted.");
  for (int i=0; i<6; i++) {
    for (int j=0; j<5; j++) {
      all_cards[i][j] = 0;
    }
  }
  if (traceOn){Serial.println("TRACE: reset_all_cards: before read all cards;");}
  read_all_cards();
}

boolean add_card_to_EEPROM() {
  if (verify()) {
    Serial.println("Card was not added to EEPROM. Cause: this card present in EEPROM.");
    return false;
  }
  if (traceOn){Serial.println("TRACE: add_card_to_EEPROM: after verify();");}
  for (int i=0; i<6; i++) {
    if (EEPROM.read(5*i) == 0) {
      if (traceOn){Serial.println("TRACE: add_card_to_EEPROM: free memory was find;"); Serial.println(EEPROM.read(5*i));}
      for (int j=0; j<5; j++)
      {
        EEPROM.write(5*i+j, reading_card[j]);
      }
      if (traceOn){Serial.println("TRACE: add_card_to_EEPROM: before read all cards;");}
      read_all_cards();
      return true;  
    }
  }
  Serial.println("Card was not added to EEPROM. Cause: memory is full.");
  return false;
}

void read_all_cards()
{
  Serial.println("All cards were read from EEPROM.");
  for (int i=0; i<6; i++) {
    Serial.println();
    Serial.print("Card #");
    Serial.print(i+1);
    Serial.println(":");
    for (int j=0; j<5; j++) {
      all_cards[i][j] = EEPROM.read(5*i+j);
      Serial.print(all_cards[i][j]);
      Serial.print(" ");
    }
  }
  Serial.println();
}

void allow() {
  Serial.println();
  Serial.println("Access accept!");
  simple_indication(GREEN_LED, blink_delay, 1);
  if (!isBlocked) {
    block();
  }
  else {
    login();
  }
}

void denied() {
  Serial.println();
  Serial.println("Access denied!");
  simple_indication(RED_LED, blink_delay, 1);
}

void simple_indication(int led, int blink_delay, int count) {
  for (int i=0; i<count; i++) {
    digitalWrite(led, HIGH);
    delay(blink_delay);
    digitalWrite(led, LOW);
    delay(blink_delay);
  }
}

void double_indication(int led1, int led2, int blink_delay, int count) {
  for (int i=0; i<count; i++) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    delay(blink_delay);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    delay(blink_delay);
  }
}

void block() {
  Keyboard.press(KEY_RIGHT_GUI);
  Keyboard.write('l');
  Keyboard.release(KEY_RIGHT_GUI);
  isBlocked = true;
}

void login() {
    Keyboard.press(KEY_RIGHT_GUI);
    Keyboard.write('d');
    Keyboard.release(KEY_RIGHT_GUI);
    delay(500);
    Keyboard.press(KEY_BACKSPACE);
    Keyboard.release(KEY_BACKSPACE);
    Keyboard.println("Arthur160489");
    Keyboard.press(KEY_RETURN);
    Keyboard.release(KEY_RETURN);
    isBlocked = false;
}
