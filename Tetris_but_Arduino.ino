// Tetris but Arduino
// Ray Hang
// 2022-12-22
// TER4M1

// CONTROLS:
// 0: Reset
// 2: Rotate
// 4: Move Left
// 6: Move Right
// 8: Soft Drop
// 7: Hard Drop

#include <LiquidCrystal.h> // include LCD library
#include <IRremote.h>  // include IR remote library

LiquidCrystal lcd(1, 2, 4, 5, 6, 7); // initialise LCD

bool FullBoard[8][40]; // initialise boolean array to save the state of the game

// initialise the (X, Y) coordinates of the four squares in the currently controlled Tetromino
int8_t Block1X, Block1Y;
int8_t Block2X, Block2Y;
int8_t Block3X, Block3Y;
int8_t Block4X, Block4Y;

long LinesCleared; // initialise score counter

long PreviousMove; // initialise the movement tracker

int BlockDelay = 101; // initialise the delay between blocks updating
long PreviousMillis = 0; // initialise the time keeper

int8_t CurrentBlockType; // initialise the block type
// 1: I, 2: O, 3: J, 4: L, 5: S, 6: T, 7: Z

// converts 5 boolean values into a decimal representation of a binary number
// sufficient for a single row in one cell of the LCD
int8_t BinaryToDecimal(bool a1, bool a2, bool a3, bool a4, bool a5) {
  return int8_t(a1 ? 16 : 0) + int8_t(a2 ? 8 : 0) + int8_t(a3 ? 4 : 0) + int8_t(a4 ? 2 : 0) + int8_t(a5 ? 1 : 0);
}

// uses the NES Tetris formula to generate the next block type
int8_t SelectNextBlock(int8_t oldBlock) {
  int8_t newBlock = random(1, 8);

  if (newBlock == 8 || newBlock == oldBlock)
    newBlock = random(1, 7);

  return newBlock;
}

// generates and places a new block
void PlaceNewBlock() {
  CurrentBlockType = SelectNextBlock(CurrentBlockType); // generates a new block

  // I block
  if (CurrentBlockType == 1) {
    Block1X = Block2X = Block3X = Block4X = 4;
    Block1Y = 39;
    Block2Y = 38;
    Block3Y = 37;
    Block4Y = 36;
  }
  // O block
  else if (CurrentBlockType == 2) {
    Block1X = Block2X = 3;
    Block3X = Block4X = 4;
    Block1Y = Block3Y = 39;
    Block2Y = Block4Y = 38;
  }
  // J block
  else if (CurrentBlockType == 3) {
    Block1X = Block2X = Block3X = 4;
    Block4X = 3;
    Block1Y = 39;
    Block2Y = 38;
    Block3Y = 37;
    Block4Y = 37;
  }
  // L block
  else if (CurrentBlockType == 4) {
    Block1X = Block2X = Block3X = 4;
    Block4X = 5;
    Block1Y = 39;
    Block2Y = 38;
    Block3Y = 37;
    Block4Y = 37;
  }
  // S block
  else if (CurrentBlockType == 5) {
    Block1X = 3;
    Block2X = Block3X = 4;
    Block4X = 5;
    Block1Y = 38;
    Block2Y = 38;
    Block3Y = 39;
    Block4Y = 39;
  }
  // T block
  else if (CurrentBlockType == 6) {
    Block1X = 5;
    Block2X = Block4X = 4;
    Block3X = 3;
    Block1Y = 38;
    Block2Y = 38;
    Block3Y = 38;
    Block4Y = 39;
  }
  // Z block
  else if (CurrentBlockType == 7) {
    Block1X = 3;
    Block2X = Block3X = 4;
    Block4X = 5;
    Block1Y = 39;
    Block2Y = 39;
    Block3Y = 38;
    Block4Y = 38;
  }
}

// redraws the game board
void DrawGrid() {
  // temporarily place the player blocks
  FullBoard[Block1X][Block1Y] = true;
  FullBoard[Block2X][Block2Y] = true;
  FullBoard[Block3X][Block3Y] = true;
  FullBoard[Block4X][Block4Y] = true;

  // uses FullBoard to generate 8 custom characters and print them
  for (int8_t block = 0; block < 8; block++) {
    byte LCDBlock[8] = {};
    for (int8_t row = 0; row < 8; row++)
      LCDBlock[row] = BinaryToDecimal(FullBoard[row][0 + block * 5], FullBoard[row][1 + block * 5], FullBoard[row][2 + block * 5], FullBoard[row][3 + block * 5], FullBoard[row][4 + block * 5]);
    lcd.createChar(block, LCDBlock);
    lcd.setCursor(block, 0);
    lcd.write(byte(block));
  }

  // removes the player blocks
  FullBoard[Block1X][Block1Y] = false;
  FullBoard[Block2X][Block2Y] = false;
  FullBoard[Block3X][Block3Y] = false;
  FullBoard[Block4X][Block4Y] = false;
}

