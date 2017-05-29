#include <string.h>
#include <stddef.h>
#include <ctype.h>

size_t strlen(const char *s)
{
  size_t len;

  for ( len = 0; s[len]; len++ );

  return len;
}

int strcmp(const char *str1, const char *str2)
{
  size_t i = 0;
  while ( str1[i] && str1[i] == str2[i] ) i++; 

  return str1[i] - str2[i];
}

int strcasecmp(const char *str1, const char *str2)
{
  size_t i = 0;
  while ( str1[i] && tolower(str1[i]) == tolower(str2[i]) ) i++;

  return str1[i] - str2[i];
}

int strncmp(const char *str1, const char *str2, size_t len)
{
  size_t i = 0;
  while ( i < len - 1 && str1[i] == str2[i] ) i++;

  return str1[i] - str2[i];
}

int strncasecmp(const char *str1, const char *str2, size_t len)
{
  size_t i = 0;
  while ( i < len - 1 && tolower(str1[i]) == tolower(str2[i]) ) i++;

  return str1[i] - str2[i];
}

char *strcpy(char *dest, const char *src)
{
  size_t i;

  for ( i = 0; src[i]; i++ )
    dest[i] = src[i];

  dest[i] = '\0';

  return dest;
}


 /*
 * if src contains less than len non-null bytes, null bytes will be
 * appended until len bytes have been written. also, if src is more
 * than len bytes long, the copy operation will not end with writing
 * a null byte. both behaviors are in keeping with the ISO C standard
 */
char *strncpy(char *dest, const char *src, size_t len)
{
  size_t i;

  for ( i = 0; src[i] && (i < len); i++ )
    dest[i] = src[i];

  if ( i < len )
  {
    for ( ; i < len; i++ )
      dest[i] = '\0';
  }

  return dest;
}

char *strcat(char *dest, const char *src)
{
  char *ret = dest;
  /* find the end of the destination string */
  while ( *dest++ ); //off by one

  /* copy src to the end of dest, overwriting the null char */
  strcpy(--dest, src); //fixes off by one

  return ret;
}
 /*
 * if src is greater than len bytes in length, strncat writes len+1
 * bytes to dest, the last of which is a null byte
 */
char *strncat(char *dest, const char *src, size_t len)
{
  char *ret = dest;

  /* find the end of the destination string */
  while ( *dest++ ); //off by one

  strncpy(--dest, src, len);
  dest[len] = '\0'; //ensure null termination

  return ret;
}

char *strchr(char *str, int ch)
{
  for ( ; *str ; str++ )
    if ( *str == (char)ch )
      return str;

  return NULL;
}

char *strrchr(char *str, int ch)
{
  char *sp = NULL;

  for ( ; *str; str++)
    if ( *str == (char)ch )
      sp = str;

  return sp;
}

/* brace pyramids are abominations, but it's more readable this way >:( */
char *strstr(char *str1, const char *str2)
{

  for (size_t i = 0; *str1; str1++, i = 0 )
  {
    /* matches first char of string we're looking for */
    if ( *str1 == *str2 )
    {
      /* do the rest match? */
      for ( i = 1; str1[i] == str2[i]; )
      {
        i++;
        /* we found the search string in the searched string */
        if ( !str2[i] ) return str1;
        /* we've reached the end of the searched string without reaching
           the end of the search string, continuing this algorithm just
           wastes cpu time so let's not */
        if ( !str1[i] ) return NULL;
      }
    }
  }
}

char *strpbrk(char *str, char *targets)
{
  for ( ; *str; str++ )
  {
    for ( char *sp = targets; *sp; sp++ )
    {
      if ( *str == *sp ) return sp;
    }
  }

  return NULL;
}

void *memset(void *arr, int val, size_t len)
{
  for (size_t i = 0; i < len; i++ )
    ((unsigned char *)arr)[i] = (unsigned char)val;

  return arr;
}

int memcmp(const void *m1, const void *m2, size_t len)
{
  size_t i;
  for ( i = 0; i < len; i++ )
  {
    /* these two ifs are long for 'if m1 is greater than/less than m2' */
    if ( ((unsigned char *)m1)[i] > ((unsigned char *)m2)[i] )
      return 1;

    if ( ((unsigned char *)m1)[i] < ((unsigned char *)m2)[i] )
      return -1;
  }
  return 0;
}

void *memcpy(void *dest, const void *src, size_t len)
{
  for ( size_t i = 0; i < len; i++ )
    ((unsigned char *)dest)[i] = ((unsigned char *)src)[i];

  return dest;
}

/* MUST BE REIMPLEMENTED ONCE MALLOC IS IMPLEMENTED */
/* current implementation is vulnerable to a stack overflow */
void *memmove(void *dest, const void *src, size_t len)
{
  unsigned char cpy[len];
  memcpy(cpy, src, len);
  return memcpy(dest, cpy, len);
}

void *memchr(void *haystack, int needle, size_t size)
{
  for ( ; size--; haystack++ )
    if ( *(char *)haystack == (char)needle )
      return haystack;

  return NULL;
}
