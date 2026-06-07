#pragma once
#include <ArduinoBleChess.h>
# define buzzer 8


extern uint8_t scan_board(uint8_t color);
extern void out_led(uint8_t square, uint8_t color);
extern void shift_register(uint16_t shift_data);
extern void out_led2(uint16_t rank_bit, uint16_t display_bit);
typedef struct
{
  uint8_t move_from;
  uint8_t move_to;
  uint8_t move_from8;
  uint8_t move_to8;
  uint8_t move_bit;
  uint8_t capturing;
  uint8_t captured;
} move_t;

uint8_t offset_loc[] = {
    0x0a, 0x0c, 0x00, 0x08, 0x08, 0x0c, 0x0c, 0x10, 0x08, 0x10, 0x08, 0x10};

uint8_t offset[] = {
    0xdf, 0xe1, 0xf2, 0x12, 0x21, 0x1f, 0x0e, 0xee,
    0xef, 0xf1, 0x11, 0x0f,
    0xf0, 0x01, 0x10, 0xff};

uint8_t piece_type[] = {
    0x01, 0x02, 0x0b, 0x0c, 0x0d, 0x06};

int list_idx;
uint8_t move_list[300], board[64];
uint8_t gilir, enpass, cast, king_w, king_b, fifty, peripheral_color, central_color;
uint8_t old_move_to, old_move_from, old_captured;
uint8_t old_movebit, old_fifty, old_enpass, old_cast;
uint8_t _move_from, _move_to, _move_bit, _central_move_from, _central_move_to, _status, _orientation;
uint8_t _buzzer_count, _board_mode;
int  _running_led;
bool end_game, _game_begin;
uint32_t _now, _time_limit;
char _move_string[6];

uint8_t piece_sym(char x){
  const char* the_pieces = "PNBRQKpnbrqk";
  uint8_t piece_num[12] = {0x21, 0x22, 0x2b, 0x2c, 0x2d, 0x26, 0x11, 0x12, 0x1b, 0x1c, 0x1d, 0x16};
  int i;
  uint8_t num = 0;
  for (i = 0; i < 12; i++){
    if (the_pieces[i] == x){
      num = piece_num[i];
      break;
    }
  }
  return num;
}

char sym_to_letter(uint8_t symbol){
  const char* the_pieces = ".PNBRQKpnbrqk";
  uint8_t piece_idx;

  piece_idx = (symbol & 0b00000111);
  if ((symbol & 0b00010000)){piece_idx += 6;}
  return the_pieces[piece_idx];
}

void print_board(){
  const char* the_pieces = "PNBRQKpnbrqk";
  char letter;
  uint8_t piece;
  int i, j, l;

  l = 8;
  Serial.println("Board:");
  
  for(j = 0; j < 64; j+= 8){
    
    if (_orientation == 0x20){Serial.print(l);} else {Serial.print(9 - l);}
    Serial.print("|");
    l--;
    for(i = 0; i < 7; i++){
      if (_orientation == 0x20){piece = board[i + j];} else {piece = board[63 - i - j];}
      letter = sym_to_letter(piece);
      Serial.print(letter);
    }
    if (_orientation == 0x20){piece = board[7 + j];} else {piece = board[56 - j];}
    letter = sym_to_letter(piece);
    Serial.println(letter);
  }
  Serial.println(" ________");

  if (_orientation == 0x20){
    Serial.println("  abcdefgh");
  }
  else {
    Serial.println("  hgfedcba");
  }
}

void fen_to_board(const char* fen){
  int ind, i;
  uint8_t piece;
  char c;
  //initialize board width empty square
  for (ind = 0; ind < 64; ind++){
    board[ind] = 0;
  }

  ind = 0;
  for (i = 0; fen[i] != '\0'; i++){

    c = fen[i];
    if (ind < 64){
      if ((c > '0') && (c < '9')){
        //skip empty square
        ind += c - '0';
      }
      else {
        piece = piece_sym(c);
        if (piece){
          board[ind] = piece;
          //white king ??
          if (piece == 0x26){king_w = ind;} //register white king position
          //black king ??
          if (piece == 0x16){king_b = ind;} //register black king position
          ind++;
        }
      }
    }
    else {
      if ((c == 'w') || (c == 'W')){gilir = 0x20;}
      if ((c == 'b') || (c == 'B')){gilir = 0x10;}
    }
  }
}

