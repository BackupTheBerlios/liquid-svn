#ifndef __KWIN_LIQUID_BITMAPS_H
#define __KWIN_LIQUID_BITMAPS_H

static unsigned char max_bits[] = {
   0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x01, 0x08, 0x01, 0x08, 0x01, 0x08,
   0x01, 0x08, 0x01, 0x08, 0x01, 0x08, 0x01, 0x08, 0x01, 0x08, 0xff, 0x0f };

static unsigned char min_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xff, 0x0f, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00 };
/*static unsigned char min_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f };
*/
static unsigned char minmax_bits[] = {
   0xf0, 0x0f, 0xf0, 0x0f, 0x10, 0x08, 0x10, 0x08, 0xff, 0x08, 0xff, 0x08,
   0x81, 0x08, 0x81, 0x0f, 0x81, 0x00, 0x81, 0x00, 0x81, 0x00, 0xff, 0x00 };

static unsigned char sticky_bits[] = {
   0x00, 0x01, 0x80, 0x03, 0xc0, 0x06, 0x60, 0x0c, 0x3e, 0x06, 0x3c, 0x03,
   0xc8, 0x01, 0xd8, 0x00, 0xfc, 0x00, 0xce, 0x00, 0x87, 0x00, 0x03, 0x00 };

static unsigned char stickydown_bits[] = {
   0x00, 0x00, 0xf0, 0x00, 0xfc, 0x03, 0x0c, 0x03, 0x66, 0x06, 0xf6, 0x06,
   0xf6, 0x06, 0x66, 0x06, 0x0c, 0x03, 0xfc, 0x03, 0xf0, 0x00, 0x00, 0x00 };
static unsigned char close_bits[] = {
   0x00, 0x00, 0x04, 0x02, 0x0e, 0x07, 0x9c, 0x03, 0xf8, 0x01, 0xf0, 0x00,
   0xf0, 0x00, 0xf8, 0x01, 0x9c, 0x03, 0x0e, 0x07, 0x04, 0x02, 0x00, 0x00 };
//taken from plastik -- i suck at graphiks  
static unsigned char help_bits[] = {
   0x50, 0x00, 0xdc, 0x01, 0x88, 0x01, 0x80, 0x03, 0x80, 0x01, 0xc0, 0x01,
   0x60, 0x00, 0x20, 0x00, 0x00, 0x00, 0x30, 0x00, 0x70, 0x00, 0x00, 0x00 };

#endif // __KWIN_LIQUID_BITMAPS_H
