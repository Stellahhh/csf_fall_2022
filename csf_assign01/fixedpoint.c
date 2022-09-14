#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "fixedpoint.h"

Fixedpoint fixedpoint_create(uint64_t whole)
{
  Fixedpoint target;
  target.integer = whole;
  target.fraction = 0UL;

  // initialize
  target.tag = 0;
  return target;
}

Fixedpoint fixedpoint_create2(uint64_t whole, uint64_t frac)
{
  Fixedpoint target = fixedpoint_create(whole);
  target.fraction = frac;

  return target;
}

Fixedpoint fixedpoint_create_from_hex(const char *hex)
{
  // initialize
  Fixedpoint final = fixedpoint_create2(0UL, 0UL);
  // exceptions handling for  '-.' '.' will return 0
  if (strcmp(hex, "-.") == 0 || strcmp(hex, ".") == 0)
  {
    return final;
  }

  uint64_t wholeStart = 0; // assume the char is positive
  // check if it is negative
  wholeStart = (hex[0] == '-') ? 1 : 0;
  final.tag = (hex[0] == '-') ? 1 : 0;

  // get the start index of the fraction part
  uint64_t decPos = strlen(hex); // default value, need to be changed
  for (size_t i = wholeStart; i < strlen(hex); i++)
  {
    if (hex[i] == '.' && decPos == strlen(hex))
    {
      decPos = i;
    }
    else if (!((hex[i] >= '0' && hex[i] <= '9') || (hex[i] >= 'A' && hex[i] <= 'F') || (hex[i] >= 'a' && hex[i] <= 'f')))
    {
      final.tag = 2;
      return final;
    }
  }

  // getting number of digits
  int wholeDigit = decPos - wholeStart;
  int fracDigit = strlen(hex) - decPos;
  if (wholeDigit > 16 || !(strlen(hex) == decPos || fracDigit - 1 <= 16))
  {
    final.tag = 2;
    return final;
  }

  // truncate whole and frac part
  char whole_part[wholeDigit + 1];
  char frac_part[strlen(hex) - decPos];
  memset(whole_part, '\0', sizeof(whole_part));
  memset(frac_part, '\0', sizeof(frac_part));
  strncpy(whole_part, &hex[wholeStart], wholeDigit);
  if (decPos < strlen(hex))
  {
    strcpy(frac_part, &hex[decPos + 1]);
    // char to uint64
    final.fraction = (uint64_t)strtoul(frac_part, NULL, 16);
    // padding 0
    final.fraction = final.fraction << ((16 - strlen(frac_part)) * 4);
  }
  else
  {
    final.fraction = 0UL;
  }

  // change whole to uint64
  final.integer = (uint64_t)strtoul(whole_part, NULL, 16);
  return final;
}

uint64_t fixedpoint_whole_part(Fixedpoint val)
{
  return val.integer;
}

uint64_t fixedpoint_frac_part(Fixedpoint val)
{
  return val.fraction;
}

Fixedpoint fixedpoint_add(Fixedpoint left, Fixedpoint right)
{
  // create a new fixedpoint to represent the sum
  Fixedpoint sum = fixedpoint_create2(0UL, 0UL);
  if (left.tag == right.tag) // if same sign
  {
    // determine if the result is negative or positive
    sum.tag = (left.tag == 1) ? 1 : 0;
    // performing addition
    sum.integer = left.integer + right.integer;
    sum.fraction = left.fraction + right.fraction;
    if ((sum.integer < left.integer) || (sum.integer < right.integer))
    { // if it is overflow
      sum.tag = (left.tag == 1) ? 3 : 4;
    }
    if (sum.fraction < left.fraction) // if carry happens
    {
      sum.integer += 1;
    }
  }
  else
  { // diff sign
    Fixedpoint big = left;
    Fixedpoint small = right;
    if (left.integer < right.integer || (left.integer == right.integer && left.fraction < right.fraction))
    { // right is larger
      big = right;
      small = left;
    }
    // performing addition
    sum.integer = big.integer - small.integer;
    if (big.fraction < small.fraction)
    { // need carry
      if (sum.integer >= 1)
      { // positive sum
        sum.fraction = -small.fraction + big.fraction;
        sum.integer -= 1;
      }
      else
      { // negative sum
        sum.fraction = small.fraction - big.fraction;
      }
    }
    else
    { // no carry
      sum.fraction = -small.fraction + big.fraction;
    }
    sum.tag = big.tag; // sign goes with the big one
  }
  return sum;
}

Fixedpoint fixedpoint_sub(Fixedpoint left, Fixedpoint right)
{
  // initialize
  Fixedpoint result = fixedpoint_create2(0UL, 0UL);
  result = fixedpoint_add(left, fixedpoint_negate(right));

  if (left.tag != right.tag && (result.integer < left.integer || result.integer < right.integer))
  {
    // handle positive overflow
    if (left.tag == 0)
    {
      result.tag = 4;
    }
    // handle negative overflow
    if (left.tag == 1)
    {
      result.tag = 3;
    }
  }

  return result;
}