void move_to_string(){
  int idx;
  idx = _move_from & 0b00000111; //square to file conversion
  _move_string[0] = 'a' + idx;

  idx = _move_from & 0b00111000; //square to rank conversion
  idx = idx >> 3;
  idx = 7 - idx;
  _move_string[1] = '1' + idx;

  idx = _move_to & 0b00000111; //square to file conversion
  _move_string[2] = 'a' + idx;

  idx = _move_to & 0b00111000; //square to rank conversion
  idx = idx >> 3;
  idx = 7 - idx;
  _move_string[3] = '1' + idx;

  _move_string[4] = '\0';

}

//********* colection chess game function ************************
uint8_t is_attacked(uint8_t location)
{
  uint8_t j, piece, piece1, awal, akhir, delta, ke;
  uint8_t mentri, raja, asal, lawan;
  uint8_t move_to;

  lawan = gilir ^ 0x30;
  raja = 0x06 | lawan;
  mentri = 0x0d | lawan;
  location += (location & 56);

  for (int i = 0; i < 4; i++)
  {
    piece = piece_type[i];
    piece = piece | lawan;

    j = i << 1;
    awal = offset_loc[j];
    j++;
    akhir = offset_loc[j];

    if (piece == 0x21)
    {
      awal = 8;
      akhir = 10;
    } // koreksi bila pion putih

    for (j = awal; j < akhir; j++)
    {
      delta = offset[j];
      asal = location;
      do
      {
        ke = asal - delta;

        move_to = ke;
        if (move_to & 0x88)
        {
          break;
        }
        move_to += (ke & 7);
        move_to = move_to >> 1;
        piece1 = board[move_to];

        if (piece1 == piece)
        {
          return 1;
        }

        else if (piece1 == mentri)
        {
          if (i > 1)
          {
            return 1;
          }
        }

        else if (piece1 == raja)
        {
          if ((i > 1) && (asal == location))
          {
            return 1;
          }
        }

        asal = ke;

      } while ((piece1 == 0) && (piece & 0x08));
    }
  }
  return 0;
}

uint8_t is_incheck(void)
{
  int result;
  if (gilir == 0x10)
  {
    result = is_attacked(king_b);
  }
  else
  {
    result = is_attacked(king_w);
  }
  return result;
}

void take_back(void)
{
  uint8_t move_from, move_to, move_bit, cap_piece, piece, idx;
  // kembalikan  gilir
  gilir = gilir ^ 0x30;

  // kembalikan cast, enpass dan fifty
  enpass = old_enpass;
  cast = old_cast;
  fifty = old_fifty;

  move_from = old_move_from;
  move_to = old_move_to;
  move_bit = old_movebit;
  cap_piece = old_captured;

  piece = board[move_to];
  board[move_from] = piece;
  board[move_to] = cap_piece;

  // back to pawn if promotion
  if (move_bit & 0x20)
  {
    board[move_from] = 0x01 | gilir;
  }

  // take back pawn if enpassant
  if (move_bit & 0x04)
  {
    if (gilir == 0x10)
    {
      board[move_to - 8] = 0x21;
    } // back turn, take back white pawn
    else
    {
      board[move_to + 8] = 0x11;
    } // white turn, take back pawn
  }

  // catat posisi raja
  if (piece == 0x16)
  {
    king_b = move_from;
  } // raja hitam
  if (piece == 0x26)
  {
    king_w = move_from;
  } // raja putih

  // kembalikan benteng bila rookade
  if (move_bit & 0x02)
  {
    if (move_to == 0x3e)
    {
      board[0x3f] = 0x2c;
      board[0x3d] = 0;
    }
    else if (move_to == 0x3a)
    {
      board[0x38] = 0x2c;
      board[0x3b] = 0;
    }
    else if (move_to == 0x06)
    {
      board[0x07] = 0x1c;
      board[0x05] = 0;
    }
    else if (move_to == 0x02)
    {
      board[0x00] = 0x1c;
      board[0x03] = 0;
    }
  }
}




