/******************************************************************************
    -----------
    Steps to use
    -----------

    1) In the sketch (i.e. this example):

    - Set values for NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN.
      There are comments beside them explaining what they are in more detail.
    - Other than where the matrix is defined and matrix.begin in the setup, you
      should now be using the virtual display for everything (drawing pixels, writing text etc).
       You can do a find and replace of all calls if it's an existing sketch
      (just make sure you don't replace the definition and the matrix.begin)
    - If the sketch makes use of MATRIX_HEIGHT or MATRIX_WIDTH, these will need to be
      replaced with the width and height of your virtual screen.
      Either make new defines and use that, or you can use virtualDisp.width() or .height()

    Thanks to:

      Brian Lough for the original example as raised in this issue:
      https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/26

      YouTube: https://www.youtube.com/brianlough
      Tindie: https://www.tindie.com/stores/brianlough/
      Twitter: https://twitter.com/witnessmenow

      Galaxy-Man for the kind donation of panels make/test that this is possible:
      https://github.com/Galaxy-Man

*****************************************************************************/


/******************************************************************************
   VIRTUAL DISPLAY / MATRIX PANEL CHAINING CONFIGURATION

   Note 1: If chaining from the top right to the left, and then S curving down
           then serpentine_chain = true and top_down_chain = true
           (these being the last two parameters of the virtualDisp(...) constructor.

   Note 2: If chaining starts from the bottom up, then top_down_chain = false.

   Note 3: By default, this library has serpentine_chain = true, that is, every
           second row has the panels 'upside down' (rotated 180), so the output
           pin of the row above is right above the input connector of the next
           row.

  Example 1 panel chaining:
  +-----------------+-----------------+-------------------+
  | 64x32px PANEL 3 | 64x32px PANEL 2 | 64x32px PANEL 1   |
  |        ------------   <--------   | ------------xx    |
  | [OUT]  |   [IN] | [OUT]      [IN] | [OUT]    [ESP IN] |
  +--------|--------+-----------------+-------------------+
  | 64x32px|PANEL 4 | 64x32px PANEL 5 | 64x32px PANEL 6   |
  |       \|/   ---------->           |  ----->           |
  | [IN]      [OUT] | [IN]      [OUT] | [IN]      [OUT]   |
  +-----------------+-----------------+-------------------+

  Example 1 configuration:

   #define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
   #define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

   #define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
   #define NUM_COLS 3 // Number of INDIVIDUAL PANELS per ROW

   virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true, true);

   = 192x64 px virtual display, with the top left of panel 3 being pixel co-ord (0,0)

  ==========================================================

  Example 2 panel chaining:

  +-------------------+
  | 64x32px PANEL 1   |
  | ----------------- |
  | [OUT]    [ESP IN] |
  +-------------------+
  | 64x32px PANEL 2   |
  |                   |
  | [IN]      [OUT]   |
  +-------------------+
  | 64x32px PANEL 3   |
  |                   |
  | [OUT]      [IN]   |
  +-------------------+
  | 64x32px PANEL 4   |
  |                   |
  | [IN]      [OUT]   |
  +-------------------+

  Example 2 configuration:

   #define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
   #define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

   #define NUM_ROWS 4 // Number of rows of chained INDIVIDUAL PANELS
   #define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW

   virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true, true);

   virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true, true);

   = 128x64 px virtual display, with the top left of panel 1 being pixel co-ord (0,0)

  ==========================================================

  Example 3 panel chaining (bottom up):

  +-----------------+-----------------+
  | 64x32px PANEL 4 | 64x32px PANEL 3 |
  |             <----------           |
  | [OUT]      [IN] | [OUT]      [in] |
  +-----------------+-----------------+
  | 64x32px PANEL 1 | 64x32px PANEL 2 |
  |             ---------->           |
  | [ESP IN]  [OUT] | [IN]      [OUT] |
  +-----------------+-----------------+

  Example 1 configuration:

   #define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
   #define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

   #define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
   #define NUM_COLS 2 // Number of INDIVIDUAL PANELS per ROW

   virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true, false);

   = 128x64 px virtual display, with the top left of panel 4 being pixel co-ord (0,0)

*/


