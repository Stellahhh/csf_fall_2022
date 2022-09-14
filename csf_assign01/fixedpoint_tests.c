#include <stdio.h>
#include <stdlib.h>
#include "fixedpoint.h"
#include "tctest.h"

// Test fixture object, has some useful values for testing
typedef struct
{
  Fixedpoint zero;
  Fixedpoint one;
  Fixedpoint one_half;
  Fixedpoint one_fourth;
  Fixedpoint large1;
  Fixedpoint large2;
  Fixedpoint max;

  // TODO: add more objects to the test fixture
} TestObjs;

// functions to create and destroy the test fixture
TestObjs *setup(void);
void cleanup(TestObjs *objs);

// test functions
void test_whole_part(TestObjs *objs);
void test_frac_part(TestObjs *objs);
void test_create_from_hex(TestObjs *objs);
void test_format_as_hex(TestObjs *objs);
void test_negate(TestObjs *objs);
void test_add(TestObjs *objs);
void test_sub(TestObjs *objs);
void test_is_overflow_pos(TestObjs *objs);
void test_is_err(TestObjs *objs);
// TODO: add more test functions
void test_create_from_hex_example(TestObjs *objs);
void compare_two_equal(TestObjs *objs);
void compare_two_pos_equal(TestObjs *objs);
void compare_two_neg_equal(TestObjs *objs);
void compare_two_pos_unequal2(TestObjs *objs);
void test_halve(TestObjs *objs);
void test_double(TestObjs *objs);
void test_compare(TestObjs *objs);
void test_sub2(TestObjs *objs);
void test_add2(TestObjs *objs);
void test_create_from_hex2(TestObjs *objs);
void test_format_as_hex2();

int main(int argc, char **argv)
{
  // if a testname was specified on the command line, only that
  // test function will be executed
  if (argc > 1)
  {
    tctest_testname_to_execute = argv[1];
  }

  TEST_INIT();

  TEST(test_whole_part);
  TEST(test_frac_part);
  TEST(test_create_from_hex);
  TEST(test_format_as_hex);
  TEST(test_negate);
  TEST(test_add);
  TEST(test_sub);
  TEST(test_is_overflow_pos);
  TEST(test_is_err);
  // TEST(test_create_from_hex_example);
  TEST(compare_two_equal);
  TEST(compare_two_pos_equal);
  TEST(compare_two_neg_equal);
  TEST(compare_two_pos_unequal2);
  TEST(test_halve);
  TEST(test_double);
  TEST(test_compare);
  TEST(test_sub2);
  TEST(test_add2);
  TEST(test_create_from_hex2);
  TEST(test_format_as_hex2);

  // IMPORTANT: if you add additional test functions (which you should!),
  // make sure they are included here.  E.g., if you add a test function
  // "my_awesome_tests", you should add
  //
  //   TEST(my_awesome_tests);
  //
  // here. This ensures that your test function will actually be executed.

  TEST_FINI();
}

TestObjs *setup(void)
{
  TestObjs *objs = malloc(sizeof(TestObjs));

  objs->zero = fixedpoint_create(0UL);
  objs->one = fixedpoint_create(1UL);
  objs->one_half = fixedpoint_create2(0UL, 0x8000000000000000UL);
  objs->one_fourth = fixedpoint_create2(0UL, 0x4000000000000000UL);
  objs->large1 = fixedpoint_create2(0x4b19efceaUL, 0xec9a1e2418UL);
  objs->large2 = fixedpoint_create2(0xfcbf3d5UL, 0x4d1a23c24fafUL);
  objs->max = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);

  return objs;
}

void cleanup(TestObjs *objs)
{
  free(objs);
}

void test_whole_part(TestObjs *objs)
{
  ASSERT(0UL == fixedpoint_whole_part(objs->zero));
  ASSERT(1UL == fixedpoint_whole_part(objs->one));
  ASSERT(0UL == fixedpoint_whole_part(objs->one_half));
  ASSERT(0UL == fixedpoint_whole_part(objs->one_fourth));
  ASSERT(0x4b19efceaUL == fixedpoint_whole_part(objs->large1));
  ASSERT(0xfcbf3d5UL == fixedpoint_whole_part(objs->large2));
}

void test_frac_part(TestObjs *objs)
{
  ASSERT(0UL == fixedpoint_frac_part(objs->zero));
  ASSERT(0UL == fixedpoint_frac_part(objs->one));
  ASSERT(0x8000000000000000UL == fixedpoint_frac_part(objs->one_half));
  ASSERT(0x4000000000000000UL == fixedpoint_frac_part(objs->one_fourth));
  ASSERT(0xec9a1e2418UL == fixedpoint_frac_part(objs->large1));
  ASSERT(0x4d1a23c24fafUL == fixedpoint_frac_part(objs->large2));
}