int make_move(int move_from, int move_to, int move_bit)
{
  int rook_from, rook_to, piece, idx;

  old_move_to = move_to;
  old_move_from = move_from;
  old_movebit = move_bit;
  old_fifty = fifty;
  old_enpass = enpass;
  old_cast = cast;

  if (move_bit & 0x02)
  { // cek apakah rookade
    if (is_incheck())
    {
      return 0;
    }
    if (move_to == 0x3e)
    { // apakah sisi raja putih
      if (board[0x3e] != 0)
      {
        return 0;
      }
      if (board[0x3d] != 0)
      {
        return 0;
      }
      if (is_attacked(0x3e))
      {
        return 0;
      }
      if (is_attacked(0x3d))
      {
        return 0;
      }
      rook_from = 0x3f;
      rook_to = 0x3d;
      piece = 0x2c;
    }
    else if (move_to == 0x3a)
    { // apakah sisi mentri putih
      if (board[0x39] != 0)
      {
        return 0;
      }
      if (board[0x3a] != 0)
      {
        return 0;
      }
      if (board[0x3b] != 0)
      {
        return 0;
      }
      if (is_attacked(0x3a))
      {
        return 0;
      }
      if (is_attacked(0x3b))
      {
        return 0;
      }
      rook_from = 0x38;
      rook_to = 0x3b;
      piece = 0x2c;
    }
    else if (move_to == 0x06)
    { // apakah sisi raja hitam
      if (board[0x06] != 0)
      {
        return 0;
      }
      if (board[0x05] != 0)
      {
        return 0;
      }
      if (is_attacked(0x06))
      {
        return 0;
      }
      if (is_attacked(0x05))
      {
        return 0;
      }
      rook_from = 0x07;
      rook_to = 0x05;
      piece = 0x1c;
    }
    else if (move_to == 0x02)
    { // apakah sisi mentri hitam
      if (board[0x01] != 0)
      {
        return 0;
      }
      if (board[0x02] != 0)
      {
        return 0;
      }
      if (board[0x03] != 0)
      {
        return 0;
      }
      if (is_attacked(0x02))
      {
        return 0;
      }
      if (is_attacked(0x03))
      {
        return 0;
      }
      rook_from = 0x00;
      rook_to = 0x03;
      piece = 0x1c;
    }
    // rookade berhasil pindahkan benteng
    board[rook_from] = 0;
    board[rook_to] = piece;
  }

  // save capture
  piece = board[move_to];
  old_captured = piece;
  piece = board[move_from];
  // bila langkah raja, update posisi raja dan rookade
  if (piece == 0x16)
  { // bila raja hitam
    king_b = move_to;
    cast = cast & 0b00110000;
  }

  if (piece == 0x26)
  { // bila raja putih
    king_w = move_to;
    cast = cast & 0b11000000;
  }

  board[move_from] = 0;
  // cek apakah promosi, bila demikian ubah pion menjadi mentri
  if (move_bit & 0x20)
  {
    piece = piece | 0x0d;
  }

  board[move_to] = piece;

  // update status rookade
  if (cast != 0)
  { // bila rookade masih sah
    if ((move_to == 0x3f) || (move_from == 0x3f))
    {
      cast &= 0b11100000;
    }
    if ((move_to == 0x38) || (move_from == 0x38))
    {
      cast &= 0b11010000;
    }
    if ((move_to == 0x07) || (move_from == 0x07))
    {
      cast &= 0b10110000;
    }
    if ((move_to == 0x00) || (move_from == 0x00))
    {
      cast &= 0b01110000;
    }
  }

  // update status enpasan
  enpass = 0x0f; // asumsikan empasan tidak legal
  if (move_bit & 0x08)
  {
    enpass = move_from & 0b00000111;
  } // apakah dua langkah pion?

  // update fifty clock
  if (move_bit & 0x11)
  {
    fifty = 0;
  }
  else
  {
    fifty++;
  } // apakah langkah pion atau capture

  // bila enpasan, maka hapus(capture) pion
  if (move_bit & 0x04)
  { // bila enpasan
    if (gilir == 0x10)
    { // bila giliran hitam capture pion putih
      board[move_to - 8] = 0;
    }
    else
    { // giliran putih capture pion hitam
      board[move_to + 8] = 0;
    }
  }


  if (is_incheck())
  {
    gilir = gilir ^ 0x30;
    take_back();
    return 0;
  }
  gilir = gilir ^ 0x30;
  return 1;
}

 

 

