#include "../s21_decimal.h"

void resetDecimal(s21_decimal *dst) {
  int row = ROW_NUMBER;
  while (row--) dst->bits[row] = 0;
}

bool isSetBit(s21_decimal dst, int index) {
  return (dst.bits[getRow(index)] & (1 << getColumn(index)));
}
bool isScaleSet(s21_decimal dst) { return (dst.bits[3] & 0x00FF0000); }

void setBit(s21_decimal *dst, int index) {
  dst->bits[getRow(index)] |= (1 << getColumn(index));
}

void inverseBit(s21_decimal *dst, int index) {
  dst->bits[getRow(index)] ^= (1 << getColumn(index));
}

void resetBit(s21_decimal *dst, int index) {
  dst->bits[getRow(index)] &= ~((1 << getColumn(index)));
}

bool isDecimalZero(s21_decimal num) {
  return num.bits[0] == 0 && num.bits[1] == 0 && num.bits[2] == 0 &&
         num.bits[3] == 0;
}

int getScale(s21_decimal value) {
  int result = (char)(value.bits[3] >> 16);
  return result;
}

void setScale(s21_decimal *num, int scale) {
  if (isSetBit(*num, MINUS_BIT_INDEX))
    num->bits[3] = (scale << 16) | (1 << getColumn(MINUS_BIT_INDEX));
  else
    num->bits[3] = (scale << 16);
}

bool isCorrectDecimal(s21_decimal *num) {
  return num != NULL && getScale(*num) <= 28 &&
         !(num->bits[3] & (0b01111111000000001111111111111111));
}

void alignScale(s21_decimal *value_1, s21_decimal *value_2, int *errorType) {
  int scale_1 = getScale(*value_1), scale_2 = getScale(*value_2);
  int scale_diff = scale_1 - scale_2;

  if (scale_diff > 0) {
    while (scale_diff-- && !(*errorType)) {
      multiplyBy10(value_2, errorType);
    }
  } else if (scale_diff < 0) {
    while (scale_diff++ && !(*errorType)) {
      multiplyBy10(value_1, errorType);
    }
  }
}

void multiplyBy10(s21_decimal *src, int *errorType) {
  s21_decimal temp = *src;
  shift_left(src, 3, errorType);
  shift_left(&temp, 1, errorType);
  addition(*src, temp, src, errorType);
}

/*
number 1:
row\col | 3 | 2 | 1 | 0 |
-------------------------
0       | 0 | 0 | 0 | 1 |
1       | 0 | 0 | 0 | 0 |
2       | 0 | 0 | 0 | 0 |
*/
void shift_left(s21_decimal *src, int shift, int *errorType) {
  while (shift--) {
    if (isSetBit(*src, VALUE_PART_SIZE - 1)) {
      *errorType = NUMBER_TOO_LARGE;
    }
    for (int bit_index = VALUE_PART_SIZE - 1; bit_index > 0; bit_index--) {
      if (isSetBit(*src, bit_index - 1)) {
        setBit(src, bit_index);
        resetBit(src, bit_index - 1);
      }
    }
    resetBit(src, 0);
  }
}

bool isIntPartBiggerOrEqual(s21_decimal value_1, s21_decimal value_2) {
  bool result = true;
  for (int bit_index = VALUE_PART_SIZE - 1;
       (bit_index >= 0) && result &&
       !(isSetBit(value_1, bit_index) && !isSetBit(value_2, bit_index));
       bit_index--) {
    if (!isSetBit(value_1, bit_index) && isSetBit(value_2, bit_index))
      result = false;
  }
  return result;
}

void copySign(s21_decimal value, s21_decimal *result) {
  if (isSetBit(value, MINUS_BIT_INDEX))
    setBit(result, MINUS_BIT_INDEX);
  else
    resetBit(result, MINUS_BIT_INDEX);
}

void printDecimal(s21_decimal value) {
  for (int i = 0; i < ROW_NUMBER; i++) {
    printf(" value.bits[%d] = ", i);
    for (int j = COLUMN_NUMBER - 1; j >= 0; j--) {
      if (value.bits[i] & (1u << j)) {
        printf("1");
      } else {
        printf("0");
      }
      if (j % 8 == 0) printf(" ");
    }
    printf("=(%d) = 0x%X\n", value.bits[i], value.bits[i]);
  }
}

int getFloatExp(const float *value) {
  fbits tmp;
  tmp.fl = *value;
  return ((tmp.ui & ~(1u << 31)) >> 23) - 127;
}

s21_decimal *setBitFloat(s21_decimal *value, int pos, int bit) {
  if (pos / 32 < 4 && bit)
    value->bits[pos / 32] |= (1 << (pos % 32));
  else if (pos / 32 < 4 && !bit)
    value->bits[pos / 32] &= ~(1 << (pos % 32));
  return value;
}
void setNegativeSign(s21_decimal *value, int bit) {
  value->bits[3] =
      (bit) ? (value->bits[3] | (1u << 31)) : (value->bits[3] & ~(1u << 31));
}

