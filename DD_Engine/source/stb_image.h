/*
stb_image - v2.28 - public domain image loader
https://github.com/nothings/stb
*/

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STBI_VERSION 1

enum
{
   STBI_default = 0,
   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4
};

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STBIDEF
#define STBIDEF extern
#endif

STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF void     stbi_image_free(void *retval_from_stbi_load);
STBIDEF const char *stbi_failure_reason(void);

#ifdef __cplusplus
}
#endif

#endif // STBI_INCLUDE_STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

#ifdef _MSC_VER
#pragma warning(disable: 4996)  // disable fopen warning
#endif

#ifndef STBI_ASSERT
#include <assert.h>
#define STBI_ASSERT(x) assert(x)
#endif

#ifndef STBI_MALLOC
#define STBI_MALLOC(sz)           malloc(sz)
#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
#define STBI_FREE(p)              free(p)
#endif

typedef unsigned char stbi__uint8;
typedef unsigned short stbi__uint16;
typedef signed short stbi__int16;
typedef unsigned int stbi__uint32;
typedef signed int stbi__int32;

static const char *stbi__g_failure_reason;

STBIDEF const char *stbi_failure_reason(void)
{
   return stbi__g_failure_reason;
}

static int stbi__err(const char *str)
{
   stbi__g_failure_reason = str;
   return 0;
}

typedef struct
{
   stbi__uint32 img_x, img_y;
   int img_n, img_out_n;
   void *io_user_data;
   int read_from_callbacks;
   int buflen;
   stbi__uint8 buffer_start[128];
   int callback_already_read;
   stbi__uint8 *img_buffer, *img_buffer_end;
   stbi__uint8 *img_buffer_original, *img_buffer_original_end;
} stbi__context;

static void stbi__refill_buffer(stbi__context *s) { }

static stbi__uint8 stbi__get8(stbi__context *s)
{
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;
   return 0;
}

static int stbi__get16be(stbi__context *s)
{
   int z = stbi__get8(s);
   return (z << 8) + stbi__get8(s);
}

static stbi__uint32 stbi__get32be(stbi__context *s)
{
   stbi__uint32 z = stbi__get16be(s);
   return (z << 16) + stbi__get16be(s);
}

static int stbi__get16le(stbi__context *s)
{
   int z = stbi__get8(s);
   return z + (stbi__get8(s) << 8);
}

static stbi__uint32 stbi__get32le(stbi__context *s)
{
   stbi__uint32 z = stbi__get16le(s);
   return z + (stbi__get16le(s) << 16);
}

static void stbi__skip(stbi__context *s, int n)
{
   if (n < 0) {
      s->img_buffer = s->img_buffer_end;
      return;
   }
   s->img_buffer += n;
}

static int stbi__getn(stbi__context *s, stbi__uint8 *buffer, int n)
{
   if (s->img_buffer+n <= s->img_buffer_end) {
      memcpy(buffer, s->img_buffer, n);
      s->img_buffer += n;
      return 1;
   }
   return 0;
}

static int stbi__at_eof(stbi__context *s)
{
   return s->img_buffer >= s->img_buffer_end;
}

static stbi__uint8 *stbi__load_and_postprocess_8bit(stbi__context *s, int *x, int *y, int *comp, int req_comp);

static void stbi__start_mem(stbi__context *s, stbi__uint8 const *buffer, int len)
{
   s->io_user_data = NULL;
   s->read_from_callbacks = 0;
   s->callback_already_read = 0;
   s->img_buffer = s->img_buffer_original = (stbi__uint8 *) buffer;
   s->img_buffer_end = s->img_buffer_original_end = (stbi__uint8 *) buffer+len;
}

#ifndef STBI_NO_STDIO
static void stbi__start_file(stbi__context *s, FILE *f)
{
   int n = (int)fread(s->buffer_start, 1, sizeof(s->buffer_start), f);
   s->io_user_data = (void*)f;
   s->read_from_callbacks = 0;
   s->img_buffer = s->img_buffer_original = s->buffer_start;
   s->img_buffer_end = s->img_buffer_original_end = s->buffer_start + n;
   s->buflen = sizeof(s->buffer_start);
}

static stbi__uint8 *stbi__load_main(stbi__context *s, int *x, int *y, int *comp, int req_comp);

STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   FILE *f = fopen(filename, "rb");
   stbi__uint8 *result;
   if (!f) return (stbi_uc*)(size_t)(stbi__err("can't fopen"));
   
   fseek(f, 0, SEEK_END);
   long len = ftell(f);
   fseek(f, 0, SEEK_SET);
   
   stbi__uint8 *buffer = (stbi__uint8*)STBI_MALLOC(len);
   if (!buffer) { fclose(f); return (stbi_uc*)(size_t)(stbi__err("outofmem")); }
   
   fread(buffer, 1, len, f);
   fclose(f);
   
   stbi__context s;
   stbi__start_mem(&s, buffer, (int)len);
   result = stbi__load_and_postprocess_8bit(&s, x, y, comp, req_comp);
   
   STBI_FREE(buffer);
   return result;
}
#endif

STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_mem(&s, buffer, len);
   return stbi__load_and_postprocess_8bit(&s, x, y, comp, req_comp);
}

STBIDEF void stbi_image_free(void *retval_from_stbi_load)
{
   STBI_FREE(retval_from_stbi_load);
}

// BMP loader
static int stbi__bmp_test(stbi__context *s)
{
   int r = stbi__get8(s) == 'B' && stbi__get8(s) == 'M';
   return r;
}

static void *stbi__bmp_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi__uint8 *out;
   unsigned int mr=0, mg=0, mb=0, ma=0;
   int psize=0, i, j, width;
   int flip_vertically, pad, target;
   
   if (stbi__get8(s) != 'B' || stbi__get8(s) != 'M') return (stbi__uint8*)(size_t)stbi__err("not BMP");
   
   stbi__get32le(s); // file size
   stbi__get16le(s); // reserved
   stbi__get16le(s); // reserved
   int offset = stbi__get32le(s);
   int hsz = stbi__get32le(s);
   
   if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124)
      return (stbi__uint8*)(size_t)stbi__err("unknown BMP");
   
   if (hsz == 12) {
      s->img_x = stbi__get16le(s);
      s->img_y = stbi__get16le(s);
   } else {
      s->img_x = stbi__get32le(s);
      s->img_y = stbi__get32le(s);
   }
   
   if (stbi__get16le(s) != 1) return (stbi__uint8*)(size_t)stbi__err("bad BMP");
   
   int bpp = stbi__get16le(s);
   if (hsz != 12) {
      int compress = stbi__get32le(s);
      if (compress == 1 || compress == 2) return (stbi__uint8*)(size_t)stbi__err("BMP RLE");
      stbi__get32le(s); // image size
      stbi__get32le(s); // x pixels per meter
      stbi__get32le(s); // y pixels per meter
      stbi__get32le(s); // colors used
      stbi__get32le(s); // important colors
      
      if (hsz == 40 || hsz == 56) {
         if (hsz == 56) {
            stbi__get32le(s);
            stbi__get32le(s);
            stbi__get32le(s);
            stbi__get32le(s);
         }
         if (bpp == 16 || bpp == 32) {
            if (compress == 0) {
               if (bpp == 32) { mr = 0xffu << 16; mg = 0xffu << 8; mb = 0xffu; ma = 0xffu << 24; }
               else { mr = 31u << 10; mg = 31u << 5; mb = 31u; }
            } else if (compress == 3) {
               mr = stbi__get32le(s);
               mg = stbi__get32le(s);
               mb = stbi__get32le(s);
            }
         }
      } else {
         for (i=0; i < (int)hsz - 40; ++i) stbi__get8(s);
         if (bpp == 16 || bpp == 32) {
            mr = 0xffu << 16; mg = 0xffu << 8; mb = 0xffu; ma = 0xffu << 24;
         }
      }
   }
   
   if (bpp == 24) { s->img_n = 3; }
   else if (bpp == 32) { s->img_n = 4; }
   else { s->img_n = 3; }
   
   if (req_comp && req_comp >= 3) target = req_comp;
   else target = s->img_n;
   
   if (!stbi__at_eof(s)) {
      int cur = (int)(s->img_buffer - s->img_buffer_original);
      if (cur < offset) stbi__skip(s, offset - cur);
   }
   
   *x = s->img_x;
   *y = s->img_y;
   if (comp) *comp = s->img_n;
   
   flip_vertically = ((int)s->img_y) > 0;
   s->img_y = abs((int)s->img_y);
   
   out = (stbi__uint8*)STBI_MALLOC(target * s->img_x * s->img_y);
   if (!out) return (stbi__uint8*)(size_t)stbi__err("outofmem");
   
   if (bpp == 24) {
      pad = (-3 * s->img_x) & 3;
      for (j = 0; j < (int)s->img_y; ++j) {
         int row = flip_vertically ? (s->img_y - 1 - j) : j;
         for (i = 0; i < (int)s->img_x; ++i) {
            int idx = (row * s->img_x + i) * target;
            out[idx + 2] = stbi__get8(s);
            out[idx + 1] = stbi__get8(s);
            out[idx + 0] = stbi__get8(s);
            if (target == 4) out[idx + 3] = 255;
         }
         stbi__skip(s, pad);
      }
   } else if (bpp == 32) {
      for (j = 0; j < (int)s->img_y; ++j) {
         int row = flip_vertically ? (s->img_y - 1 - j) : j;
         for (i = 0; i < (int)s->img_x; ++i) {
            int idx = (row * s->img_x + i) * target;
            out[idx + 2] = stbi__get8(s);
            out[idx + 1] = stbi__get8(s);
            out[idx + 0] = stbi__get8(s);
            if (target == 4) out[idx + 3] = stbi__get8(s);
            else stbi__get8(s);
         }
      }
   }
   
   *x = s->img_x;
   *y = s->img_y;
   return out;
}