void push_move(move_t *move)
{
  uint8_t hasil;
  hasil = make_move(move->move_from, move->move_to, move->move_bit);
  if (hasil == 0)
  {
      return;
  }

  take_back();

  move_list[list_idx] = move->move_from;
  list_idx++;
  move_list[list_idx] = move->move_to;
  list_idx++;
  move_list[list_idx] = move->move_bit;
  list_idx++;
}

void get_piece(move_t *move)
{
  // berada diluar papan
  if (move->move_to8 & 0x88)
  {
    move->captured = 0x80;
  }
  else
  {
    move->move_to = move->move_to8 + (move->move_to8 & 7);
    move->move_to = (move->move_to) >> 1;
    move->captured = board[move->move_to];
  }
}



void move_gen()
{
  uint8_t lawan, i, awal, akhir, indeks, move_bit;
  move_t m0;

  lawan = gilir ^ 0x30;
  list_idx = 0;

  for (int row = 0; row < 64; row = row + 8)
  {
    for (int file = 0; file < 8; file++)
    {
      m0.move_from = row + file;
      m0.capturing = board[m0.move_from];
      m0.move_from8 = file + (row << 1);

      if ((m0.capturing & gilir) == 0)
        continue; // apakah sesuai gilir?

      if (m0.capturing == 0x21)
      { // apakah pion putih?
        // set move_bit bila langkah promosi
        if (row == 0x08)
        {
          move_bit = 0x30; //langkah pion + promosi
        }
        else
        {
          move_bit = 0x10; //langkah pion
        }

        // pion putih capture sebelah kiri
        m0.move_to8 = m0.move_from8 + 0xef;
        get_piece(&m0);

        if (m0.captured & 0x10)
        { // musuh?
          // berhasil capture
          m0.move_bit = move_bit | 1;
          push_move(&m0);
        }

        // pion putih capture sebelah kanan
        m0.move_to8 = m0.move_from8 + 0xf1;
        get_piece(&m0);
        if (m0.captured & 0x10)
        { // musuh?
          // berhasil capture
          m0.move_bit = move_bit | 1;
          push_move(&m0);
        }

        // pion putih maju selangkah
        m0.move_to8 = m0.move_from8 + 0xf0;
        get_piece(&m0);
        if (m0.captured == 0)
        {
          // berhasil pindah ke tempat kosong
          m0.move_bit = move_bit;
          push_move(&m0);
          // pion putih maju dua langkah
          if (row == 0x30)
          { // ada pada baris ke-2 ?
            m0.move_to8 = m0.move_from8 + 0xe0;
            get_piece(&m0);
            if (m0.captured == 0)
            {
              // berhasil pindah ke tempat kosong
              m0.move_bit = 0x18;
              push_move(&m0);
            }
          }
        }
      }
      else if (m0.capturing == 0x11)
      { // apakah pion hitam
        // set move_bit bila promosi
        if (row == 0x30)
        {
          move_bit = 0x30;
        }
        else
        {
          move_bit = 0x10;
        }
        m0.move_to8 = m0.move_from8 + 0x0f; // pion hitam capture sebelah kanan
        get_piece(&m0);
        if (m0.captured & 0x20)
        { // musuh?
          // berhasil capture
          m0.move_bit = move_bit | 1;
          push_move(&m0);
        }

        m0.move_to8 = m0.move_from8 + 0x11; // pion hitam capture sebelah kiri
        get_piece(&m0);
        if (m0.captured & 0x20)
        { // musuh?
          // berhasil capture
          m0.move_bit = move_bit | 1;
          push_move(&m0);
        }

        // pion hitam maju selangkah
        m0.move_to8 = m0.move_from8 + 0x10;
        get_piece(&m0);
        if (m0.captured == 0)
        {
          // berhasil pindah ke tempat kosong
          m0.move_bit = move_bit;
          push_move(&m0);
          // pion putih maju dua langkah
          if (row == 0x08)
          { // ada pada baris ke-2 ?
            m0.move_to8 = m0.move_from8 + 0x20;
            get_piece(&m0);
            if (m0.captured == 0)
            {
              // berhasil pindah ke tempat kosong
              m0.move_bit = 0x18;
              push_move(&m0);
            }
          }
        }
      }
      else
      { // perwira
        indeks = (m0.capturing & 0x07);
        indeks--;
        indeks = indeks << 1;
        awal = offset_loc[indeks];
        akhir = offset_loc[indeks + 1];

        for (i = awal; i < akhir; i++)
        {
          indeks = offset[i];
          m0.move_to8 = m0.move_from8;
          do
          {
            m0.move_to8 = m0.move_to8 + indeks;
            get_piece(&m0);
            if (m0.captured == 0)
            {
              m0.move_bit = 0x00;
              push_move(&m0);
            }
            else if (m0.captured & lawan)
            {
              m0.move_bit = 0x01;
              push_move(&m0);
            }

          } while ((m0.captured == 0) && (m0.capturing & 0x08));
        }
      } // perwira

    } // loop for file

  } // loop for row

  // move gen tahap kedua, rookade
  m0.move_bit = 0x02; // move bit untuk rookade
  if (gilir == 0x10)
  {                      // giliran hitam
    m0.move_from = 0x04; // asal raja hitam
    if (cast & 0b01000000)
    { // boleh rookade sisi raja
      m0.move_to = 0x06;
      push_move(&m0);
    }
    if (cast & 0b10000000)
    { // boleh rookade sisi mentri
      m0.move_to = 0x02;
      push_move(&m0);
    }
  }
  else
  {                      // giliran putih
    m0.move_from = 0x3c; // asal raja putih
    if (cast & 0b00010000)
    { // boleh rookade sisi raja
      m0.move_to = 0x3e;
      push_move(&m0);
    }
    if (cast & 0b00100000)
    { // boleh rookade sisi mentri};
      m0.move_to = 0x3a;
      push_move(&m0);
    }
  }

  // move gen tahap ketiga enpassan
  if (enpass != 0x0f)
  { // cek apakah enpassan sah
    m0.move_bit = 0x15;
    if (gilir & 0x10)
    {                              // giliran hitam
      m0.move_to8 = enpass + 0x3f; // pion hitam di kanan
      get_piece(&m0);
      if (m0.captured == 0x11)
      { // apakah pion hitam?
        m0.move_from = enpass + 0x1f;
        m0.move_to = enpass + 0x28;
        push_move(&m0);
      }
      m0.move_to8 = enpass + 0x41; // pion hitam di kiri
      get_piece(&m0);
      if (m0.captured == 0x11)
      { // apakah pion hitam?
        m0.move_from = enpass + 0x21;
        m0.move_to = enpass + 0x28;
        push_move(&m0);
      }
    }
    else
    {                              // giliran putih
      m0.move_to8 = enpass + 0x2f; // pion putih sebelah kiri
      get_piece(&m0);
      if (m0.captured == 0x21)
      { // apakah pion putih?
        m0.move_from = enpass + 0x17;
        m0.move_to = enpass + 0x10;
        push_move(&m0);
      }

      m0.move_to8 = enpass + 0x31; // pion putih sebelah kanan
      get_piece(&m0);
      if (m0.captured == 0x21)
      { // apakah pion putih?
        m0.move_from = enpass + 0x19;
        m0.move_to = enpass + 0x10;
        push_move(&m0);
      }
    }
  }

}



