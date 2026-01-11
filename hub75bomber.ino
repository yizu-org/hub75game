#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "spritebank.h"

#define SOUND_PIN 33     //声音引脚
// ================= 配置矩阵参数 =================
// 根据你的面板规格调整宽度和高度
#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 64

MatrixPanel_I2S_DMA *dma_display = nullptr;
uint16_t myColor = 0;//定义游戏画面颜色
int fire_milliseconds = 100;
unsigned long fire_time = 0;
unsigned long currentTime;
bool button_A, button_B, UP, DOWN, LEFT, RIGHT;//控制按键

int8_t BOMBXY[4] = { -10, -10, 0, 0 };
uint8_t live;
uint8_t INGAME;
int8_t Level;
uint8_t Frame;
uint8_t BlocBombMem[14];

void LoadLevel(uint8_t Level) {
  for (uint8_t t = 0; t < 14; t++) {
    BlocBombMem[t] = pgm_read_byte(&level[t + (Level * 14)]);
  }
}

void ResetVar(PERSONAGE *Sprite) {
  live = 3;
  Level = -1;
  INGAME = 0;
  resetMonster(Sprite);
}

void resetMonster(PERSONAGE *Sprite) {
  Sprite[1].dead = 0;
  Sprite[2].dead = 0;
  Sprite[3].dead = 0;
  Sprite[4].dead = 0;
}

void resetBomb(void) {
  BOMBXY[0] = -10;
  BOMBXY[1] = -10;
  BOMBXY[2] = 0;
  BOMBXY[3] = 0;
  BOMBXY[4] = 0;
  BOMBXY[5] = 0;
}

void readButton(){
  //读取游戏控制信号
  if(Serial1.available() > 0) {
    byte receivedData = Serial1.read();
    button_A = bitRead(receivedData, 0);
    button_B = bitRead(receivedData, 1);
    UP = bitRead(receivedData, 2);
    DOWN = bitRead(receivedData, 3);
    LEFT = bitRead(receivedData, 4);
    RIGHT = bitRead(receivedData, 5);
  }
}

