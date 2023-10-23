#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "LCDprint.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"
#include "cmsis_os2.h"
#include "cmsis_compiler.h"

#define LCD_PRINT_MAX_SIZE      64U     // maximum size of print memory
#define LCD_PRINT_MEM_NUM        4U     // number of print memories

__USED char          LCDPrintMem[LCD_PRINT_MEM_NUM][LCD_PRINT_MAX_SIZE];

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

static uint32_t       cursor_x;         // GLCD cursor x (horizontal) position (in pixels)
static uint32_t       cursor_y;         // GLCD cursor y (vertical)   position (in pixels)
static osMutexId_t    mid_GLCD;         // Mutex ID of GLCD mutex

// Print formated string to test terminal
int32_t LCDPrint (uint32_t level, const char *format, ...) {
  va_list args;
  int32_t ret;

  uint16_t cursor_x, cursor_y;          // GLCD cursor position (in pixels)
  uint8_t  font_w, font_h;
  uint8_t  i;
  char     ch;

  if (level > LCDLevelError) {
    return (-1);
  }

  if (level > LCD_PRINT_MEM_NUM) {
    return (-1);
  }

  va_start(args, format);

  ret = vsnprintf((char *)LCDPrintMem[level], sizeof(LCDPrintMem[level]), format, args);

  va_end(args);
  
  if (mid_GLCD != NULL) {
    osMutexAcquire(mid_GLCD, osWaitForever);
    switch (level) {
      case LCDLevelNone:                // Normal text
        font_w   = GLCD_Font_6x8.width;
        font_h   = GLCD_Font_6x8.height;
        cursor_x = 0U;                  // Normal text starting position
        cursor_y = 0U;                  // 1st text row
        (void)GLCD_SetFont            (&GLCD_Font_6x8);
        (void)GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
        break;
      case LCDLevelHeading:             // Heading text
        font_w   = GLCD_Font_16x24.width;
        font_h   = GLCD_Font_16x24.height;
        cursor_x = 0U;                  // Heading text starting position
        cursor_y = font_h;              // 2nd text row
        (void)GLCD_SetFont            (&GLCD_Font_16x24);
        (void)GLCD_SetForegroundColor (GLCD_COLOR_GREEN);
        break;
      case LCDLevelMessage:             // Message text
        font_w   = GLCD_Font_16x24.width;
        font_h   = GLCD_Font_16x24.height;
        cursor_x = 0U;                  // Message text starting position
        cursor_y = font_h * 5U;         // 6th text row
        (void)GLCD_SetFont            (&GLCD_Font_16x24);
        (void)GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
        break;
      case LCDLevelError:               // Error text
        font_w   = GLCD_Font_16x24.width;
        font_h   = GLCD_Font_16x24.height;
        cursor_x = 0U;                  // Error text starting position
        cursor_y = font_h * 9U;         // 10th text row
        (void)GLCD_SetFont            (&GLCD_Font_16x24);
        (void)GLCD_SetForegroundColor (GLCD_COLOR_RED);
        break;
    }

    i = 0U;
    while (LCDPrintMem[level][i] != 0) {
      ch = LCDPrintMem[level][i];
      i++;

      switch (ch) {
        case 0x0A:                      // Line Feed ('\n')
          cursor_y += font_h;
          if (cursor_y >= GLCD_HEIGHT) {
            // If cursor is out of screen vertically then rollover to vertical 0 
            cursor_y = 0U;
          }
          break;
        case 0x0D:                      // Carriage Return ('\r')
          // Move the cursor to horizontal 0
          cursor_x = 0U;
          break;
        default:                        // Any other character
          // Display current character at the cursor position
          (void)GLCD_DrawChar(cursor_x, cursor_y, ch);
          // Move the cursor to the next character on the right
          cursor_x += font_w;
          if (cursor_x >= GLCD_WIDTH) {
            // If cursor is out of screen horizontally then rollover to horizontal 0
            // and into new line
            cursor_x  = 0U;
            cursor_y += font_h;
            if (cursor_y >= GLCD_HEIGHT) {
              // If cursor is out of screen vertically then rollover to vertical 0
              cursor_y = 0U;
            }
          }
          break;
      }
    }

    osMutexRelease(mid_GLCD);
  }

  return (ret);
}

void  LCDInit (void) {
  (void)LED_Initialize();

  mid_GLCD = osMutexNew(NULL);          // Create GLCD mutex
  if (mid_GLCD != NULL) {
    (void)GLCD_Initialize();
    (void)GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);
    (void)GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    (void)GLCD_ClearScreen();
  }
}