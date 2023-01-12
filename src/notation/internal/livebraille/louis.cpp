#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> // for getcwd()
#include <iostream>
#include <sstream>

#include "thirdparty/liblouis/liblouis/internal.h"
#include "thirdparty/liblouis/liblouis/liblouis.h"

#define WIDECHARS_ARE_UCS4

#define FUNC u32_to_u8
#define SRC_UNIT uint32_t
#define DST_UNIT uint8_t

/* Type representing a Unicode character.  */
typedef uint32_t ucs4_t;

int
u8_uctomb (uint8_t *s, ucs4_t uc, int n)
{
  if (uc < 0x80)
    {
      if (n > 0)
        {
          s[0] = uc;
          return 1;
        }
      /* else return -2, below.  */
    }
  else
    {
      int count;

      if (uc < 0x800)
        count = 2;
      else if (uc < 0x10000)
        {
          if (uc < 0xd800 || uc >= 0xe000)
            count = 3;
          else
            return -1;
        }
      else if (uc < 0x110000)
        count = 4;
      else
        return -1;

      if (n >= count)
        {
          switch (count) /* note: code falls through cases! */
            {
            case 4: s[3] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x10000;
            case 3: s[2] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x800;
            case 2: s[1] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0xc0;
          /*case 1:*/ s[0] = uc;
            }
          return count;
        }
    }
  return -2;
}

DST_UNIT *
FUNC (const SRC_UNIT *s, size_t n, DST_UNIT *resultbuf, size_t *lengthp)
{
  const SRC_UNIT *s_end = s + n;
  /* Output string accumulator.  */
  DST_UNIT *result;
  size_t allocated;
  size_t length;

  if (resultbuf != NULL)
    {
      result = resultbuf;
      allocated = *lengthp;
    }
  else
    {
      result = NULL;
      allocated = 0;
    }
  length = 0;
  /* Invariants:
     result is either == resultbuf or == NULL or malloc-allocated.
     If length > 0, then result != NULL.  */

  while (s < s_end)
    {
      ucs4_t uc;
      int count;

      /* Fetch a Unicode character from the input string.  */
      uc = *s++;
      /* No need to call the safe variant u32_mbtouc, because
         u8_uctomb will verify uc anyway.  */

      /* Store it in the output string.  */
      count = u8_uctomb (result + length, uc, allocated - length);
      if (count == -1)
        {
          if (!(result == resultbuf || result == NULL))
            free (result);
          errno = EILSEQ;
          return NULL;
        }
      if (count == -2)
        {
          DST_UNIT *memory;

          allocated = (allocated > 0 ? 2 * allocated : 12);
          if (length + 6 > allocated)
            allocated = length + 6;
          if (result == resultbuf || result == NULL)
            memory = (DST_UNIT *) malloc (allocated * sizeof (DST_UNIT));
          else
            memory =
              (DST_UNIT *) realloc (result, allocated * sizeof (DST_UNIT));

          if (memory == NULL)
            {
              if (!(result == resultbuf || result == NULL))
                free (result);
              errno = ENOMEM;
              return NULL;
            }
          if (result == resultbuf && length > 0)
            memcpy ((char *) memory, (char *) result,
                    length * sizeof (DST_UNIT));
          result = memory;
          count = u8_uctomb (result + length, uc, allocated - length);
          if (count < 0)
            abort ();
        }
      length += count;
    }

  if (length == 0)
    {
      if (result == NULL)
        {
          /* Return a non-NULL value.  NULL means error.  */
          result = (DST_UNIT *) malloc (1);
          if (result == NULL)
            {
              errno = ENOMEM;
              return NULL;
            }
        }
    }
  else if (result != resultbuf && length < allocated)
    {
      /* Shrink the allocated memory if possible.  */
      DST_UNIT *memory;

      memory = (DST_UNIT *) realloc (result, length * sizeof (DST_UNIT));
      if (memory != NULL)
        result = memory;
    }

  *lengthp = length;
  return result;
}

std::string braille_translate(const char *table_name, std::string txt) {
    //cout << "braille_translate " << table_name << " " << txt << "\n";
	uint8_t *outputbuf;
	size_t outlen;
	widechar inbuf[MAXSTRING];
	widechar transbuf[MAXSTRING];
	int inlen;
	int translen;

	inlen = _lou_extParseChars(txt.c_str(), inbuf);

    translen = MAXSTRING;
    lou_translateString(
            table_name, inbuf, &inlen, transbuf, &translen, NULL, NULL, 0);

#ifdef WIDECHARS_ARE_UCS4
        //outputbuf = (uint8_t *) malloc (translen * sizeof(widechar) * sizeof (uint8_t));
		outputbuf = u32_to_u8(transbuf, translen, NULL, &outlen);
#else
		outputbuf = u16_to_u8(transbuf, translen, NULL, &outlen);
#endif

    std::string ret = std::string(outputbuf, outputbuf + outlen);
	free(outputbuf);
	//cout << " " << ret << "\n";
	return ret;
}

int check_tables(const char *tables) {
    if(lou_checkTable(tables) == 0)
        return -1;
    else return 0;
}
char * setTablesDir(const char *tablesdir){
    return lou_setDataPath(tablesdir);
}
char * getTablesDir(){
    return lou_getDataPath();
}
