// Example sketch which shows how to display a 64x32 animated GIF image stored in FLASH memory
// on a 64x32 LED matrix
//
// Credits: https://github.com/bitbank2/AnimatedGIF/tree/master/examples/ESP32_LEDMatrix_I2S
//

/* INSTRUCTIONS

   1. First Run the 'ESP32 Sketch Data Upload Tool' in Arduino from the 'Tools' Menu.
      - If you don't know what this is or see it as an option, then read this:
        https://github.com/me-no-dev/arduino-esp32fs-plugin
      - This tool will upload the contents of the data/ directory in the sketch folder onto
        the ESP32 itself.

   2. You can drop any animated GIF you want in there, but keep it to the resolution of the
      MATRIX you're displaying to. To resize a gif, use this online website: https://ezgif.com/

   3. Have fun.
*/

// library includes
#define FILESYSTEM SPIFFS
#include <SPIFFS.h>
#include <AnimatedGIF.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

MatrixPanel_I2S_DMA *dma_display = nullptr;
// ----------------------------
#include "SD.h"
#include "SPI.h"

// Micro SD Card Module Pinout
// VCC = 5V
// GND = GND
//#define HSPI_MISO 19
//#define HSPI_MOSI 4
//#define HSPI_SCLK 14
//#define HSPI_CS   5
//
//// Led Panel Pinout
//#define R1_PIN 25
//#define G1_PIN 26
//#define B1_PIN 27
//#define R2_PIN 21
//#define G2_PIN 22
//#define B2_PIN 23
//#define A_PIN 12
//#define B_PIN 16
//#define C_PIN 17
//#define D_PIN 18
//#define E_PIN 13
//#define LAT_PIN 32
//#define OE_PIN 33
//#define CLK_PIN 15


//=======================Configuration for my Testing Board =====================================
//===============Micro SD Card Module Pinout=============
// VCC = 5V
// GND = GND

#define HSPI_MISO 16
#define HSPI_MOSI 32
#define HSPI_SCLK 21
#define HSPI_CS   17

//===================Led Panel Pinout====================
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 17
#define E_PIN -1 //18
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16
//=========================================================================================

// Configure for your panel(s) as appropriate!

#define PANEL_RES_X 64 // Number of pixels width of each individual panel module.
#define PANEL_RES_Y 32 // Number of pixels height of each individual panel module.

#define NUM_ROWS 1 // Number of Vertical rows of chained panels
#define NUM_COLS 1 // Number of Horizontal rows of chained panels

#define PANEL_CHAIN NUM_ROWS*NUM_COLS // Total number of panels chained one to another

// Change this to your needs, for details on VirtualPanel pls see ChainedPanels example
#define SERPENT false
#define TOPDOWN false

SPIClass *spi = NULL;
AnimatedGIF gif;
File f;
int x_offset, y_offset;