void make_move_central(const char *move){
  int move_from, move_to, i;
  uint8_t move_bit, piece;
  char piece_letter;
  bool found;
  
  move_from = move[0] - 'a'  + 56 + ('1' - move[1])*8;
  found = false;
  for (i = 0; i < list_idx; i += 3){
    if (move_from == move_list[i]){
      found = true;
      break;
    }
  }

  if (!found){
    Serial.println("Error, Move-from from central not legal");
  }

  move_to = move[2] - 'a'  + 56 + ('1' - move[3])*8;
  found = false;
  for (i = 0; i < list_idx; i += 3){
    if ((move_from == move_list[i]) && (move_to == move_list[i+1])){
      found = true;
      move_bit = move_list[i + 2];
      break;
    }
  }

  if (!found){
    Serial.println("Error, Move-to from central not legal");
  }

make_move(move_from, move_to, move_bit);

//handle promotion
if (move_bit & 0x20){
  piece_letter = move[4];
  if (gilir == 0x10){ 
    piece_letter = piece_letter - 32; //convert to upper case
  }
  piece = piece_sym(piece_letter);

  if (piece != 0){
    board[move_to] = piece;
  }

}

  print_board();
  move_gen();
  _central_move_from = move_from;
  _central_move_to = move_to;
  _status = 3;
}