// checks for line clears and updates the board
void UpdateScore() {
  // checks each line of the board
  for (int8_t col = 0; col < 40; col++) {
    bool ClearedLine = true;
    for (int8_t row = 0; row < 8; row++) {
      if (!FullBoard[row][col]) {
        ClearedLine = false;
        break;
      }
    }
    if (ClearedLine) {
      // updates the score
      LinesCleared++;
      PrintScore();

      // increases the speed slightly
      BlockDelay = max(1, BlockDelay - 5);

      // clears the line
      for (int8_t row = 0; row < 8; row++)
        FullBoard[row][col] = false;

      // moves the rest of the board down one block
      for (int8_t col1 = col + 1; col1 < 40; col1++)
        for (int8_t row = 0; row < 8; row++)
          FullBoard[row][col1 - 1] = FullBoard[row][col1];
      col--;
    }
  }
}

// collision check and block movement
void UpdateGame() {
  // updates the player block via a delay
  if (millis() - PreviousMillis > BlockDelay) {
    PreviousMillis = millis();
    MoveDown();
  }
  // too far to the right
  while (Block1X > 7 || Block2X > 7 || Block3X > 7 || Block4X > 7) {
    Block1X--;
    Block2X--;
    Block3X--;
    Block4X--;
  }
  // too far to the left
  while (Block1X < 0 || Block2X < 0 || Block3X < 0 || Block4X < 0) {
    Block1X++;
    Block2X++;
    Block3X++;
    Block4X++;
  }
  // too far up
  while (Block1Y > 39 || Block2Y > 39 || Block3Y > 39 || Block4Y > 39) {
    Block1Y--;
    Block2Y--;
    Block3Y--;
    Block4Y--;
  }
  // too far down
  while (Block1Y < 0 || Block2Y < 0 || Block3Y < 0 || Block4Y < 0) {
    Block1Y++;
    Block2Y++;
    Block3Y++;
    Block4Y++;
  }
  // checking for collisions with other blocks or if topped out
  bool collided = false;

  // if topped out, finish the game
  if (Block1Y > 39 || Block2Y > 39 || Block3Y > 39 || Block4Y > 39) {
    FinishGame(false);
    return;
  }

  // if in another block, shove upwards and place
  while (Block1Y < 40 && Block2Y < 40 && Block3Y < 40 && Block4Y < 40 && (FullBoard[Block1X][Block1Y] || FullBoard[Block2X][Block2Y] || FullBoard[Block3X][Block3Y] || FullBoard[Block4X][Block4Y])) {
    Block1Y++;
    Block2Y++;
    Block3Y++;
    Block4Y++;
    if (Block1Y == 39 || Block2Y == 39 || Block3Y == 39 || Block4Y == 39) {
      FinishGame(false);
      return;
    }

    collided = true;
  }

  // if topped out, finish the game check #2
  if (Block1Y > 39 || Block2Y > 39 || Block3Y > 39 || Block4Y > 39) {
    FinishGame(false);
    return;
  }

  // if on ground, place the block
  if (Block1Y == 0 || Block2Y == 0 || Block3Y == 0 || Block4Y == 0)
    collided = true;

  // place the block if flag caught, then place a new one
  if (collided) {
    // finalise the player Tetromino placement
    FullBoard[Block1X][Block1Y] = true;
    FullBoard[Block2X][Block2Y] = true;
    FullBoard[Block3X][Block3Y] = true;
    FullBoard[Block4X][Block4Y] = true;

    // check if there is a line clear
    UpdateScore();

    // place the next block
    PlaceNewBlock();
  }
}

// finish the game by displaying the score, waiting 5 seconds unless otherwise specified, clearing the board, and starting again
void FinishGame(bool reset) {
  // write the final score
  UpdateScore();
  PrintScore();

  lcd.setCursor(0, 1);
  lcd.print(" Game ! ");

  if (!reset)
    delay(5000);

  // clear the board
  for (int8_t i = 0; i < 8; i++)
    memset(FullBoard[i], false, 40);

  LinesCleared = 0;
  BlockDelay = 101;
  PrintScore();

  // start a new round
  PlaceNewBlock();
  DrawGrid();
}

