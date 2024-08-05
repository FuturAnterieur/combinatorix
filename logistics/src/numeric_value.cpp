#include "numeric_value.h"

#include <math.h>

numeric_value::numeric_value(){
}

numeric_value::numeric_value(int64_t val){
  IntVal = val;
  FloatVal = static_cast<float>(val);
}

numeric_value::numeric_value(float val){
  FloatVal = val;
  IntVal = static_cast<int64_t>(val);
}

numeric_value::numeric_value(const numeric_value &other){
  IntVal = other.IntVal;
  FloatVal = other.FloatVal;
}

numeric_value &numeric_value::operator=(int64_t val){
  IntVal = val;
  FloatVal = static_cast<float>(val);
  return *this;
}

numeric_value &numeric_value::operator=(float val){
  FloatVal = val;
  IntVal = static_cast<int64_t>(val);
  return *this;
}

numeric_value &numeric_value::operator=(const numeric_value &other){
  IntVal = other.IntVal;
  FloatVal = other.FloatVal;
  return *this;
}

numeric_value numeric_value::operator*(const numeric_value &other) const{
  numeric_value result(*this);
  result.FloatVal *= other.FloatVal;
  result.IntVal = static_cast<int64_t>(roundf(result.FloatVal));
  return result;
}

numeric_value numeric_value::operator+(const numeric_value &other) const{
  numeric_value result(*this);
  result.FloatVal += other.FloatVal;
  result.IntVal = static_cast<int64_t>(roundf(result.FloatVal));
  return result;
}

numeric_value numeric_value::operator*(float val) const{
  numeric_value result(*this);
  result.FloatVal *= val;
  result.IntVal = static_cast<int64_t>(roundf(result.FloatVal));
  return result;
}

numeric_value numeric_value::operator+(float val) const{
  numeric_value result(*this);
  result.FloatVal += val;
  result.IntVal = static_cast<int64_t>(roundf(result.FloatVal));
  return result;
}

bool numeric_value::operator==(const numeric_value &other) const
{
  return IntVal == other.IntVal && fabsf(FloatVal - other.FloatVal) < 0.005f;
}

float numeric_value::float_val() const{
  return FloatVal;
}

int64_t numeric_value::int_val() const{
  return IntVal;
}