void test_create_from_hex(TestObjs *objs)
{
  (void)objs;

  Fixedpoint val1 = fixedpoint_create_from_hex("f6a5865.00f2");

  ASSERT(fixedpoint_is_valid(val1));

  ASSERT(0xf6a5865UL == fixedpoint_whole_part(val1));

  ASSERT(0x00f2000000000000UL == fixedpoint_frac_part(val1));
}

void test_format_as_hex(TestObjs *objs)
{
  char *s;

  s = fixedpoint_format_as_hex(objs->zero);
  ASSERT(0 == strcmp(s, "0"));
  free(s);

  s = fixedpoint_format_as_hex(objs->one);
  ASSERT(0 == strcmp(s, "1"));
  free(s);

  s = fixedpoint_format_as_hex(objs->one_half);
  ASSERT(0 == strcmp(s, "0.8"));
  free(s);

  s = fixedpoint_format_as_hex(objs->one_fourth);
  ASSERT(0 == strcmp(s, "0.4"));
  free(s);

  s = fixedpoint_format_as_hex(objs->large1);
  ASSERT(0 == strcmp(s, "4b19efcea.000000ec9a1e2418"));
  free(s);

  s = fixedpoint_format_as_hex(objs->large2);
  ASSERT(0 == strcmp(s, "fcbf3d5.00004d1a23c24faf"));
  free(s);
}

void test_negate(TestObjs *objs)
{
  // none of the test fixture objects are negative
  ASSERT(!fixedpoint_is_neg(objs->zero));
  ASSERT(!fixedpoint_is_neg(objs->one));
  ASSERT(!fixedpoint_is_neg(objs->one_half));
  ASSERT(!fixedpoint_is_neg(objs->one_fourth));
  ASSERT(!fixedpoint_is_neg(objs->large1));
  ASSERT(!fixedpoint_is_neg(objs->large2));

  // negate the test fixture values
  Fixedpoint zero_neg = fixedpoint_negate(objs->zero);
  Fixedpoint one_neg = fixedpoint_negate(objs->one);
  Fixedpoint one_half_neg = fixedpoint_negate(objs->one_half);
  Fixedpoint one_fourth_neg = fixedpoint_negate(objs->one_fourth);
  Fixedpoint large1_neg = fixedpoint_negate(objs->large1);
  Fixedpoint large2_neg = fixedpoint_negate(objs->large2);

  // zero does not become negative when negated
  ASSERT(!fixedpoint_is_neg(zero_neg));

  // all of the other values should have become negative when negated
  ASSERT(fixedpoint_is_neg(one_neg));
  ASSERT(fixedpoint_is_neg(one_half_neg));
  ASSERT(fixedpoint_is_neg(one_fourth_neg));
  ASSERT(fixedpoint_is_neg(large1_neg));
  ASSERT(fixedpoint_is_neg(large2_neg));

  // magnitudes should stay the same
  ASSERT(0UL == fixedpoint_whole_part(objs->zero));
  ASSERT(1UL == fixedpoint_whole_part(objs->one));
  ASSERT(0UL == fixedpoint_whole_part(objs->one_half));
  ASSERT(0UL == fixedpoint_whole_part(objs->one_fourth));
  ASSERT(0x4b19efceaUL == fixedpoint_whole_part(objs->large1));
  ASSERT(0xfcbf3d5UL == fixedpoint_whole_part(objs->large2));
  ASSERT(0UL == fixedpoint_frac_part(objs->zero));
  ASSERT(0UL == fixedpoint_frac_part(objs->one));
  ASSERT(0x8000000000000000UL == fixedpoint_frac_part(objs->one_half));
  ASSERT(0x4000000000000000UL == fixedpoint_frac_part(objs->one_fourth));
  ASSERT(0xec9a1e2418UL == fixedpoint_frac_part(objs->large1));
  ASSERT(0x4d1a23c24fafUL == fixedpoint_frac_part(objs->large2));
}

void test_add(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs, sum;

  lhs = fixedpoint_create_from_hex("-c7252a193ae07.7a51de9ea0538c5");
  rhs = fixedpoint_create_from_hex("d09079.1e6d601");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(fixedpoint_is_neg(sum));
  ASSERT(0xc7252a0c31d8eUL == fixedpoint_whole_part(sum));
  ASSERT(0x5be47e8ea0538c50UL == fixedpoint_frac_part(sum));
}

