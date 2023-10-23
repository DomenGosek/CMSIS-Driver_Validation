#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "LCDprint.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_sdram.h"
#include "cmsis_os2.h"
#include "cmsis_compiler.h"

#define LCD_PRINT_MAX_SIZE      64U     // maximum size of print memory
#define LCD_PRINT_MEM_NUM        4U     // number of print memories

__USED char          LCDPrintMem[LCD_PRINT_MEM_NUM][LCD_PRINT_MAX_SIZE];

static osMutexId_t mid_mutLCD;

typedef struct displayArea {
  uint16_t   xOrigin;          // x Origin
  uint16_t   xWidth;           // x width
  uint16_t   yOrigin;          // y Origin
  uint16_t   yHeight;          // y height
  uint16_t   fontWidth;        // font width
  uint16_t   fontHeight;       // font height
} displayArea_t;

static displayArea_t display[4];

/**
  write a string to the selected display

  \param[in]   idx   Display index.
  \param[in]   str   String
*/
static void displayString (uint32_t idx, char *str) {
  char ch;
  uint8_t i = 0;
  uint16_t cursor_x, cursor_y;
  
  cursor_x = display[idx].xOrigin;
  cursor_y = display[idx].yOrigin;

  while (str[i] != '\0') {
    ch = str[i];                                            /* Get character and increase index */
    i++;

    switch (ch) {
      case 0x0A:                                            // Line Feed
        cursor_y += display[idx].fontHeight;                /* Move cursor one row down */
        if (cursor_y >= display[idx].yHeight) {             /* If bottom of display was overstepped */
          cursor_y = display[idx].yOrigin;                  /* Stay in last row */
        }
        break;
      case 0x0D:                                            /* Carriage Return */
        cursor_x = display[idx].xOrigin;                    /* Move cursor to first column */
        break;
      default:
        // Display character at current cursor position
        BSP_LCD_DisplayChar(cursor_x, cursor_y, ch);
        cursor_x += display[idx].fontWidth;                 /* Move cursor one column to right */
        if (cursor_x >= display[idx].xWidth) {              /* If last column was overstepped */
          cursor_x = display[idx].xOrigin;                  /* First column */
          cursor_y += display[idx].fontHeight;              /* Move cursor one row down */
          if (cursor_y >= display[idx].yHeight) {           /* If bottom of display was overstepped */
            cursor_y = display[idx].yOrigin;                /* Rollover to vertical origin */
          }
        }
        break;
    }
  }
}

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
  
  osMutexAcquire(mid_mutLCD, 0xFF);
  switch (level) {
    case LCDLevelNone:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      displayString(level, (char *)LCDPrintMem[level]);
      break;
    case LCDLevelHeading:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
      displayString(level, (char *)LCDPrintMem[level]);
      break;
    case LCDLevelMessage:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      displayString(level, (char *)LCDPrintMem[level]);
      break;
    case LCDLevelError:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_RED);
      displayString(level, (char *)LCDPrintMem[level]);
      break;
  }

  osMutexRelease(mid_mutLCD);

  return (ret);
}

void LCDInit (void) {
  // Create LCD mutex
  mid_mutLCD = osMutexNew(NULL);
  if (mid_mutLCD == NULL) { /* add error handling */ }

  // Initialize the LCD
  BSP_LCD_Init();

  // Initialize the LCD Layers
  BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);

  // Set LCD Foreground Layer
  BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);

  BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

  // Clear the LCD
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_Clear(LCD_COLOR_BLUE);

  // Initialize display areas
  display[LCDLevelHeading].fontWidth  =  11;
  display[LCDLevelHeading].fontHeight =  16;
  display[LCDLevelHeading].xOrigin    =   3;
  display[LCDLevelHeading].xWidth     = BSP_LCD_GetXSize() - 4;
  display[LCDLevelHeading].yOrigin    =   4;
  display[LCDLevelHeading].yHeight    =  2 * display[LCDLevelHeading].fontHeight + display[LCDLevelHeading].yOrigin;

  display[LCDLevelNone].fontWidth     =  11;
  display[LCDLevelNone].fontHeight    =  16;
  display[LCDLevelNone].xOrigin       =   3;
  display[LCDLevelNone].xWidth        = BSP_LCD_GetXSize() - 4;
  display[LCDLevelNone].yOrigin       =  40;
  display[LCDLevelNone].yHeight       =  2 * display[LCDLevelNone].fontHeight + display[LCDLevelNone].yOrigin;

  display[LCDLevelError].fontWidth    =  11;
  display[LCDLevelError].fontHeight   =  16;
  display[LCDLevelError].xOrigin      =   3;
  display[LCDLevelError].xWidth       = BSP_LCD_GetXSize() - 4;
  display[LCDLevelError].yOrigin      =  68;
  display[LCDLevelError].yHeight      =  2 * display[LCDLevelError].fontHeight + display[LCDLevelError].yOrigin;

  display[LCDLevelMessage].fontWidth  =  11;
  display[LCDLevelMessage].fontHeight =  16;
  display[LCDLevelMessage].xOrigin    =   3;
  display[LCDLevelMessage].xWidth     = BSP_LCD_GetXSize() - 4;
  display[LCDLevelMessage].yOrigin    = 120;
  display[LCDLevelMessage].yHeight    =  2 * display[LCDLevelMessage].fontHeight + display[LCDLevelMessage].yOrigin;

}