void PrintScore() {

  // clean up boundaries
  lcd.setCursor(8, 0);
  for (int8_t i = 0; i < 8; i++)
    lcd.write(252);
  lcd.setCursor(0, 1);
  for (int8_t i = 0; i < 16; i++)
    lcd.write(252);

  // write the score
  lcd.setCursor(9, 0);
  lcd.print(F("Lines:"));
  lcd.setCursor(9, 1);
  if (LinesCleared < 10)
    lcd.print(F("00000"));
  else if (LinesCleared < 100)
    lcd.print(F("0000"));
  else if (LinesCleared < 1000)
    lcd.print(F("000"));
  else if (LinesCleared < 10000)
    lcd.print(F("00"));
  else if (LinesCleared < 100000)
    lcd.print(F("0"));
  lcd.print(LinesCleared);
}

// controls player movement of the piece via remote
void Movement(long ButtonPressed) {
  if (ButtonPressed != 0) {
    switch (ButtonPressed) {
      case -417792256: // keypad "2", rotate
        RotateBlock();
        break;

      case -150405376: // keypad "4", move left
        MoveLeft();
        break;

      case -1520763136: // keypad "6", move right
        MoveRight();
        break;

      case -1387069696: // keypad "8", move down
        MoveDown();
        break;

      case -1119682816: // keypad "7", hard drop
        HardDrop();
        break;

      case -384368896:
        FinishGame(true);
        break;

      default:
        break;
    }
  } else {
    switch (PreviousMove) {
      case -417792256: // keypad "2", rotate
        RotateBlock();
        break;

      case -150405376: // keypad "4", move left
        MoveLeft();
        break;

      case -1520763136: // keypad "6", move right
        MoveRight();
        break;

      case -1387069696: // keypad "8", move down
        MoveDown();
        break;

      case -1119682816: // keypad "7", hard drop
        HardDrop();
        break;

      case -384368896:
        FinishGame(true);
        break;

      default:
        break;
    }
  }
  DrawGrid();
}

// moves the player Tetromino down by one block
void MoveDown() {
  Block1Y--;
  Block2Y--;
  Block3Y--;
  Block4Y--;
}

void HardDrop() {
  while (Block1Y > -1 && Block2Y > -1 && Block3Y > -1 && Block4Y > -1 && !FullBoard[Block1X][Block1Y - 1] && !FullBoard[Block2X][Block2Y - 1] && !FullBoard[Block3X][Block3Y - 1] && !FullBoard[Block4X][Block4Y - 1]) {
    Block1Y--;
    Block2Y--;
    Block3Y--;
    Block4Y--;
  }
}

// moves the player Tetromino right by one block
void MoveRight() {
  if ((Block1X + 1) < 8 && (Block2X + 1) < 8 && (Block3X + 1) < 8 && (Block4X + 1) < 8 && !FullBoard[Block1X + 1][Block1Y] && !FullBoard[Block2X + 1][Block2Y] && !FullBoard[Block3X + 1][Block3Y] && !FullBoard[Block4X + 1][Block4Y]) {
    Block1X++;
    Block2X++;
    Block3X++;
    Block4X++;
  }
}

// moves the player Tetromino left by one block
void MoveLeft() {
  if ((Block1X - 1) > -1 && (Block2X - 1) > -1 && (Block3X - 1) > -1 && (Block4X - 1) > -1 && !FullBoard[Block1X - 1][Block1Y] && !FullBoard[Block2X - 1][Block2Y] && !FullBoard[Block3X - 1][Block3Y] && !FullBoard[Block4X - 1][Block4Y]) {
    Block1X--;
    Block2X--;
    Block3X--;
    Block4X--;
  }
}

