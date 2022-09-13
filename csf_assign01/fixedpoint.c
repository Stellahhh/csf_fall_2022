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

// need modify
Fixedpoint fixedpoint_create_from_hex(const char *hex)
{
  Fixedpoint final = fixedpoint_create2(0UL, 0UL);

  // exceptions handling for  '-.' '.' will return 0
  if (strcmp(hex, "-.") == 0 || strcmp(hex, ".") == 0)
  {
    return final;
  }

  uint64_t wholeStart = 0; // assume the char is positive

  // check if it is negative
  if (hex[0] == '-')
  {
    final.tag = 1;
    wholeStart = 1;
  }

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

  if (decPos - wholeStart > 16 || !(strlen(hex) == decPos || strlen(hex) - decPos - 1 <= 16))
  {
    final.tag = 2;
    return final;
  }

  // getting number of digits
  // int wholeDigit = decPos - wholeStart;
  // int fracDigit = strlen(hex) - decPos;

  // truncate whole and frac part
  char whole_part[decPos - wholeStart + 1];
  memset(whole_part, '\0', sizeof(whole_part));
  strncpy(whole_part, &hex[wholeStart], decPos - wholeStart);

  char frac_part[strlen(hex) - decPos];
  memset(frac_part, '\0', sizeof(frac_part));
  if (decPos < strlen(hex))
  {
    strcpy(frac_part, &hex[decPos + 1]);
    // change frac to uint64
    final.fraction = (uint64_t)strtoul(frac_part, NULL, 16);
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

// need modify
Fixedpoint fixedpoint_halve(Fixedpoint val)
{

  if ((val.fraction % 2) == 1) // it is odd, underflow happens
  {
    if (fixedpoint_is_neg(val) == 1)
    {
      val.tag = 5; // negagtive underflow
    }
    else
    {
      val.tag = 6; // positive underflow
    }
  }
  val.fraction = val.fraction / 2; // divide frac by two

  if ((val.integer % 2) == 1) // if the integer part is odd
  {                           // need to shift 1 to frac
    val.fraction |= (1UL << 63);
  }
  val.integer = (val.integer >> 1); // divide whole by two
  return val;
}

Fixedpoint fixedpoint_double(Fixedpoint val)
{
  Fixedpoint result = fixedpoint_add(val, val);
  return result;
}

// need modify
int fixedpoint_compare(Fixedpoint left, Fixedpoint right)
{
  int result;
  if (sameSign(left, right) == 1) // same sign
  {
    // first compare the magnitude of whole parts
    if (left.integer > right.integer)
    {
      result = 1;
    }
    else if (left.integer < right.integer)
    {
      result = -1;
    }
    else
    { // integers are the same need to compare fracs
      if (left.fraction > right.fraction)
      { // left frac is bigger
        result = 1;
      }
      else if (left.fraction < right.fraction)
      { // right frac is bigger
        result = -1;
      }
      else
      { // fracs are the same --> all equal
        return 0;
      }
    }
    if (left.tag == 0)
    {
      return result;
    }
    else
    {
      return (-1 * result);
    }
  }
  else // diff sign
  {
    if (fixedpoint_is_zero(left) && fixedpoint_is_zero(right))
    {
      return 0;
    }
    if (left.tag == 1)
    { // left is negative
      return -1;
    }
    return 1; // right is negative
  }
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

// need modify
char *fixedpoint_format_as_hex(Fixedpoint val)
{
  int string_ptr = 0;
  char *s = (char *)malloc(35 * sizeof(char));
  if (val.tag == 1)
  { // insert negative sign if needed
    s[string_ptr] = '-';
    string_ptr++;
  }
  // init imporant vars
  uint64_t ptr = 1;
  ptr = (ptr << 63);
  int back_shift = 60;
  int leading_zero = 1;
  for (int i = 0; i < 16; i++)
  { // loop through all sets of 4 bits of whole
    uint64_t hex = 0;
    for (int j = 0; j < 4; j++)
    { // loop through chunk of 4 bits
      uint64_t temp = (val.integer & ptr);
      hex += (temp >> back_shift);
      ptr = ptr >> 1;
    }
    // convert hex to char
    if (hex >= 10 && hex <= 15)
    { // A - F
      hex += 87;
    }
    else
    { // digits
      hex += 48;
    }
    if (!((leading_zero == 1) && ((hex == 48))))
    {                            // make sure we dont save leading zeros
      s[string_ptr] = (char)hex; // add correct char
      string_ptr++;
      leading_zero = 0;
    }
    back_shift -= 4;
  }
  if (leading_zero == 1)
  { // kill leading zero stipulation
    s[string_ptr] = '0';
    string_ptr++;
    leading_zero = 0;
  }
  if (val.fraction != 0)
  {                      // only add fractional part if one exists
    s[string_ptr] = '.'; // add decimal
    string_ptr++;
    // init impt vars
    ptr = 1;
    ptr = (ptr << 63);
    back_shift = 60;
    int trail_zeros = 0;
    for (int i = 0; i < 16; i++)
    { // loop through 16 chunks of 4 bits
      uint64_t hex = 0;
      for (int j = 0; j < 4; j++)
      { // loop through chunks of 4
        uint64_t temp = (val.fraction & ptr);
        hex += (temp >> back_shift);
        ptr = (ptr >> 1);
      }
      // convert hex to char
      if (hex >= 10 && hex <= 15)
      { // A and F
        hex += 87;
      }
      else
      {
        hex += 48;
      }

      if (hex != 48)
      { // only add a char directly if not a zero
        for (int i = 0; i < trail_zeros; i++)
        { // add saved up zeros
          s[string_ptr] = (char)48;
          string_ptr++;
        }
        s[string_ptr] = (char)hex; // add correct char
        string_ptr++;
        trail_zeros = 0;
      }
      else
      { // if we find a zero tally it up in case we need to add it (if zeros arent trail)
        trail_zeros++;
      }
      back_shift -= 4;
    }
  }
  s[string_ptr] = '\0'; // end string
  return s;
}
