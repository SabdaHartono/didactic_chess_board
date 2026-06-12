#include <ArduinoBleChess.h>
#include "MyPeripheral.h"

//pin on banana pi pico hard ware
# define buzzer 3

# define file_a 17
# define file_b 47
# define file_c 44
# define file_d  43
# define file_e 14
# define file_f 12
# define file_g 13
# define file_h 15

# define clock 39
# define data 21
# define latch 38

#ifdef BLE_CHESS_LOGS
#ifndef ARDUINO_ARCH_ESP32
extern "C" {
int _write(int fd, char *ptr, int len) {
  (void) fd;
  return Serial.write(ptr, len);
}
}
#endif
#endif


extern void eboard_init(void);

MyPeripheral peripheral{};

void shift_register(uint16_t shift_data){
    uint16_t mask = 0x8000;
    digitalWrite(latch, LOW);
    while (mask){
        digitalWrite(clock, LOW);

        if (mask & shift_data){
            digitalWrite(data, HIGH);
        }
        else {
            digitalWrite(data, LOW);
        }

        delay(1);
        digitalWrite(clock, HIGH);
        delay(1);
        mask = mask >> 1;
    }

    digitalWrite(latch, HIGH);
    delay(2);
    digitalWrite(latch, LOW);
}

void out_led(uint8_t square, uint8_t color){
  uint16_t row_bit;
  uint8_t num;

  if (color == 0x10){square = 0x3f - square;}
  shift_register(0xffff);

  pinMode(file_a, OUTPUT);
  digitalWrite(file_a, LOW);
    
  pinMode(file_b, OUTPUT);
  digitalWrite(file_b, LOW);

  pinMode(file_c, OUTPUT);
  digitalWrite(file_c, LOW);

  pinMode(file_d, OUTPUT);
  digitalWrite(file_d, LOW);

  pinMode(file_e, OUTPUT);
  digitalWrite(file_e, LOW);

  pinMode(file_f, OUTPUT);
  digitalWrite(file_f, LOW);

  pinMode(file_g, OUTPUT);
  digitalWrite(file_g, LOW);

  pinMode(file_h, OUTPUT);
  digitalWrite(file_h, LOW);


  num = square & 0b00000111;
  switch (num){
    case 0:{
      pinMode(file_a, INPUT);
      break;
    }
    case 1:{
      pinMode(file_b, INPUT);
      break;
    }
    case 2:{
      pinMode(file_c, INPUT);
      break;
    }
    case 3:{
      pinMode(file_d, INPUT);
      break;
    }
    case 4:{
      pinMode(file_e, INPUT);
      break;
    }
    case 5:{
      pinMode(file_f, INPUT);
      break;
    }
    case 6:{
      pinMode(file_g, INPUT);
      break;
    }
    case 7:{
      pinMode(file_h, INPUT);
    }
  }

  num = 56 - (square & 0b00111000);
  num = num >> 2;
  row_bit = 1 << num;
  row_bit = ~ row_bit;

  shift_register(row_bit);
}


uint8_t get_file_value(){
     if (!digitalRead(file_a)){
        return 0;
    }

    if (!digitalRead(file_b)){
        return 1;
    }

    if (!digitalRead(file_c)){;
        return 2;
    }

    if (!digitalRead(file_d)){
        return 3;
    }

    if (!digitalRead(file_e)){
        return 4;
    }

    if (!digitalRead(file_f)){
        return 5;
    }

    if (!digitalRead(file_g)){
        return 6;
    }

    if (!digitalRead(file_h)){
        return 7;
    }
    return 8;
}

uint8_t scan_board(uint8_t color){
    int i;
    uint8_t square, num;

    shift_register(0xfffd);

    pinMode(file_a, INPUT);
    pinMode(file_b, INPUT);
    pinMode(file_c, INPUT);
    pinMode(file_d, INPUT);
    pinMode(file_e, INPUT);
    pinMode(file_f, INPUT);
    pinMode(file_g, INPUT);
    pinMode(file_h, INPUT);

    delay(1);

    num = get_file_value();
    if (num != 8){
      square = 56 + num;
      if (color == 0x10){square = 0x3f - square;}
      return square;
    }

    digitalWrite(data, HIGH);
    digitalWrite(latch, LOW);
    for (i = 48; i >= 0; i = i - 8){
        digitalWrite(clock, LOW);
        delay(1);
        digitalWrite(clock, HIGH);
        delay(1);

        digitalWrite(clock, LOW);
        delay(1);
        digitalWrite(clock, HIGH);
        delay(1);

        digitalWrite(latch, HIGH);
        delay(1);
        digitalWrite(latch, LOW);
        delay(1);
        
        num = get_file_value();
        if (num != 8){
          square = i + num;
          if (color == 0x10){square = 0x3f - square;}
          return square;
        }

    }
    return 64;
}


void setup() {
  Serial.begin(115200);
  //while (!Serial);
  ArduinoBleChess.begin("Arduino Ble Chess", peripheral);

  pinMode(buzzer, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(latch, OUTPUT);

  pinMode(file_a, INPUT);
  pinMode(file_b, INPUT);
  pinMode(file_c, INPUT);
  pinMode(file_d, INPUT);
  pinMode(file_e, INPUT);
  pinMode(file_f, INPUT);
  pinMode(file_g, INPUT);
  pinMode(file_h, INPUT);

  shift_register(0xffff);

  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);

  eboard_init();
}


void loop() {
  BLE.poll();
  peripheral.checkPeripheralMove();
}