void setup() {
  pinMode(SOUND_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("游戏初始化...");
  // 初始化 UART1,用于接收手柄控制信号，RX 引脚为 35, TX 引脚为 32
  Serial1.setTimeout(50);
  Serial1.begin(115200, SERIAL_8N1, 35, 32);
  // 初始化矩阵配置
  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_width = MATRIX_WIDTH;
  mxconfig.mx_height = MATRIX_HEIGHT;
  mxconfig.chain_length = 2;
  mxconfig.gpio.r1 = 25;
  mxconfig.gpio.g1 = 26;
  mxconfig.gpio.b1 = 27;
  mxconfig.gpio.r2 = 14;
  mxconfig.gpio.g2 = 12;
  mxconfig.gpio.b2 = 13;
  mxconfig.gpio.a = 23;
  mxconfig.gpio.b = 19;
  mxconfig.gpio.c = 5;
  mxconfig.gpio.d = 17;
  mxconfig.gpio.e = 18;
  mxconfig.gpio.lat = 4;
  mxconfig.gpio.oe = 15;
  mxconfig.gpio.clk = 16;
  //mxconfig.double_buff = true; // 启用双缓冲
  // 初始化驱动
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(50); // 设置亮度 (0-255)

  myColor = dma_display->color565(0, 200, 0);//定义游戏画面颜色（单色）
  dma_display->fillScreen(myColor);
  delay(1000);
}

void StartGame(void) {
  if (INGAME == 0) {
    while (1) {
      if (!button_A) { break; }
    }
    INGAME = 1;
  }
}

void loop() {
  currentTime = millis();
  PERSONAGE Sprite[5];
NEWGAME:
  ResetVar(&Sprite[0]);
  while (1) {
    Tiny_Flip(1, &Sprite[0]);
    if (button_A) {
      StartGame();
      break;
    }
  }
NEWLEVEL:
  if (Level < 2) {
    if (Level > -1) {
      {
        for (uint8_t t = 0; t <= 4; t++) {
          Sound(80, 100);
        }
      }
    }
    Level++;
  } else {
    Level = 0;
  }
  resetMonster(Sprite);
  LoadLevel(Level);
RESTARTLEVEL:
  resetBomb();
  Sprite[0].type = 0;
  Sprite[0].x = 9;
  Sprite[0].y = 7;
  Sprite[0].Decalagey = 0;
  Sprite[0].DirectionV = 2;
  Sprite[0].DirectionH = 2;
  Sprite[0].DirectionAnim = 0;
  Sprite[0].dead = 0;
  Sprite[1].type = 1;
  Sprite[1].x = 56;
  Sprite[1].y = 1;
  Sprite[1].Decalagey = 0;
  Sprite[2].type = 1;
  Sprite[2].x = 120;
  Sprite[2].y = 2;
  Sprite[2].Decalagey = 0;
  Sprite[3].type = 1;
  Sprite[3].x = 119;
  Sprite[3].y = 7;
  Sprite[3].Decalagey = 0;
  Sprite[4].type = 1;
  Sprite[4].x = 24;
  Sprite[4].y = 5;
  Sprite[4].Decalagey = 0;
  //INGAME:
  while (1) {
    if (button_A) {
      StartGame(); 
      }
    if (INGAME) {
      //方向控制
      if (LEFT) {
        Sprite[0].DirectionV = 0;
      } else if (RIGHT) {
        Sprite[0].DirectionV = 1;
      } else {
        if (((Sprite[0].x) % 8) == 0) Sprite[0].DirectionV = 2;
      }
      if (DOWN) {
        Sprite[0].DirectionH = 1;
      } else if (UP) {
        Sprite[0].DirectionH = 0;
      } else {
        if (Sprite[0].Decalagey == 0) Sprite[0].DirectionH = 2;
      }
      //方向控制结束
      if (button_A && (BOMBXY[2] == 0)) {
        BOMBXY[0] = (uint8_t((Sprite[0].x + 3) / 8)) * 8;
        BOMBXY[1] = (uint8_t(((Sprite[0].y * 8) + (Sprite[0].Decalagey) + 3) / 8));
        BOMBXY[2] = 5;
        {
          uint8_t t;
          for (t = 155; t < 255; t++) {
            Sound(2 + t, 2);
            Sound(25 + t, 1);
          }
        }
      }
    }
    if (Frame < 24) {
      Frame++;
    } else {
      Frame = 0;
    }
    if (CollisionTiny2Caracter(&Sprite[0]) == 0) {
      if (Frame % 2 == 0) { RefreshCaracter(&Sprite[0]); }
    } else {
      DeadSong();
      if (live > 0) {
        live--;
        goto RESTARTLEVEL;
      } else {
        //delay(200);
        {
          for (uint8_t t = 0; t < 5; t++) {
            Sound(100, 100);
            Sound(1, 100);
          }
        }
        goto NEWGAME;
      }
    }
    if ((Frame == 0) || (Frame == 8) || (Frame == 16)) {
      if (BOMBXY[2] > 1) {
        if (BOMBXY[3] < 2) {
          BOMBXY[3]++;
        } else {
          BOMBXY[3] = 0;
          BOMBXY[2]--;
        }
      }
    }
    if (Frame % 4 == 0) {
      if (BOMBXY[2] == 1) {
        if (BOMBXY[3] < 2) {
          if (BOMBXY[3] == 1) {
            DestroyEnemy(&Sprite[0]);
            DestroyBloc();
          }
          BOMBXY[3]++;
        } else {
          resetBomb();
          for (uint8_t t = 210; t < 255; t++) {
            Sound(2 + t, 1);
            Sound(255 - t, 2);
          }
        }
      }
    }
    if (Frame % 2 == 0) {
      Tiny_Flip(0, &Sprite[0]);
      if (INGAME == 1) {
        for (uint8_t t = 0; t < 42; t = t + 2) { Sound(pgm_read_byte(&Music[t]), pgm_read_byte(&Music[t + 1])); }
        INGAME = 2;
      }
    }
    if (Sprite[0].dead == 1) {
      if (live > 0) {
        DeadSong();
        live--;
        goto RESTARTLEVEL;
      } else {
        DeadSong();
        {
          for (uint8_t t = 0; t < 5; t++) {
            Sound(100, 100);
            Sound(1, 100);
          }
        }
        goto NEWGAME;
      }
    }
    if (Frame == 12) {
      if (CheckLevelEnded(&Sprite[0]) == 1) goto NEWLEVEL;
    }
    delay(10);
  }
}
////////////////////////////////// main end /////////////////////////////////

uint8_t CheckLevelEnded(PERSONAGE *Sprite) {
  for (uint8_t t = 1; t <= 4; t++) {
    if (Sprite[t].dead == 0) { return 0; }
  }
  return 1;
}

void DeadSong(void) {
  for (uint8_t t = 1; t < 255; t++) {
    Sound(t, 1);
    Sound(255 - t, 2);
  }
}

void DestroyBloc(void) {
  uint8_t x = ((BOMBXY[0] - 8) / 8);
  uint8_t y = (BOMBXY[1] - 1);
  if (y != 0) BOOLWRITE0(x + ((y - 1) * 15));
  BOOLWRITE0(x + ((y)*15));
  if (y != 6) BOOLWRITE0(x + ((y + 1) * 15));
  if (x != 0) BOOLWRITE0((x - 1) + ((y)*15));
  if (x != 14) BOOLWRITE0((x + 1) + ((y)*15));
}

void DestroyEnemy(PERSONAGE *Sprite) {
#define xminf(I) (Sprite[I].x)
#define yminf(I) ((Sprite[I].y * 8) + Sprite[I].Decalagey)
  for (uint8_t t = 0; t <= 4; t++) {
    if (COLLISION11(xminf(t), yminf(t), 7, 7, BOMBXY[0] + 1, (BOMBXY[1] * 8) - 6, 6, 21)) {
      Sprite[t].y = 0;
      Sprite[t].Decalagey = 0;
      if (Sprite[t].dead != 1) { Sprite[t].dead = 1; }
    }
  }
  for (uint8_t t = 0; t <= 4; t++) {
    if (COLLISION11(xminf(t), yminf(t), 7, 7, BOMBXY[0] - 6, (BOMBXY[1] * 8) + 1, 21, 6)) {
      Sprite[t].y = 0;
      Sprite[t].Decalagey = 0;
      if (Sprite[t].dead != 1) { Sprite[t].dead = 1; }
    }
  }
}

uint8_t COLLISION11(uint8_t x1, uint8_t y1, uint8_t w1, uint8_t h1, uint8_t x2, uint8_t y2, uint8_t w2, uint8_t h2) {
  if ((x1 + w1 < x2) || (y1 + h1 < y2) || (x1 > x2 + w2) || (y1 > y2 + h2)) {
    return 0;
  } else {
    return 1;
  }
}

uint8_t CollisionTiny2Caracter(PERSONAGE *Sprite) {
  uint8_t ReturnCollision = 0;
#define xmax(I) (Sprite[I].x + 6)
#define xmin(I) (Sprite[I].x)
#define ymax(I) ((Sprite[I].y * 8) + Sprite[I].Decalagey + 6)
#define ymin(I) ((Sprite[I].y * 8) + Sprite[I].Decalagey)
  if ((INGAME)) {
    for (uint8_t t = 1; t <= 4; t++) {
      if (Sprite[t].dead != 1) {
        if ((xmax(0) < xmin(t)) || (xmin(0) > xmax(t)) || (ymax(0) < ymin(t)) || (ymin(0) > ymax(t))) {
        } else {
          ReturnCollision = 1;
        }
      }
    }
  }
  return ReturnCollision;
}

void RefreshCaracter(PERSONAGE *Sprite) {
  uint8_t memx, memy, memdecalagey;
  for (uint8_t t = 0; t <= 4; t++) {
    memx = Sprite[t].x;
    memy = Sprite[t].y;
    memdecalagey = Sprite[t].Decalagey;
    if (Sprite[t].DirectionV == 1) { Sprite[t].x++; }
    if (Sprite[t].DirectionV == 0) { Sprite[t].x--; }
    if (CheckCollisionWithBack(t, 1, Sprite)) {
      if (t != 0) {
        Sprite[t].DirectionV = random() % 2;
      } else {
        Sprite[t].DirectionV = 2;
      }
      Sprite[t].x = memx;
    }
    if (Sprite[t].DirectionH == 1) {
      if (Sprite[t].Decalagey < 7) {
        Sprite[t].Decalagey++;
      } else {
        Sprite[t].Decalagey = 0;
        Sprite[t].y++;
      }
    }
    if (Sprite[t].DirectionH == 0) {
      if (Sprite[t].Decalagey > 0) {
        Sprite[t].Decalagey--;
      } else {
        Sprite[t].Decalagey = 7;
        Sprite[t].y--;
      }
    }
    if (CheckCollisionWithBack(t, 0, Sprite)) {
      if (t != 0) {
        Sprite[t].DirectionH = random() % 2;
      } else {
        Sprite[t].DirectionH = 2;
      }
      Sprite[t].y = memy;
      Sprite[t].Decalagey = memdecalagey;
    }
    if (t == 0) {
      if (Frame % 2 == 0) {
        if (Sprite[t].DirectionH == 1) { Sprite[t].DirectionAnim = 0; }
        if (Sprite[t].DirectionH == 0) { Sprite[t].DirectionAnim = (2 * 3); }
        if (Sprite[t].DirectionV == 1) { Sprite[t].DirectionAnim = (3 * 3); }
        if (Sprite[t].DirectionV == 0) { Sprite[t].DirectionAnim = (1 * 3); }
      }
    } else {
      if ((Frame == 0) || (Frame == 12)) {
        Sprite[t].DirectionAnim = 0;
        if (Sprite[t].DirectionH == 1) { Sprite[t].DirectionAnim = 0; }
        if (Sprite[t].DirectionH == 0) { Sprite[t].DirectionAnim = 0; }
      }
    }
    if (t == 0) {
      if (Frame % 2 == 0) {
        if (Sprite[0].switchanim == 0) {
          if ((Sprite[0].anim < 2) && ((Sprite[t].DirectionH != 2) || (Sprite[t].DirectionV != 2))) {
            Sprite[0].anim++;
          } else {
            Sprite[0].switchanim = 1;
          }
        } else {
          if (Sprite[0].anim > 0) {
            Sprite[0].anim--;
          } else {
            Sprite[0].switchanim = 0;
          }
        }
      }
    } else {
      if (Frame % 6 == 0) {
        if (Sprite[t].anim < 2) {
          Sprite[t].anim++;
        } else {
          Sprite[t].anim = 0;
        }
      }
    }
  }
}
uint8_t CheckCollisionWithBack(uint8_t SpriteCheck, uint8_t HorVcheck, PERSONAGE *Sprite) {
  uint8_t BacktoComp;
  if (HorVcheck == 1) {
    BacktoComp = RecupeBacktoCompV(SpriteCheck, Sprite);
  } else {
    BacktoComp = RecupeBacktoCompH(SpriteCheck, Sprite);
  }
  BacktoComp = BacktoComp + CheckCollisionWithBlock(SpriteCheck, Sprite);
  BacktoComp = BacktoComp + CheckCollisionWithBomb(SpriteCheck, Sprite);
  if ((BacktoComp) != 0) {
    return 1;
  } else {
    return 0;
  }
}
uint8_t CheckCollisionWithBomb(uint8_t SpriteCheck, PERSONAGE *Sprite) {
  if (SpriteCheck == 0) { return 0; }
  return COLLISION11(Sprite[SpriteCheck].x, (Sprite[SpriteCheck].y * 8) + Sprite[SpriteCheck].Decalagey, 7, 7, BOMBXY[0], (BOMBXY[1] * 8), 7, 7);
}

uint8_t CheckCollisionWithBlock(uint8_t SpriteCheck, PERSONAGE *Sprite) {
#define A (Sprite[SpriteCheck].x)
#define AA ((A + 7))
  uint8_t B = ((((Sprite[SpriteCheck].y - 1) * 8) + Sprite[SpriteCheck].Decalagey) / 8);
  uint8_t BB = (((((Sprite[SpriteCheck].y - 1) * 8) + Sprite[SpriteCheck].Decalagey) + 7) / 8);
  uint8_t Rest = 0;
  uint8_t ReadBloc = pgm_read_byte(&BlocDetect[A]);
  if ((ReadBloc != 0xff) && (ReadBloc != 0xfE)) {
    Rest = Rest + BOOLREAD(ReadBloc + (B * 15));
    Rest = Rest + BOOLREAD(ReadBloc + (BB * 15));
  }
  ReadBloc = pgm_read_byte(&BlocDetect[AA]);
  if ((ReadBloc != 0xff) && (ReadBloc != 0xfE)) {
    Rest = Rest + BOOLREAD(ReadBloc + (B * 15));
    Rest = Rest + BOOLREAD(ReadBloc + (BB * 15));
  }
  return Rest;
}

uint8_t RecupeBacktoCompV(uint8_t SpriteCheck, PERSONAGE *Sprite) {
  uint8_t Y1 = 0b00000000;
  uint8_t Y2 = Y1;
#define SpriteWide 6
#define MAXV (Sprite[SpriteCheck].x + SpriteWide)
#define MINV (Sprite[SpriteCheck].x)
  if (Sprite[SpriteCheck].DirectionV == 1) {
    Y1 = pgm_read_byte(&back[((Sprite[SpriteCheck].y) * 128) + (MAXV)]);
    Y2 = pgm_read_byte(&back[((Sprite[SpriteCheck].y + 1) * 128) + (MAXV)]);
  } else if (Sprite[SpriteCheck].DirectionV == 0) {
    Y1 = pgm_read_byte(&back[((Sprite[SpriteCheck].y) * 128) + (MINV)]);
    Y2 = pgm_read_byte(&back[((Sprite[SpriteCheck].y + 1) * 128) + (MINV)]);
  } else {
    Y1 = 0;
    Y2 = 0;
  }
  //decortique
  Y1 = Trim(0, Y1, Sprite[SpriteCheck].Decalagey);
  Y2 = Trim(1, Y2, Sprite[SpriteCheck].Decalagey);
  if (((Y1) != 0b00000000) || ((Y2) != 0b00000000)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t Trim(uint8_t Y1orY2, uint8_t TrimValue, uint8_t Decalage) {
  uint8_t Comp;
  if (Y1orY2 == 0) {
    Comp = 0b01111111 << Decalage;
    return (TrimValue & Comp);
  } else {
    Comp = (0b01111111 >> (8 - Decalage));
    return (TrimValue & Comp);
  }
}


uint8_t ScanHRecupe(uint8_t UporDown, uint8_t Decalage) {
  if (UporDown == 0) {
    return 0b01111111 << Decalage;
  } else {
    return 0b01111111 >> (8 - Decalage);
  }
}

uint8_t RecupeBacktoCompH(uint8_t SpriteCheck, PERSONAGE *Sprite) {
  uint8_t TempPGMByte;
  if (Sprite[SpriteCheck].DirectionH == 0) {
    uint8_t RECUPE = (ScanHRecupe(0, Sprite[SpriteCheck].Decalagey));
    for (uint8_t t = 0; t <= 6; t++) {
      if ((((Sprite[SpriteCheck].y) * 128) + (Sprite[SpriteCheck].x + t) > 1023) || (((Sprite[SpriteCheck].y) * 128) + (Sprite[SpriteCheck].x + t) < 0)) {
        TempPGMByte = 0x00;
      } else {
        TempPGMByte = (pgm_read_byte(&back[((Sprite[SpriteCheck].y) * 128) + (Sprite[SpriteCheck].x + t)]));
      }
#define CHECKCOLLISION ((RECUPE) & (TempPGMByte))
      if (CHECKCOLLISION != 0) { return 1; }
    }
  } else if (Sprite[SpriteCheck].DirectionH == 1) {
    uint8_t tadd = 0;
    if (Sprite[SpriteCheck].Decalagey > 2) {
      tadd = 1;
    } else {
      tadd = 0;
    }
    uint8_t RECUPE = (ScanHRecupe(tadd, Sprite[SpriteCheck].Decalagey));
    for (uint8_t t = 0; t <= 6; t++) {
      if (((((Sprite[SpriteCheck].y + tadd) * 128) + (Sprite[SpriteCheck].x + t)) > 1023) || ((((Sprite[SpriteCheck].y + tadd) * 128) + (Sprite[SpriteCheck].x + t)) < 0)) {
        TempPGMByte = 0x00;
      } else {
        TempPGMByte = pgm_read_byte(&back[((Sprite[SpriteCheck].y + tadd) * 128) + (Sprite[SpriteCheck].x + t)]);
      }
#define CHECKCOLLISION2 ((RECUPE) & (TempPGMByte))
      if (CHECKCOLLISION2 != 0) { return 1; }
    }
  }
  return 0;
}

void Tiny_Flip(uint8_t render0_picture1, PERSONAGE *Sprite) {
  readButton();
  uint8_t y, x;
  uint8_t Div2x = 0;
  for (y = 0; y < 8; y++) {
    for (x = 0; x < 128; x++) {
      if (render0_picture1 == 0) {
        if (INGAME) {
          uint8_t myByte = PrintLive(x, y) & ((background(x, y) | SpriteWrite(x, y, Sprite) | BlockBomb(x, y, Div2x) | BombBlitz(x, y) | Explose(x, y)));
          for(int i = 0; i < 8; i++){
            if(bitRead(myByte, i)){
              dma_display->drawPixel(x, y*8+i, myColor);
            }else{
              dma_display->drawPixel(x, y*8+i, dma_display->color565(0, 0, 0));
            }
          }
        } else {
        }
      } else if (render0_picture1 == 1) {
        uint8_t myByte = pgm_read_byte(&BackBlitz[x + (y * 128)]);
        for(int i = 0; i < 8; i++){
          if(bitRead(myByte, i)){
            dma_display->drawPixel(x, y*8+i, myColor);
          }else{
            dma_display->drawPixel(x, y*8+i, dma_display->color565(0, 0, 0));
          }
        }
      }
      Div2x = !Div2x;
    }
  }
}

uint8_t PrintLive(uint8_t x, uint8_t y) {
  uint8_t rest = 0xFF;
  if ((x > 7) || (x < 1)) return 0xFF;
  switch (live) {
    case 0:
      if (y == 6) rest = 0x00;
    case 1:
      if (y == 5) rest = 0x00;
    case 2:
      if (y == 4) rest = 0x00;
    case 3:
    default: break;
  }
  return rest;
}

uint8_t Explose(uint8_t x, uint8_t y) {
#define XPOSFIRE (BOMBXY[0] - 8)
#define YPOSFIRE (BOMBXY[1] - 1)
  if (BOMBXY[2] == 1) {
    if ((x > 7) && (y > 0)) {
      if ((x >= XPOSFIRE) && (x < XPOSFIRE + 24) && (y >= YPOSFIRE) && (y <= YPOSFIRE + 2)) {
        return pgm_read_byte(&fire[((x - (XPOSFIRE)) + ((y - (YPOSFIRE)) * 72)) + (BOMBXY[3] * 24)]);
      }
    }
  }
  return 0x00;
}

uint8_t BombBlitz(uint8_t x, uint8_t y) {
  if ((x >= BOMBXY[0]) && (x <= BOMBXY[0] + 7) && (y == BOMBXY[1])) {
    return pgm_read_byte(&bomb[(x - BOMBXY[0]) + (BOMBXY[3] * 8)]);
  } else {
    return 0x00;
  }
}

uint8_t BlockBomb(uint8_t x, uint8_t y, uint8_t Divx) {
  if ((y > 0)) {
    uint8_t BLOCVAL = pgm_read_byte(&BlocDetect[x]);
    if (BLOCVAL == 0xFE) BLOCVAL = 0x01;
    if (BLOCVAL == 0xFF) return 0x00;
    if (BOOLREAD(BLOCVAL + ((y - 1) * 15))) {
      if (Divx == 0) {
        return 0b10101010;
      } else {
        return 0b01010101;
      }
    }
  }
  return 0x00;
}

void BOOLWRITE0(uint8_t BoolNumber) {
  uint8_t REST = BoolNumber;
  uint8_t DOTBOOLPOSITION = 0;
  uint8_t SOUSTRAIRE;
DECREASE:
  if (REST >= 8) {
    REST = REST - 8;
    DOTBOOLPOSITION = DOTBOOLPOSITION + 1;
    goto DECREASE;
  }
  switch (REST) {
    case (0): SOUSTRAIRE = 0b01111111; break;
    case (1): SOUSTRAIRE = 0b10111111; break;
    case (2): SOUSTRAIRE = 0b11011111; break;
    case (3): SOUSTRAIRE = 0b11101111; break;
    case (4): SOUSTRAIRE = 0b11110111; break;
    case (5): SOUSTRAIRE = 0b11111011; break;
    case (6): SOUSTRAIRE = 0b11111101; break;
    case (7): SOUSTRAIRE = 0b11111110; break;
  }
  BlocBombMem[DOTBOOLPOSITION] = BlocBombMem[DOTBOOLPOSITION] & SOUSTRAIRE;
}

uint8_t BOOLREAD(uint8_t Numero) {
  if (Numero > 105) return 0;
  uint8_t BYTECHECK = Numero;
  uint8_t INT8CHOSE = 0;
  while (BYTECHECK > 7) {
    BYTECHECK = BYTECHECK - 8;
    INT8CHOSE++;
  }
  uint8_t Var0 = 0;
  switch (BYTECHECK) {
    case 0: Var0 = 0b10000000; break;
    case 1: Var0 = 0b01000000; break;
    case 2: Var0 = 0b00100000; break;
    case 3: Var0 = 0b00010000; break;
    case 4: Var0 = 0b00001000; break;
    case 5: Var0 = 0b00000100; break;
    case 6: Var0 = 0b00000010; break;
    case 7: Var0 = 0b00000001; break;
    default: Var0 = 0b00000000; break;
  }
  if ((BlocBombMem[INT8CHOSE] & Var0) != 0) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t SplitSpriteDecalageY(uint8_t decalage, uint8_t Input, uint8_t UPorDOWN) {
  if (UPorDOWN) {
    return Input << decalage;
  } else {
    return Input >> (8 - decalage);
  }
}

uint8_t SpriteWrite(uint8_t x, uint8_t y, PERSONAGE *Sprite) {
  uint8_t var1 = 0;
  uint8_t AddBin = 0b00000000;
  while (1) {
    if (Sprite[var1].y == y) {
      if (Sprite[var1].dead != 1) AddBin = AddBin | SplitSpriteDecalageY(Sprite[var1].Decalagey, return_if_sprite_present(x, Sprite, var1), 1);
    } else if (((Sprite[var1].y + 1) == y) && (Sprite[var1].Decalagey != 0)) {
      if (Sprite[var1].dead != 1) AddBin = AddBin | SplitSpriteDecalageY(Sprite[var1].Decalagey, return_if_sprite_present(x, Sprite, var1), 0);
    }
    var1++;
    if (var1 == 5) { break; }
  }
  return AddBin;
}

uint8_t return_if_sprite_present(uint8_t x, PERSONAGE *Sprite, uint8_t SpriteNumber) {
  if ((x >= Sprite[SpriteNumber].x) && (x < (Sprite[SpriteNumber].x + 8))) {
    if ((INGAME == 0) && (SpriteNumber == 0)) { return 0; }
    return pgm_read_byte(&caracters[((x - Sprite[SpriteNumber].x) + (8 * (Sprite[SpriteNumber].type * 12))) + (Sprite[SpriteNumber].anim * 8) + (Sprite[SpriteNumber].DirectionAnim * 8)]);
  }
  return 0;
}

uint8_t background(uint8_t x, uint8_t y) {
  return pgm_read_byte(&BackBlitz[((y)*128) + ((x))]);
}

void Sound(uint8_t freq, uint8_t dur) {
  for (uint8_t t = 0; t < dur; t++) {
    if (freq != 0) {digitalWrite(SOUND_PIN, HIGH);
    }
    for (uint8_t t = 0; t < (255 - freq); t++) {
      delayMicroseconds(1);
    }
    digitalWrite(SOUND_PIN, LOW);
    for (uint8_t t = 0; t < (255 - freq); t++) {
      delayMicroseconds(1);
    }
  }
}
