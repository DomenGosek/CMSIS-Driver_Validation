#ifndef LCDPRINT_H
#define LCDPRINT_H

#include <stdint.h>
// LCDPrint: levels
#define LCDLevelNone        (0U)        ///< \ref LCDPrint \a level parameter: None
#define LCDLevelHeading     (1U)        ///< \ref LCDPrint \a level parameter: Heading
#define LCDLevelMessage     (2U)        ///< \ref LCDPrint \a level parameter: Message
#define LCDLevelError       (3U)        ///< \ref LCDPrint \a level parameter: Error

int32_t LCDPrint (uint32_t level, const char *format, ...);
void    LCDInit (void);

#endif