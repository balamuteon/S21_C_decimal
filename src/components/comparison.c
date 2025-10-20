#include "../s21_decimal.h"

int s21_is_greater_or_equal(s21_decimal a, s21_decimal b) {
  return (decimal_compare(a, b) >= 0);
}

int s21_is_less(s21_decimal a, s21_decimal b) {
  return (decimal_compare(a, b) < 0);
}

int s21_is_less_or_equal(s21_decimal a, s21_decimal b) {
  return (decimal_compare(a, b) <= 0);
}

int s21_is_greater(s21_decimal a, s21_decimal b) {
  return (decimal_compare(a, b) > 0);
}

int s21_is_equal(s21_decimal a, s21_decimal b) {
  return (decimal_compare(a, b) == 0);
}

int s21_is_not_equal(s21_decimal a, s21_decimal b) {
  return (decimal_compare(a, b) != 0);
}
