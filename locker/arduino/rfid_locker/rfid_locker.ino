#include <SPI.h>
#include "RFID.h"
#include <Keyboard.h>

#define SS_PIN 10
#define RST_PIN 9
#define BLUE_LED 6
#define YELLOW_LED 7

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

unsigned char reading_card[5]; //for reading card
unsigned char master[2][5] = {{118,111,43,20,38},{146,241,61,213,139}}; // allowed card
boolean isBlocked;
unsigned char i;
unsigned char j;

void indication(int led);
void allow();
void denied();

void setup()
{ 
  Serial.begin(9600);
  SPI.begin(); 
  rfid.init();
  Keyboard.begin();
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  isBlocked = false;
}

void loop()
{
    if (rfid.isCard()) 
    {
        if (rfid.readCardSerial()) 
        {
                /* Reading card */
                Serial.println(" ");
                Serial.println("Card found");
                Serial.println("Cardnumber:"); 
                for (i = 0; i < 5; i++)
                {     
                  Serial.print(rfid.serNum[i]);
                  Serial.print(" ");
                  reading_card[i] = rfid.serNum[i];
                }
                Serial.println();
                //verification
                for (i = 0; i < 5; i++)              
                {
                    if (reading_card[i]!=master[0][i])
                    {
                      if (reading_card[i]!=master[1][i])
                      {
                        break;
                      }
                  }
                }
                if (i == 5)
                {
                  allow();
                }
                else
                {
                  denied();
                }
         } 
    }
    rfid.halt();
}

void allow()
{
  Serial.println("Access accept!");
  indication(BLUE_LED);
  if (!isBlocked)
  {
    Keyboard.press(KEY_RIGHT_GUI);
    Keyboard.write('l');
    Keyboard.release(KEY_RIGHT_GUI);
    isBlocked = true;
  }
  else
  {
    //Keyboard.press(KEY_LEFT_ALT);
    //Keyboard.write(KEY_LEFT_SHIFT);
    //Keyboard.release(KEY_LEFT_ALT);
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
  delay(3000);
  
}
void denied()
{
  Serial.println("Access denied!");
  indication(YELLOW_LED);
}
void indication(int led)
{
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
}
