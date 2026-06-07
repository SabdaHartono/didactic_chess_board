#include <ArduinoBleChess.h>
#include "MyPeripheral.h"

// pin on esp32-s3 hardware
//# define buzzer 17
//# define file_a 41
//# define file_b 40
//# define file_c 47
//# define file_d 21
//# define file_e 16
//# define file_f 15
//# define file_g 7
//# define file_h 6
//# define file_i 5

//# define clock 1
//# define data 2
//# define latch 42

//pin on banana pi pico hard ware
# define buzzer 8

# define file_a 17
# define file_b 15
# define file_c 10
# define file_d  9
# define file_e 7
# define file_f 6
# define file_g 5
# define file_h 4
# define file_i 3

# define clock 43
# define data 44
# define latch 47

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
    delay(1);
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

  pinMode(file_i, OUTPUT);
  digitalWrite(file_i, LOW);

    

  num = square & 0b00000111;
  switch (num){
    case 0:{
      pinMode(file_a, INPUT);
      pinMode(file_b, INPUT);
      break;
    }
    case 1:{
      pinMode(file_b, INPUT);
      pinMode(file_c, INPUT);
      break;
    }
    case 2:{
      pinMode(file_c, INPUT);
      pinMode(file_d, INPUT);
      break;
    }
    case 3:{
      pinMode(file_d, INPUT);
      pinMode(file_e, INPUT);
      break;
    }
    case 4:{
      pinMode(file_e, INPUT);
      pinMode(file_f, INPUT);
      break;
    }
    case 5:{
      pinMode(file_f, INPUT);
      pinMode(file_g, INPUT);
      break;
    }
    case 6:{
      pinMode(file_g, INPUT);
      pinMode(file_h, INPUT);
      break;
    }
    case 7:{
      pinMode(file_h, INPUT);
      pinMode(file_i, INPUT);
    }
  }

  num = square & 0b00111000;
  num = num >> 2;
  row_bit = 1 << num;
  row_bit = ~ row_bit;

  shift_register(row_bit);
}

void out_led2(uint16_t rank_bit, uint16_t display_bit){

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

  pinMode(file_i, OUTPUT);
  digitalWrite(file_i, LOW);

  if (display_bit & 1){
    pinMode(file_a, INPUT);
  } 
  if (display_bit & 2){
    pinMode(file_b, INPUT);
  }
  if (display_bit & 4){
    pinMode(file_c, INPUT);
  }
  if (display_bit & 8){
    pinMode(file_d, INPUT);
  }
  if (display_bit & 16){
    pinMode(file_e, INPUT);
  }
  if (display_bit & 32){
    pinMode(file_f, INPUT);
  }
  if (display_bit & 64){
    pinMode(file_g, INPUT);
  }
  if (display_bit & 128){
    pinMode(file_h, INPUT);
  }
  if (display_bit & 256){
    pinMode(file_i, INPUT);
  }
  shift_register(rank_bit);
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
    pinMode(file_i, INPUT);

    delay(1);

    num = get_file_value();
    if (num != 8){
      square = num;
      if (color == 0x10){square = 0x3f - square;}
      return square;
    }

    digitalWrite(data, HIGH);
    digitalWrite(latch, LOW);
    for (i = 8; i < 64; i = i + 8){
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
  while (!Serial);
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
  pinMode(file_i, INPUT);

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