Fixedpoint fixedpoint_negate(Fixedpoint val)
{
  if (fixedpoint_is_zero(val))
  {
    return val;
  }
  // change sign
  val.tag = (val.tag == 1) ? 0 : 1;
  return val;
}

Fixedpoint fixedpoint_halve(Fixedpoint val)
{
  int isOddFrac = (val.fraction % 2);
  // performing division
  val.fraction = val.fraction / 2;

  if (isOddFrac == 1) // it is odd, underflow happens
  {
    // determine if it's negative/positive underflow
    val.tag = (fixedpoint_is_neg(val) == 1) ? 5 : 6;
  }
  if ((val.integer % 2) == 1) // odd whole part
  {
    val.fraction |= (1UL << 63); // shift 1
  }

  // performing division
  val.integer = (val.integer / 2);
  return val;
}

Fixedpoint fixedpoint_double(Fixedpoint val)
{
  Fixedpoint result = fixedpoint_add(val, val);
  return result;
}

int fixedpoint_compare(Fixedpoint left, Fixedpoint right)
{
  int result;
  if (sameSign(left, right) == 1) // same sign
  {
    if (left.integer == right.integer) // same whole part
    {
      if (left.fraction == right.fraction) // same frac part
      {
        return 0;
      }
      else if (left.fraction > right.fraction)
      { // left frac greater magnitude
        result = (left.tag == 1) ? -1 : 1;
      }
      else
      {
        result = (left.tag == 1) ? 1 : -1;
      }
    }
    else // diff whole part
    {
      if (left.integer > right.integer)
      {
        result = (left.tag == 1) ? -1 : 1;
      }
      else if (left.integer < right.integer)
      {
        result = (left.tag == 1) ? 1 : -1;
      }
    }
  }
  else // diff sign
  {
    if (fixedpoint_is_zero(left) && fixedpoint_is_zero(right))
    {
      return 0;
    }
    result = (left.tag == 1) ? -1 : 1;
  }
  return result;
}

int sameSign(Fixedpoint left, Fixedpoint right)
{
  if (left.tag == right.tag && (left.tag == 0 || left.tag == 1))
  {
    return 1;
  }
  return 0;
}

int fixedpoint_is_zero(Fixedpoint val)
{
  if ((val.tag == 0 || val.tag == 1) && val.integer == 0 && val.fraction == 0)
  {
    // verify val is valid and val is zero for both whole part and fraction
    return 1;
  }
  return 0;
}

int fixedpoint_is_err(Fixedpoint val)
{
  if (val.tag == 2)
  {
    return 1;
  }
  return 0;
}

int fixedpoint_is_neg(Fixedpoint val)
{
  if (val.tag == 1)
  {
    return 1;
  }
  return 0;
}

int fixedpoint_is_overflow_neg(Fixedpoint val)
{
  if (val.tag == 3)
  {
    return 1;
  }
  return 0;
}

int fixedpoint_is_overflow_pos(Fixedpoint val)
{
  if (val.tag == 4)
  {
    return 1;
  }
  return 0;
}

int fixedpoint_is_underflow_neg(Fixedpoint val)
{
  if (val.tag == 5)
  {
    return 1;
  }
  return 0;
}

int fixedpoint_is_underflow_pos(Fixedpoint val)
{
  if (val.tag == 6)
  {
    return 1;
  }
  return 0;
}

int fixedpoint_is_valid(Fixedpoint val)
{
  if (val.tag == 0 || val.tag == 1)
  {
    return 1;
  }
  return 0;
}

char *fixedpoint_format_as_hex(Fixedpoint val)
{
  char *result = malloc(35);

  if (val.tag == 1 && val.fraction == 0)
  { // negative whole only
    sprintf(result, "-%lx", val.integer);
  }
  if (val.tag == 1 && val.fraction != 0)
  { // negative both whole and frac
    sprintf(result, "-%lx.%016lx", val.integer, val.fraction);

    // remove zeros in frac
    int curr = strlen(result) - 1;
    while (result[curr] == '0')
    {
      curr--;
    }
    result[curr + 1] = '\0';
  }
  if (val.tag == 0 && val.fraction == 0)
  { // positive whole only
    sprintf(result, "%lx", val.integer);
  }
  if (val.tag == 0 && val.fraction != 0)
  { // positive whole and frac
    sprintf(result, "%lx.%016lx", val.integer, val.fraction);

    // remove zeros in frac
    int curr = strlen(result) - 1;
    while (result[curr] == '0')
    {
      curr--;
    }
    result[curr + 1] = '\0';
  }

  return result;
}