void test_sub(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs, diff;

  lhs = fixedpoint_create_from_hex("-ccf35aa3a04a3b.b105");
  rhs = fixedpoint_create_from_hex("f676e8.58");
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT(fixedpoint_is_neg(diff));
  ASSERT(0xccf35aa496c124UL == fixedpoint_whole_part(diff));
  ASSERT(0x0905000000000000UL == fixedpoint_frac_part(diff));
}

void test_is_overflow_pos(TestObjs *objs)
{
  Fixedpoint sum;

  sum = fixedpoint_add(objs->max, objs->one);
  ASSERT(fixedpoint_is_overflow_pos(sum));

  sum = fixedpoint_add(objs->one, objs->max);
  ASSERT(fixedpoint_is_overflow_pos(sum));

  Fixedpoint negative_one = fixedpoint_negate(objs->one);

  sum = fixedpoint_sub(objs->max, negative_one);
  ASSERT(fixedpoint_is_overflow_pos(sum));
}

void test_is_err(TestObjs *objs)
{
  (void)objs;

  // too many characters
  Fixedpoint err1 = fixedpoint_create_from_hex("88888888888888889.6666666666666666");
  ASSERT(fixedpoint_is_err(err1));

  // too many characters
  Fixedpoint err2 = fixedpoint_create_from_hex("6666666666666666.88888888888888889");
  ASSERT(fixedpoint_is_err(err2));

  // this one is actually fine
  Fixedpoint err3 = fixedpoint_create_from_hex("-6666666666666666.8888888888888888");
  ASSERT(fixedpoint_is_valid(err3));
  ASSERT(!fixedpoint_is_err(err3));

  // whole part is too large
  Fixedpoint err4 = fixedpoint_create_from_hex("88888888888888889");
  ASSERT(fixedpoint_is_err(err4));

  // fractional part is too large
  Fixedpoint err5 = fixedpoint_create_from_hex("7.88888888888888889");
  ASSERT(fixedpoint_is_err(err5));

  // invalid hex digits in whole part
  Fixedpoint err6 = fixedpoint_create_from_hex("123xabc.4");
  ASSERT(fixedpoint_is_err(err6));

  // invalid hex digits in fractional part
  Fixedpoint err7 = fixedpoint_create_from_hex("7.0?4");
  ASSERT(fixedpoint_is_err(err7));
}

// TODO: implement more test functions

// my test 1 to see if create_from_hex is correctly implemented

void test_create_from_hex_example(TestObjs *objs)
{
  (void)objs;

  Fixedpoint val1 = fixedpoint_create_from_hex("f6a5865.00f2");

  printf("%ld\n", (long)fixedpoint_whole_part(val1));

  // ASSERT(fixedpoint_is_valid(val1));

  // ASSERT(0xf6a5865UL == fixedpoint_whole_part(val1));

  // ASSERT(0x00f2000000000000UL == fixedpoint_frac_part(val1));
}

void test_add_two_positive_no_overflow(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs, sum;

  lhs = fixedpoint_create_from_hex("-c7252a193ae07.7a51de9ea0538c5");
  rhs = fixedpoint_create_from_hex("d09079.1e6d601");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(fixedpoint_is_neg(sum));
  ASSERT(0xc7252a0c31d8eUL == fixedpoint_whole_part(sum));
  ASSERT(0x5be47e8ea0538c50UL == fixedpoint_frac_part(sum));
}

void compare_two_pos_equal(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;

  lhs = fixedpoint_create(0x4b19efceaUL);
  rhs = fixedpoint_create(0x4b19efceaUL);
  // rhs.tag = 1;
  ASSERT(fixedpoint_compare(lhs, rhs) == 0);
}

void compare_two_neg_equal(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;

  lhs = fixedpoint_create(0x4b19efceaUL);
  rhs = fixedpoint_create(0x4b19efceaUL);
  rhs.tag = 1;
  lhs.tag = 1;
  ASSERT(fixedpoint_compare(lhs, rhs) == 0);
}

void compare_two_equal(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;

  lhs = fixedpoint_create2(0x4b19efceaUL, 0x4b19efceaUL);
  rhs = fixedpoint_create2(0x4b19efceaUL, 0x4b19efceaUL);
  // rhs.tag = 1;
  ASSERT(fixedpoint_compare(lhs, rhs) == 0);
}

