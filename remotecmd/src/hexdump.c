#include "hexdump.h"
#ifndef _CMOC_VERSION_
#include <stdio.h>
#else
#include <cmoc.h>
#endif

#define COLUMNS 16

void hexdump(void *buffer, int count)
{
  int outer, inner;
  unsigned char c, *bytes = buffer;


  for (outer = 0; outer < count; outer += COLUMNS) {
    for (inner = 0; inner < COLUMNS; inner++) {
      if (inner + outer < count) {
	c = bytes[inner + outer];
	printf("%02x ", c);
      }
      else
	printf("   ");
    }
    printf(" |");
    for (inner = 0; inner < COLUMNS && inner + outer < count; inner++) {
      c = bytes[inner + outer];
      if (c >= ' ' && c <= 0x7f)
	printf("%c", c);
      else
	printf(".");
    }
    printf("|\n");
  }

  return;
}
