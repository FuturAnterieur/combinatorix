#pragma once

#include "logistics_export.h"
#include <stdint.h>
#include <cereal/access.hpp>

class logistics_API numeric_value{
  private:
    float FloatVal{0.f};
    int64_t IntVal{0};

    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive &archive){
      archive(FloatVal, IntVal);
    }
    
  
  public:
    numeric_value();
    numeric_value(int64_t val);
    numeric_value(float val);
    numeric_value(const numeric_value &other);
    

    numeric_value &operator=(int64_t val);
    numeric_value &operator=(float val);
    numeric_value &operator=(const numeric_value &other);

    numeric_value operator*(const numeric_value &other) const;
    numeric_value operator+(const numeric_value &other) const;

    numeric_value operator*(float val) const;
    numeric_value operator+(float val) const;

    bool operator==(const numeric_value &other) const;

    float float_val() const;
    int64_t int_val() const;
};