void compare_two_pos_unequal1(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;

  lhs = fixedpoint_create(0x4b19efceaUL);
  rhs = fixedpoint_create(0x4b19eaceaUL);
  // rhs.tag = 1;
  ASSERT(fixedpoint_compare(lhs, rhs) == 1);
}

void compare_two_pos_unequal2(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;

  lhs = fixedpoint_create2(0x4b19efceaUL, 0x4b19eaceaUL);
  rhs = fixedpoint_create2(0x4b19eaceaUL, 0x4b19efceaUL);
  // rhs.tag = 1;
  ASSERT(fixedpoint_compare(lhs, rhs) == 1);
}

// my tests for each function
void test_halve(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;

  // at 0
  lhs = fixedpoint_create2(0UL, 0UL);
  rhs = fixedpoint_create2(0UL, 0UL);
  ASSERT(0 == fixedpoint_compare(fixedpoint_halve(lhs), rhs));

  // 1 at whole part
  lhs = fixedpoint_create(1);
  rhs = fixedpoint_create2(0, (1UL << 63));
  ASSERT(0 == fixedpoint_compare(fixedpoint_halve(lhs), rhs));

  lhs = fixedpoint_create2(4, 6);
  rhs = fixedpoint_create2(2, 3);
  ASSERT(0 == fixedpoint_compare(fixedpoint_halve(lhs), rhs));

  // edge case 1
  lhs = fixedpoint_create2(1, 1UL);
  rhs = fixedpoint_halve(lhs);
  ASSERT(1 == fixedpoint_is_underflow_pos(rhs));

  // edge case 2
  lhs = fixedpoint_create2(1, 1UL);
  lhs = fixedpoint_negate(lhs);
  rhs = fixedpoint_halve(lhs);
  ASSERT(1 == fixedpoint_is_underflow_neg(rhs));
}

void test_double(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;
  // positive
  lhs = fixedpoint_create(3);
  rhs = fixedpoint_create(6);
  ASSERT(0 == fixedpoint_compare(fixedpoint_double(lhs), rhs));

  lhs = fixedpoint_create2(10, 10);
  rhs = fixedpoint_create2(20, 20);
  ASSERT(0 == fixedpoint_compare(fixedpoint_double(lhs), rhs));

  lhs = fixedpoint_create2(10, 10);
  lhs = fixedpoint_negate(lhs);
  rhs = fixedpoint_create2(20, 20);
  rhs = fixedpoint_negate(rhs);
  ASSERT(0 == fixedpoint_compare(fixedpoint_double(lhs), rhs));

  lhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  rhs = fixedpoint_double(lhs);
  ASSERT(1 == fixedpoint_is_overflow_pos(rhs));

  lhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  lhs = fixedpoint_negate(lhs);
  rhs = fixedpoint_double(lhs);
  ASSERT(1 == fixedpoint_is_overflow_neg(rhs));
}

void test_compare(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs;

  lhs = fixedpoint_create2(0UL, 0UL);
  rhs = fixedpoint_create2(0UL, 0UL);
  ASSERT(0 == fixedpoint_compare(lhs, rhs));

  lhs = fixedpoint_create(8);
  rhs = fixedpoint_create2(0UL, 10);
  ASSERT(1 == fixedpoint_compare(lhs, rhs));

  lhs = fixedpoint_create(0);
  rhs = fixedpoint_create2(0UL, 10);
  ASSERT(-1 == fixedpoint_compare(lhs, rhs));

  lhs = fixedpoint_create2(10, 10);
  rhs = fixedpoint_create2(10, 11);
  ASSERT(-1 == fixedpoint_compare(lhs, rhs));

  // different sign
  lhs = fixedpoint_create2(10, 10);
  rhs = fixedpoint_create2(10, 11);
  rhs = fixedpoint_negate(rhs);
  ASSERT(1 == fixedpoint_compare(lhs, rhs));

  lhs = fixedpoint_create2(11, 10);
  rhs = fixedpoint_create2(10, 11);
  ASSERT(1 == fixedpoint_compare(lhs, rhs));

  // same sign, both negative
  lhs = fixedpoint_create2(11, 10);
  rhs = fixedpoint_create2(10, 11);
  lhs = fixedpoint_negate(lhs);
  rhs = fixedpoint_negate(rhs);
  ASSERT(-1 == fixedpoint_compare(lhs, rhs));
}