//-------------------------------------------------------------------
void eboard_init(void){
  _move_string[0] = '\0';
  _central_move_from = 64;
  _status = 0;
  _buzzer_count = 0;
  _orientation = 0x20;
  _board_mode = 1; //default play vs computer
  end_game = false;
  _game_begin = false;
}

void select_option(uint8_t square){
  if (square == 0x3f){
    _board_mode = 1;
    Serial.println("Board mode: play vs computer");
  }
  else if (square == 0x37){
    _orientation = 0x20;
    _board_mode = 2;
    Serial.println("Board mode: plan on line");
    Serial.println("Board orientation normal");
  }
  else if (square == 0x2f){
    _board_mode = 3;
    Serial.println("Board mode: play on the board");
  }
  _status = 0;
  out_led(square, 0x20);
  digitalWrite(buzzer, HIGH);
  _buzzer_count = 1;
  _time_limit = millis() + 500;
}

void board_move(void){
  static uint8_t selected_square;
  static uint8_t scan_counter = 20;
  static bool wait_release = false, long_press = false;
  static uint32_t press_time;
  uint8_t square, file;
  int i;
  bool found;
  uint32_t now;

  //handling buzzer
  if (_buzzer_count == 1){
    now = millis();
    if (now > _time_limit){
      _buzzer_count = 0;
      digitalWrite(buzzer, LOW);
    }
  }
  else if (_buzzer_count == 3){
    now = millis();
    if (now > _time_limit){
      _buzzer_count = 2;
      digitalWrite(buzzer, LOW);
      _time_limit = now + 500;
    }
  }
  else if (_buzzer_count == 2){
    now = millis();
    if (now > _time_limit){
      _buzzer_count = 1;
      digitalWrite(buzzer, HIGH);
      _time_limit = now + 500;
    }
  }

  //wait square to release (unpress the square)
  if (wait_release){
    square = scan_board(0x20);
    if (square == 64){
      wait_release = false;
      long_press = false;
    }
    else {
      if (selected_square != 64){out_led(selected_square, _orientation);}
      if (long_press){
        now = millis();
        if (now > press_time){
          long_press = false;
          select_option(square);
        }
      }
      return;
    }
  }

  switch(_status){
    case 1: //scan for move-from in peripheral turn
      if (_board_mode == 3) { //play on the board
        if (scan_counter > 0){scan_counter--;}
        if ((scan_counter == 0) && (_move_to != 64)){
          out_led(_move_to, _orientation); //turn on led on last move
          scan_counter = 20;
        }
      }
      
      square = scan_board(_orientation);

      if (square == 64){break; } //peripheral do not press any square
      

      if ((_board_mode == 1) && _game_begin){//play vs computer
        if (_orientation != gilir){
          _orientation = gilir;
          square = 0x3f - square;
        }
        if (_orientation == 0x20){ Serial.println("Board orientation: normal");} else {Serial.println("Board orientation: inverted");}
        _game_begin = false;
      }

      found = false;
      for (i = 0; i < list_idx; i += 3){ //check if move from legal
        if (square == move_list[i]){
          found = true;
          if (_game_begin && (_board_mode == 3)){
            if (_orientation == 0x20){ Serial.println("Board orientation: normal");} else {Serial.println("Board orientation: inverted");}
          }
          _game_begin = false;
          break;
        }
      }

      if ((_board_mode == 3) && !found  && _game_begin){  
        _orientation = _orientation ^ 0x30;
        square = scan_board(_orientation);

        for (i = 0; i < list_idx; i += 3){ //check if move from legal
          if (square == move_list[i]){
            found = true;
            if (_orientation == 0x20){ Serial.println("Board orientation: normal");} else {Serial.println("Board orientation: inverted");}
            _game_begin = false;
            break;
          }
        }
      }

      if (found){ //move_from legal
       
        Serial.print("pressed square(move from): ");
        Serial.println(square);

        _move_from = square; //move legal, save move from
        out_led(square, _orientation); //turn on led on square from
        wait_release = true;
        selected_square = square;
        _status = 2; //next state scan for move to
      }
      else { //
        Serial.print("pressed square(move from): ");
        Serial.println(square);
        Serial.println("warning, move from not legal!");
        _buzzer_count = 1;
        digitalWrite(buzzer, HIGH);
        _time_limit = millis() + 500;
        wait_release = true;
        selected_square = 64;
      }
      long_press = true;
      press_time = millis() + 3000;
      break;
    case 2: ///scan for move-to in peripheral turn
      out_led(_move_from, _orientation);
      square = scan_board(_orientation);
      if (square == 64) {break; } //peripheral do not press any square
      Serial.print("pressed square(move to): ");
      Serial.println(square);

      found = false;
      for (i = 0; i < list_idx; i += 3){ //check if move to legal
        if ((_move_from == move_list[i]) && (square == move_list[i+1])){
          found = true;
          _move_bit = move_list[i + 2];
          break;
        }
      }
      
      if (found){ //legal move
          _move_to = square; //move legal, save move from
          make_move(_move_from, _move_to, _move_bit);
          move_gen();
          move_to_string();
          out_led(_move_to, _orientation);
          if (_board_mode == 3){
            _status = 1;
            scan_counter = 20;
            wait_release = true;
            selected_square = 64;
          }
          else {
            _status = 3; //next move-from from central
          }
      }
      else if (_move_from == square){//cancel move
        Serial.println("move from cancelled");
        wait_release = true;
        selected_square = 64;
        _status = 1;
      }
      else {
        Serial.println("warning, move to not legal!");
        _buzzer_count = 1;
        digitalWrite(buzzer, HIGH);
        _time_limit = millis() + 500;
         wait_release = true;
        selected_square = 64;
      }
      break;
      case 3:
        if (_central_move_from == 64){ //central do not move yet
          if (end_game){_status = 0; _buzzer_count = 1; digitalWrite(buzzer, HIGH), _time_limit = millis() + 1500;}
          break;
         }

        out_led(_central_move_from, _orientation);
        square = scan_board(_orientation);

        if (square == _central_move_from){
          _central_move_from = 64;
          _status = 4; //next move-to from central central
          break;
        }

        if (square == _central_move_to){
          _central_move_from = 64;
          _status = 5;
          break;
        }

        out_led(_central_move_from, _orientation);
        square = scan_board(_orientation);

        if (square == _central_move_from){
          _central_move_from = 64;
          _status = 4; //next move-to from central central
          break;
        }

        if (square == _central_move_to){
          _central_move_from = 64;
          _status = 5;
          break;
        }

        out_led(_central_move_to, _orientation);
        square = scan_board(_orientation);

        if (square == _central_move_from){
          _central_move_from = 64;
          _status = 4; //next move-to from central central
          break;
        }
        
        if (square == _central_move_to){
          _central_move_from = 64;
          _status = 5;
        }
        break;
      case 4:
        out_led(_central_move_to, _orientation);
        square = scan_board(_orientation);
        if (square == _central_move_to){
          if (is_incheck()){
            _buzzer_count = 3;
            digitalWrite(buzzer, HIGH);
            _time_limit = millis() + 500;
          }
          wait_release = true;
          selected_square = 64;
          if (end_game){
            _status = 0;
            _buzzer_count = 1;
            digitalWrite(buzzer, HIGH);
            _time_limit = millis() + 1500;
          }
          else {
            _status = 1;
          }
          
        }
      break;
      case 5:
        if (is_incheck()){
          _buzzer_count = 3;
          digitalWrite(buzzer, HIGH);
          _time_limit = millis() + 500;
        }

        wait_release = true;
        selected_square = 64;
        if (end_game){
          _status = 0;
          _buzzer_count = 1;
          digitalWrite(buzzer, HIGH);
          _time_limit = millis() + 1500;
        }
        else {
          _status = 1;
        }
      break;
      case 0:   //end game_status
        switch(_running_led){
          case 0:
            out_led2(0xffbf, 8);
            break;
          case 1:
            out_led2(0xffbf, 16);
            break;
          case 2:
            out_led2(0xffbf, 32);
            break;
          case 3:
            out_led2(0xfeff, 32);
            break;
          case 4:
            out_led2(0xfeff, 16);
            break;
          case 5:
            out_led2(0xfeff, 8);
        }
        _running_led++; 
        if (_running_led > 5){
          _running_led = 0;
          square = scan_board(0x20);
          if (square != 64){ //press square, select option
              wait_release = true;
              selected_square = 64;
              select_option(square);
          }
        }   
  }
}