void RotateBlock() {
  // I block
  if (CurrentBlockType == 1) {
    // horizontal to vertical
    if (Block1Y == Block2Y) {
      Block1X = Block3X;
      Block2X = Block3X;
      Block4X = Block3X;
      Block1Y = Block3Y + 2;
      Block2Y = Block3Y + 1;
      Block4Y = Block3Y - 1;
    }
    // vertical to horizontal
    else {
      Block1X = Block3X - 2;
      Block2X = Block3X - 1;
      Block4X = Block3X + 1;
      Block1Y = Block3Y;
      Block2Y = Block3Y;
      Block4Y = Block3Y;
    }
  }
  // J block
  else if (CurrentBlockType == 3) {
    if (Block1X == Block2X - 1) {
      Block1X = Block2X;
      Block3X = Block2X;
      Block4X = Block2X - 1;
      Block1Y = Block2Y + 1;
      Block3Y = Block2Y - 1;
    } else if (Block1Y == Block2Y + 1) {
      Block1X = Block2X + 1;
      Block3X = Block2X - 1;
      Block1Y = Block2Y;
      Block3Y = Block2Y;
      Block4Y = Block2Y + 1;
    } else if (Block1X == Block2X + 1) {
      Block1X = Block2X;
      Block3X = Block2X;
      Block4X = Block2X + 1;
      Block1Y = Block2Y - 1;
      Block3Y = Block2Y + 1;
    } else {
      Block1X = Block2X - 1;
      Block3X = Block2X + 1;
      Block1Y = Block2Y;
      Block3Y = Block2Y;
      Block4Y = Block2Y - 1;
    }
  }
  // L block
  else if (CurrentBlockType == 4) {
    if (Block1X == Block2X + 1) {
      Block1X = Block2X;
      Block3X = Block2X;
      Block1Y = Block2Y - 1;
      Block3Y = Block2Y + 1;
      Block4Y = Block2Y + 1;
    } else if (Block1Y == Block2Y - 1) {
      Block1X = Block2X - 1;
      Block3X = Block2X + 1;
      Block4X = Block2X + 1;
      Block1Y = Block2Y;
      Block3Y = Block2Y;
      Block4Y = Block2Y + 1;
    } else if (Block1X == Block2X - 1) {
      Block1X = Block2X;
      Block3X = Block2X;
      Block1Y = Block2Y + 1;
      Block3Y = Block2Y - 1;
      Block4Y = Block2Y - 1;
    } else {
      Block1X = Block2X + 1;
      Block3X = Block2X - 1;
      Block4X = Block2X - 1;
      Block1Y = Block2Y;
      Block3Y = Block2Y;
      Block4Y = Block2Y - 1;
    }
  }
  // S block
  else if (CurrentBlockType == 5) {
    if (Block1Y == Block2Y) {
      Block1X = Block3X;
      Block2X = Block3X + 1;
      Block1Y = Block3Y + 1;
      Block2Y = Block3Y;
      Block4Y = Block3Y - 1;
    } else {
      Block1X = Block3X - 1;
      Block2X = Block3X;
      Block1Y = Block3Y - 1;
      Block2Y = Block3Y - 1;
      Block4Y = Block3Y;
    }
  }
  // T block
  else if (CurrentBlockType == 6) {
    if (Block1X == Block2X - 1) {
      Block1X = Block2X;
      Block3X = Block2X;
      Block4X = Block2X - 1;
      Block1Y = Block2Y + 1;
      Block3Y = Block2Y - 1;
      Block4Y = Block2Y;
    } else if (Block1Y == Block2Y + 1) {
      Block1X = Block2X + 1;
      Block3X = Block2X - 1;
      Block4X = Block2X;
      Block1Y = Block2Y;
      Block3Y = Block2Y;
      Block4Y = Block2Y + 1;
    } else if (Block1X == Block2X + 1) {
      Block1X = Block2X;
      Block3X = Block2X;
      Block4X = Block2X + 1;
      Block1Y = Block2Y - 1;
      Block3Y = Block2Y + 1;
      Block4Y = Block2Y;
    } else {
      Block1X = Block2X - 1;
      Block3X = Block2X + 1;
      Block4X = Block2X;
      Block1Y = Block2Y;
      Block3Y = Block2Y;
      Block4Y = Block2Y - 1;
    }
  }
  // Z block
  else if (CurrentBlockType == 7) {
    if (Block1Y == Block2Y) {
      Block1X = Block2X + 1;
      Block1Y = Block2Y + 1;
      Block4Y = Block2Y;
    } else {
      Block1X = Block2X - 1;
      Block1Y = Block2Y;
      Block4Y = Block2Y - 1;
    }
  }
  UpdateGame();
}

void setup() {
  // set up lcd
  lcd.begin(16, 2);

  // set up remote
  IrReceiver.begin(13, ENABLE_LED_FEEDBACK);

  // set up the scoreboard
  PrintScore();

  // start the game
  PlaceNewBlock();
}

void loop() {
  // decode instructions
  if (IrReceiver.decode()) {
    long Value = IrReceiver.decodedIRData.decodedRawData;
    if (Value != 0)
      PreviousMove = Value;
    Movement(Value);
    IrReceiver.resume();
  }

  // update the game state
  UpdateGame();

  // draw the grid
  DrawGrid();
}