void test_sub2(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs, diff;

  // two neg values
  lhs = fixedpoint_create_from_hex("-bdb4.20e41");
  rhs = fixedpoint_create_from_hex("-0ebc.e6");
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT(fixedpoint_is_neg(diff));
  ASSERT(0x0000000000aef7UL == fixedpoint_whole_part(diff));
  ASSERT(0x3ae4100000000000UL == fixedpoint_frac_part(diff));

  // pos - pos, result in neg
  lhs = fixedpoint_create_from_hex("d39717c94cd.bbb");
  rhs = fixedpoint_create_from_hex("26653c8341a3d.25b683658f98b5");
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT(fixedpoint_is_neg(diff));
  ASSERT(0x02591a56b7856fUL == fixedpoint_whole_part(diff));
  ASSERT(0x6a0683658f98b500UL == fixedpoint_frac_part(diff));

  // positive overflow
  lhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  rhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  rhs = fixedpoint_negate(rhs);
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT(1 == fixedpoint_is_overflow_pos(diff));

  // negative overflow
  lhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  rhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  lhs = fixedpoint_negate(lhs);
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT(1 == fixedpoint_is_overflow_neg(diff));
}

void test_add2(TestObjs *objs)
{
  (void)objs;

  Fixedpoint lhs, rhs, sum;

  // pos + neg = pos
  lhs = fixedpoint_create_from_hex("ed8f457a935b.bfe7438bcbaeaf");
  rhs = fixedpoint_create_from_hex("-db406ff7b933.ab27989b92b");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(0 == fixedpoint_is_neg(sum));
  ASSERT(0x00124ed582da28UL == fixedpoint_whole_part(sum));
  ASSERT(0x14bfaaf038feaf00UL == fixedpoint_frac_part(sum));

  // pos + neg = neg
  lhs = fixedpoint_create_from_hex("a49.ffba98b0f990");
  rhs = fixedpoint_create_from_hex("-bc69143.5e2");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(fixedpoint_is_neg(sum));
  ASSERT(0x0000000bc686f9UL == fixedpoint_whole_part(sum));
  ASSERT(0x5e65674f06700000UL == fixedpoint_frac_part(sum));

  // pos + pos = pos
  lhs = fixedpoint_create_from_hex("b2f73.841aab4");
  rhs = fixedpoint_create_from_hex("ef064472fa.a9cc503754");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(0 == fixedpoint_is_neg(sum));
  ASSERT(0x0000ef064fa26eUL == fixedpoint_whole_part(sum));
  ASSERT(0x2de6fb7754000000UL == fixedpoint_frac_part(sum));

  // neg + neg = neg
  lhs = fixedpoint_create_from_hex("-1a3b70125e2.d69abbde27bcaa9");
  rhs = fixedpoint_create_from_hex("-07.bb10e442b0f8a");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(fixedpoint_is_neg(sum));
  ASSERT(0x0001a3b70125eaUL == fixedpoint_whole_part(sum));
  ASSERT(0x91aba020d8b54a90UL == fixedpoint_frac_part(sum));

  // positive overflow
  lhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  rhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(1 == fixedpoint_is_overflow_pos(sum));

  // negative overflow
  lhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  rhs = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);
  lhs = fixedpoint_negate(lhs);
  rhs = fixedpoint_negate(rhs);
  sum = fixedpoint_add(lhs, rhs);
  ASSERT(1 == fixedpoint_is_overflow_neg(sum));
}

void test_create_from_hex2(TestObjs *objs)
{
  (void)objs;

  Fixedpoint val1 = fixedpoint_create_from_hex("-.");

  ASSERT(fixedpoint_is_zero(val1));

  val1 = fixedpoint_create_from_hex(".");
  ASSERT(fixedpoint_is_zero(val1));

  val1 = fixedpoint_create_from_hex("-a.");
  ASSERT(0x000000aUL == fixedpoint_whole_part(val1));
  ASSERT(0x0000000000000000UL == fixedpoint_frac_part(val1));

  val1 = fixedpoint_create_from_hex("-.a");
  ASSERT(0x0000000UL == fixedpoint_whole_part(val1));
  ASSERT(0xa000000000000000UL == fixedpoint_frac_part(val1));
}

void test_format_as_hex2()
{
  char *s;

  // no decimal point should be included
  Fixedpoint a = fixedpoint_create(1UL);
  s = fixedpoint_format_as_hex(a);
  ASSERT(0 == strcmp(s, "1"));
  free(s);

  a = fixedpoint_create(1UL);
  a = fixedpoint_negate(a);
  s = fixedpoint_format_as_hex(a);
  ASSERT(0 == strcmp(s, "-1"));
  free(s);
}