// TGA loader
static int stbi__tga_test(stbi__context *s)
{
   int res = 0;
   int sz;
   stbi__get8(s);
   int tga_color_type = stbi__get8(s);
   if (tga_color_type > 1) goto errorEnd;
   sz = stbi__get8(s);
   if (tga_color_type == 1) {
      if (sz != 1 && sz != 9) goto errorEnd;
   } else {
      if (sz != 2 && sz != 3 && sz != 10 && sz != 11) goto errorEnd;
   }
   res = 1;
errorEnd:
   return res;
}

static void *stbi__tga_load(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   int tga_offset = stbi__get8(s);
   int tga_indexed = stbi__get8(s);
   int tga_image_type = stbi__get8(s);
   int tga_is_RLE = 0;
   int tga_palette_start = stbi__get16le(s);
   int tga_palette_len = stbi__get16le(s);
   int tga_palette_bits = stbi__get8(s);
   int tga_x_origin = stbi__get16le(s);
   int tga_y_origin = stbi__get16le(s);
   int tga_width = stbi__get16le(s);
   int tga_height = stbi__get16le(s);
   int tga_bits_per_pixel = stbi__get8(s);
   int tga_comp, tga_rgb16=0;
   int tga_inverted = stbi__get8(s);
   (void)tga_palette_start; (void)tga_palette_bits; (void)tga_x_origin; (void)tga_y_origin;
   
   if (tga_image_type >= 8) { tga_image_type -= 8; tga_is_RLE = 1; }
   tga_inverted = 1 - ((tga_inverted >> 5) & 1);
   
   if (tga_indexed) tga_comp = 3;
   else if (tga_bits_per_pixel == 15 || tga_bits_per_pixel == 16) { tga_comp = 3; tga_rgb16 = 1; }
   else if (tga_bits_per_pixel == 24) tga_comp = 3;
   else if (tga_bits_per_pixel == 32) tga_comp = 4;
   else return (stbi__uint8*)(size_t)stbi__err("bad bits/pixel");
   
   if (tga_offset) stbi__skip(s, tga_offset);
   if (tga_indexed) stbi__skip(s, tga_palette_len * (tga_palette_bits / 8));
   
   *x = tga_width;
   *y = tga_height;
   if (comp) *comp = tga_comp;
   
   int target = req_comp ? req_comp : tga_comp;
   stbi__uint8 *out = (stbi__uint8*)STBI_MALLOC(target * tga_width * tga_height);
   if (!out) return (stbi__uint8*)(size_t)stbi__err("outofmem");
   
   for (int j = 0; j < tga_height; ++j) {
      int row = tga_inverted ? (tga_height - 1 - j) : j;
      for (int i = 0; i < tga_width; ++i) {
         int idx = (row * tga_width + i) * target;
         if (tga_rgb16) {
            int v = stbi__get16le(s);
            out[idx + 0] = (stbi__uint8)(((v >> 10) & 31) * 255 / 31);
            out[idx + 1] = (stbi__uint8)(((v >> 5) & 31) * 255 / 31);
            out[idx + 2] = (stbi__uint8)((v & 31) * 255 / 31);
            if (target == 4) out[idx + 3] = 255;
         } else {
            out[idx + 2] = stbi__get8(s);
            out[idx + 1] = stbi__get8(s);
            out[idx + 0] = stbi__get8(s);
            if (tga_comp == 4) {
               if (target == 4) out[idx + 3] = stbi__get8(s);
               else stbi__get8(s);
            } else if (target == 4) out[idx + 3] = 255;
         }
      }
   }
   return out;
}

static stbi__uint8 *stbi__load_and_postprocess_8bit(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   // Try BMP first
   stbi__uint8 *orig = s->img_buffer;
   if (stbi__bmp_test(s)) {
      s->img_buffer = orig;
      return (stbi__uint8*)stbi__bmp_load(s, x, y, comp, req_comp);
   }
   
   // Try TGA
   s->img_buffer = orig;
   if (stbi__tga_test(s)) {
      s->img_buffer = orig;
      return (stbi__uint8*)stbi__tga_load(s, x, y, comp, req_comp);
   }
   
   return (stbi__uint8*)(size_t)stbi__err("unknown format");
}

#endif // STB_IMAGE_IMPLEMENTATION