//===============Pins Configuration=====================
//======================================================

// Micro SD Card Module Pinout
// VCC = 5V
// GND = GND

#define HSPI_MISO 19
#define HSPI_MOSI 4
#define HSPI_SCLK 14
#define HSPI_CS   5

// Led Panel Pinout
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 21
#define G2_PIN 22
#define B2_PIN 23
#define A_PIN 12
#define B_PIN 16
#define C_PIN 17
#define D_PIN 18
#define E_PIN 13
#define LAT_PIN 32
#define OE_PIN 33
#define CLK_PIN 15


//=======================Configuration for my Testing Board =====================================
//===============Micro SD Card Module Pinout=============
// VCC = 5V
// GND = GND

//#define HSPI_MISO 16
//#define HSPI_MOSI 32
//#define HSPI_SCLK 21
//#define HSPI_CS   17
//
////===================Led Panel Pinout====================
//#define R1_PIN 25
//#define G1_PIN 26
//#define B1_PIN 27
//#define R2_PIN 14
//#define G2_PIN 12
//#define B2_PIN 13
//#define A_PIN 23
//#define B_PIN 19
//#define C_PIN 5
//#define D_PIN 17
//#define E_PIN -1 //18
//#define LAT_PIN 4
//#define OE_PIN 15
//#define CLK_PIN 16
//=========================================================================================

// Matrix Configuration
#define PANEL_RES_X 64 // Number of pixels width of each individual panel module.
#define PANEL_RES_Y 32 // Number of pixels height of each individual panel module.

#define NUM_ROWS 3 // Number of Vertical rows of chained panels
#define NUM_COLS 4 // Number of Horizontal rows of chained panels
#define PANEL_CHAIN NUM_ROWS*NUM_COLS // Total number of panels chained one to another

// Change this to your needs, for details on VirtualPanel pls see ChainedPanels example
#define SERPENT false
#define TOPDOWN false

// library includes
#include <AnimatedGIF.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include "SD.h"
#include "SPI.h"

// Placeholder for the matrix object
MatrixPanel_I2S_DMA *dma_display = nullptr;

// Placeholder for the virtual display object
VirtualMatrixPanel *virtualDisp = nullptr;

SPIClass *spi = NULL;

AnimatedGIF gif;
File f;
int x_offset, y_offset;

// ======== Draw a line of image directly on the LED Matrix ==========
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
  if (pDraw->ucDisposalMethod == 2) { // restore to background color

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
        if (c == ucTransparent) {   // done, stop
          s--; // back up to treat it like transparent
        }
        else {// opaque
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels

      if (iCount) { // any opaque pixels?
        for (int xOffset = 0; xOffset < iCount; xOffset++ ) {
          virtualDisp->drawPixel(x + xOffset, y, usTemp[xOffset]);
        }
        x += iCount;
        iCount = 0;
      }

      // No, look for a run of transparent pixels
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
  else { // does not have transparency
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++) {
      virtualDisp->drawPixel(x, y, usPalette[*s++]);
    }
  }
}

// ===================== Open Gif File ====================

void * GIFOpenFile(const char *fname, int32_t *pSize) {
  f = SD.open(fname);           // For SD Card
  if (f) {
    *pSize = f.size();
    return (void *)&f;
  }
  return NULL;
}

// ====================== Close Gif File ======================
void GIFCloseFile(void *pHandle) {
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
    f->close();
}

// ===================== Read Gif File =======================
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
}

// ================= Seek Gif File ====================
int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
  //  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
}

// ================= Show Gif ====================
unsigned long start_tick = 0;
void ShowGIF(char *name) {
  start_tick = millis();
  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
    x_offset = (MATRIX_WIDTH - gif.getCanvasWidth()) / 2;
    if (x_offset < 0) x_offset = 0;
    y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight()) / 2;

    if (y_offset < 0) y_offset = 0;
    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    Serial.println();
    //    Serial.println(gif.getCanvasWidth());
    //    Serial.println(gif.getCanvasHeight());
    Serial.flush();
    while (gif.playFrame(true, NULL)) {
      if ( (millis() - start_tick) > 8000) { // we'll get bored after about 8 seconds of the same looping gif
        break;
      }
    }
    gif.close();
  }
}