int big_get_sign(big_decimal num) {
  return ((unsigned int)num.bits[6]) >> 31 & 1;
}

int big_get_bit(big_decimal num, int bit) {
  return num.bits[getRow(bit)] & (1 << getColumn(bit));
}

void big_set_bit(big_decimal *num, int bit, unsigned value) {
  if (value) {
    num->bits[getRow(bit)] |= (1 << getColumn(bit));
  } else {
    num->bits[getRow(bit)] &= ~(1 << getColumn(bit));
  }
}

int big_get_scale(big_decimal num) {
  return (((unsigned int)num.bits[6]) >> 16) & 0b11111111;
}

void big_set_scale(big_decimal *num, int scale) {
  num->bits[6] =
      (((((unsigned int)num->bits[6]) >> 16) & (~0b11111111)) | scale) << 16;
}

int big_decimal_is_zero(big_decimal num) {
  return num.bits[0] == 0 && num.bits[1] == 0 && num.bits[2] == 0 &&
         num.bits[3] == 0 && num.bits[4] == 0 && num.bits[5] == 0;
}

void big_add(big_decimal a, big_decimal b, big_decimal *result) {
  char carry = 0;
  *result = (big_decimal){0};
  int scale_1 = big_get_scale(a), scale_2 = big_get_scale(b);

  if (scale_1 != scale_2) scale_normalization(&a, &b);

  big_set_scale(result, scale_1 > scale_2 ? scale_1 : scale_2);

  for (int i = 0; i <= LONG_MANTISSA - 1; i++) {
    int sum = (big_get_bit(a, i)) ^ (big_get_bit(b, i));
    sum ^= carry << getColumn(i);
    if (carry)
      carry = (big_get_bit(a, i) || big_get_bit(b, i));
    else
      carry = (big_get_bit(a, i) && big_get_bit(b, i));
    big_set_bit(result, i, sum);
  }
}

int big_shift_left(big_decimal *src, int shift) {
  int result = 0;
  unsigned int overflow = (unsigned int)src->bits[5] >> (32 - shift);
  if (overflow) {
    result = 1;
  } else {
    overflow = 0;
    for (int i = 0; i < 6; i++) {
      unsigned int temp = src->bits[i];
      src->bits[i] <<= shift;
      src->bits[i] |= overflow;
      overflow = temp >> (32 - shift);
    }
  }
  return result;
}

void big_multiply_by_ten(big_decimal *src) {
  big_decimal temp = *src;
  big_shift_left(src, 3);
  big_shift_left(&temp, 1);
  big_add(*src, temp, src);
}

void scale_normalization(big_decimal *a, big_decimal *b) {
  int scale_1 = big_get_scale(*a);
  int scale_2 = big_get_scale(*b);
  int scale_diff = scale_1 - scale_2;

  if (scale_diff > 0) {
    for (int i = 0; i < abs(scale_diff); i++) big_multiply_by_ten(b);

  } else if (scale_diff < 0) {
    for (int i = 0; i < abs(scale_diff); i++) big_multiply_by_ten(a);
  }
}

big_decimal s21_decimal_to_big_decimal(s21_decimal dec) {
  big_decimal big_dec = {0};
  for (int i = 0; i < 3; i++) {
    big_dec.bits[i] = dec.bits[i];
  }
  big_dec.bits[6] = dec.bits[3];
  return big_dec;
}

int decimal_compare(s21_decimal a, s21_decimal b) {
  big_decimal big_dec_a = s21_decimal_to_big_decimal(a);
  big_decimal big_dec_b = s21_decimal_to_big_decimal(b);
  return big_decimal_compare(big_dec_a, big_dec_b);
}

int big_decimal_compare(big_decimal a, big_decimal b) {
  int resulting_code = 0;
  int sign_of_a = big_get_sign(a);
  int sign_of_b = big_get_sign(b);

  if (sign_of_a != sign_of_b &&
      !(big_decimal_is_zero(a) && big_decimal_is_zero(b))) {
    resulting_code = sign_of_a ? -1 : 1;
  }

  if (resulting_code == 0) scale_normalization(&a, &b);

  if (sign_of_a == 0 && resulting_code == 0) {
    for (int i = 5; i >= 0 && resulting_code == 0; i--) {
      if (((unsigned int)a.bits[i]) > ((unsigned int)b.bits[i])) {
        resulting_code = 1;
      } else if (((unsigned int)a.bits[i]) < ((unsigned int)b.bits[i])) {
        resulting_code = -1;
      }
    }
  } else if (sign_of_a == 1 && resulting_code == 0) {
    for (int i = 5; i >= 0 && resulting_code == 0; i--) {
      if (((unsigned int)a.bits[i]) > ((unsigned int)b.bits[i])) {
        resulting_code = -1;
      } else if (((unsigned int)a.bits[i]) < ((unsigned int)b.bits[i])) {
        resulting_code = 1;
      }
    }
  }

  return resulting_code;
}