class MyPeripheral : public BleChessPeripheral
{
public:
 

  void handleCentralBegin(BleChessStringView fen) override {
    int i, j, k;
    const char* fen_string;
    fen_string = fen.data();
    Serial.print("begin: ");
    Serial.println(fen_string);
    fen_to_board(fen_string);
    print_board();
    

    enpass = 0x0f; //?? enpass tidak sah
    cast = 0xf0; //??
    fifty = 0; //??

    //guess castling right
    if (board[0x3c] != 0x26){cast = cast & 0b11000000;} //white king has moved
    if (board[0x3f] != 0x2c){cast = cast & 0b11100000;} //white king's rook has moved
    if (board[0x38] != 0x2c){cast = cast & 0b11010000;} //white queen's rook has moved

    if (board[0x04] != 0x16){cast = cast & 0b00110000;} //black king has moved
    if (board[0x07] != 0x1c){cast = cast & 0b10110000;} //black king's rook has moved
    if (board[0x00] != 0x1c){cast = cast & 0b01110000;} //black queen's rook has moved

    move_gen();

    shift_register(0xffff);
    digitalWrite(buzzer, LOW);
    sendPeripheralSync(BleChessString(fen));
    _status = 1;
    _move_to = 64;

    end_game = false;
    _game_begin = true;
    _buzzer_count = 1;
    _time_limit = millis() + 1000;
    digitalWrite(buzzer, HIGH);
  }