// Draw a line of image directly on the LED Matrix
void GIFDraw(GIFDRAW *pDraw) {
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > MATRIX_WIDTH)
    iWidth = MATRIX_WIDTH;

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) {// restore to background color
    for (x = 0; x < iWidth; x++) {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) { // if transparency used
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + pDraw->iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < pDraw->iWidth) {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent) {// done, stop
          s--; // back up to treat it like transparent
        }
        else {// opaque
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels
      if (iCount) {// any opaque pixels?
        for (int xOffset = 0; xOffset < iCount; xOffset++ ) {
          dma_display->drawPixel(x + xOffset, y, usTemp[xOffset]); // 565 Color Format
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount) {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  }
  else {// does not have transparency
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++) {
      dma_display->drawPixel(x, y, usPalette[*s++]); // color 565
    }
  }
} /* GIFDraw() */

void * GIFOpenFile(const char *fname, int32_t *pSize) {
  f = FILESYSTEM.open(fname);
  if (f) {
    *pSize = f.size();
    return (void *)&f;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle) {
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
    f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
  if (iBytesRead <= 0)
    return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
  //  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

unsigned long start_tick = 0;

void ShowGIF(char *name) {
  start_tick = millis();

  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
    x_offset = (MATRIX_WIDTH - gif.getCanvasWidth()) / 2;
    if (x_offset < 0) x_offset = 0;
    y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight()) / 2;
    if (y_offset < 0) y_offset = 0;
    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    Serial.flush();
    while (gif.playFrame(true, NULL)) {
      if ( (millis() - start_tick) > 8000) { // we'll get bored after about 8 seconds of the same looping gif
        break;
      }
    }
    gif.close();
  }
} /* ShowGIF() */

/************************* Arduino Sketch Setup and Loop() *******************************/
void setup() {
  Serial.begin(115200);
  delay(180);

  //Serial.println("Starting AnimatedGIFs Sketch");

  // ==================Start filesystem=======================
  //  Serial.println(" * Loading SPIFFS");
  //  if(!SPIFFS.begin()){
  //        Serial.println("SPIFFS Mount Failed");
  //  }

  //====================== Setting SD CARD ===========================
  //  Serial.println("Micro SD Card Mounting...");
  //
  //  spi = new SPIClass(HSPI);
  //  spi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
  //
  //  SD.begin(HSPI_CS, *spi);
  //  delay(50);
  //
  //  if (!SD.begin(HSPI_CS, *spi)) {
  //    Serial.println("Card Mount Failed");
  //    return;
  //  }
  //
  //  uint8_t cardType = SD.cardType();
  //
  //  if (cardType == CARD_NONE) {
  //    Serial.println("No SD card attached");
  //    return;
  //  }
  //
  //  Serial.print("SD Card Type: ");
  //
  //  if (cardType == CARD_MMC) {
  //    Serial.println("MMC");
  //  } else if (cardType == CARD_SD) {
  //    Serial.println("SDSC");
  //  } else if (cardType == CARD_SDHC) {
  //    Serial.println("SDHC");
  //  } else {
  //    Serial.println("UNKNOWN");
  //  }
  //
  //  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  //  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  //  Serial.println();

  // Manually configured GPIO pins assigned to: _pins
  HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

  // (width, height, chain length, manual pin mapping, driver chip)
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,
    PANEL_RES_Y,
    PANEL_CHAIN,
    _pins
    //HUB75_I2S_CFG::FM6126A
  );

  // FM6126A panels could be clocked at 10MHz or 20MHz with no visual artefacts
  // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_20M;

  // Create our matrix object
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  //  // Allocate memory and start DMA display
  //  if( not dma_display->begin(); )
  //      Serial.println("****** !KABOOM! I2S memory allocation failed ***********");
  //      Serial.println();

  dma_display->begin();

  //  /* all other pixel drawing functions can only be called after .begin() */
  //  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  //  gif.begin(LITTLE_ENDIAN_PIXELS);

  // Fill the screen with 'black'
  //  dma_display->fillScreen(dma_display->color444(0, 0, 0));

  dma_display->fillScreen(dma_display->color444(180, 0, 0));
  delay(1000);

  dma_display->fillScreen(dma_display->color444(0, 180, 0));
  delay(1000);

  dma_display->fillScreen(dma_display->color444(0, 0, 180));
  delay(1000);

  dma_display->fillScreen(dma_display->color444(80, 180, 180));
  delay(1000);

  dma_display->fillScreen(dma_display->color444(0, 0, 0));
  delay(1000);

  dma_display->setFont(&FreeSansBold12pt7b);
  //dma_display->setFont(&FreeSans9pt7b);

  dma_display->setTextColor(dma_display->color565(0, 100, 100));
  dma_display->setTextSize(1); //3
  //dma_display->setCursor(10, dma_display->height()-20);
  //dma_display->setCursor(43, dma_display->height() - 26);
  dma_display->setCursor(5, dma_display->height() - 10);

  // Red text inside red rect
  dma_display->print("1234");
  dma_display->drawRect(1, 1, dma_display->width() - 2, dma_display->height() - 2, dma_display->color565(255, 0, 0));

  // White line from top left to bottom right
  dma_display->drawLine(0, 0, dma_display->width() - 1, dma_display->height() - 1, dma_display->color565(255, 255, 255));
  //   dma_display->drawLine(0,0, dma_display->width()-0, dma_display->height()-0, dma_display->color565(255,255,255));

  delay(5000);

  dma_display->fillScreen(dma_display->color444(0, 0, 0));

  Serial.println("Starting AnimatedGIFs Sketch");

  gif.begin(LITTLE_ENDIAN_PIXELS);
  delay(50);
}

void loop() {
  char *szDir = "/gifs"; // play all GIFs in this directory on the SD card
  char fname[256];
  File root, temp;

  while (1) {// run forever
    //root = FILESYSTEM.open(szDir); // File System
    root = SD.open(szDir);  // For SD card
    if (root) {
      temp = root.openNextFile();
      while (temp) {
        if (!temp.isDirectory()) {// play it
          strcpy(fname, temp.name());

          Serial.printf("Playing %s\n", temp.name());
          Serial.flush();
          ShowGIF((char *)temp.name());
        }
        temp.close();
        temp = root.openNextFile();
      }
      root.close();
    } // root
    delay(100); // pause before restarting
  } // while
}