// ======================== Setup ============================
void setup() {
  Serial.begin(115200);
  delay(200);

  //Serial.println("Starting AnimatedGIFs Sketch");

  // ==================Start filesystem=======================
  //  Serial.println(" * Loading SPIFFS");
  //  if(!SPIFFS.begin()){
  //        Serial.println("SPIFFS Mount Failed");
  //  }

  // ==================Start SD Card==========================
      Serial.println("Micro SD Card Mounting...");
  
      spi = new SPIClass(HSPI);
      spi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
  
      SD.begin(HSPI_CS, *spi);
      delay(50);
  
      if (!SD.begin(HSPI_CS, *spi)) {
        Serial.println("Card Mount Failed");
        return;
      }
  
      uint8_t cardType = SD.cardType();
  
      if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
      }
  
      Serial.print("SD Card Type: ");
  
      if (cardType == CARD_MMC) {
        Serial.println("MMC");
      } else if(cardType == CARD_SD){
        Serial.println("SDSC");
      } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
      } else {
        Serial.println("UNKNOWN");
      }
  
      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.printf("SD Card Size: %lluMB\n", cardSize);
      Serial.println();


  // Manually configured GPIO pins assigned to: _pins
  HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

  // (width, height, chain length, manual pin mapping, driver chip)
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,
    PANEL_RES_Y,
    PANEL_CHAIN,
    _pins//,
    //HUB75_I2S_CFG::FM6126A
  );

  //====FM6126A panels could be clocked at 10MHz or 20MHz with no visual artefacts
  //mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_20M;

  // Create our matrix object
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  // Range is 0-255. 0=0%, 50=20%, 100=40%, 150=60%, 200=80%, 255 = 100%
  dma_display->setBrightness8(180);

  // Create VirtualDisplay object based on our newly created dma_display object
  virtualDisp = new VirtualMatrixPanel((*dma_display), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true);

  // Allocate memory and start DMA display
  if ( not dma_display->begin() )
    Serial.println("****** !KABOOM! I2S memory allocation failed ***********");
  Serial.println();

  // Fill the screen with 'black'
  //  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));

  virtualDisp->fillScreen(virtualDisp->color444(180, 0, 0));
  delay(1000);

  virtualDisp->fillScreen(virtualDisp->color444(0, 180, 0));
  delay(1000);

  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 180));
  delay(1000);

  virtualDisp->fillScreen(virtualDisp->color444(80, 180, 180));
  delay(1000);

  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));
  delay(1000);

  virtualDisp->setFont(&FreeSansBold12pt7b);
  virtualDisp->setTextColor(virtualDisp->color565(0, 100, 100));
  virtualDisp->setTextSize(1);//3
  //virtualDisp->setCursor(10, virtualDisp->height()-20);
  //virtualDisp->setCursor(43, virtualDisp->height() - 26);
  virtualDisp->setCursor(5, virtualDisp->height() - 10);

  // Red text inside red rect
  virtualDisp->print("1234");
  virtualDisp->drawRect(1, 1, virtualDisp->width() - 2, virtualDisp->height() - 2, virtualDisp->color565(255, 0, 0));

  // White line from top left to bottom right
  virtualDisp->drawLine(0, 0, virtualDisp->width() - 1, virtualDisp->height() - 1, virtualDisp->color565(255, 255, 255));
  //   virtualDisp->drawLine(0,0, virtualDisp->width()-0, virtualDisp->height()-0, virtualDisp->color565(255,255,255));

  delay(5000);

  //virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));  //Black

  Serial.println("Starting AnimatedGIFs Sketch");

  gif.begin(LITTLE_ENDIAN_PIXELS);
  delay(50);
}

// ======================== Loop =========================
void loop() {
  char *szDir = "/gifs"; // Play all GIFs in this directory on the SD card
  char fname[256];
  File root, temp;

  while (1) { // Run forever
    //root = FILESYSTEM.open(szDir); //For File System
    root = SD.open(szDir);  // For SD card
    if (root) {
      temp = root.openNextFile();
      while (temp) {
        if (!temp.isDirectory()) {    // Play it
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
    virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));
  } // while
}