  void handleCentralMove(BleChessStringView mv) override {
    bool legal;
    Serial.print("central move: ");
    Serial.println(mv.data());
    if ((_board_mode == 1) && _game_begin){ //play vs computer
      _orientation = gilir ^ 0x30;
      if (_orientation == 0x20){ Serial.println("Board orientation: normal");} else {Serial.println("Board orientation: inverted");}
      _game_begin = false;
    }
    make_move_central(mv.data());
    _buzzer_count = 1;
    digitalWrite(buzzer, HIGH);
    _time_limit = millis() + 500;
  }

  void handlePeripheralMoveAck(bool ack) override {
    if (ack){
      print_board();
    }
    else {
      Serial.println("Error, Move_rejected");
      _buzzer_count = 1;
      digitalWrite(buzzer, HIGH);
      _time_limit = millis() + 500;
      _status = 1;
    }
  }
  void handlePeripheralMovePromoted(BleChessStringView prom) override {
    const char* promote_move;
    char piece_letter;
    uint8_t piece;
    promote_move = prom.data();
    Serial.print("promote: ");
    Serial.println(promote_move);
    piece_letter = promote_move[4];

    if (gilir == 0x10){ //convert to upper case
      piece_letter = piece_letter - 32;
    }

    piece = piece_sym(piece_letter);
    if (piece != 0){
      board[_move_to] = piece;
      move_gen();
    }
    
  }
  
  void handleCentralEnd(BleChessStringView reason) override {
    Serial.print("end: ");
    Serial.println(reason.data());
    end_game  = true;
    _running_led = 0;
  }

  void checkPeripheralMove() {
    board_move();
    if (_move_string[0] != '\0'){
      BleChessString move(_move_string);
      Serial.print("send move: ");
      Serial.println(_move_string);
      sendPeripheralMove(move);
      _move_string[0] = '\0';
    }
  }
};