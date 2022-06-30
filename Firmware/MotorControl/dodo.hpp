#ifndef __DODO_HPP
#define __DODO_HPP

#include <autogen/interfaces.hpp>

struct Config_t {
    uint32_t config_val = 42;
};

struct Test_t {
    uint32_t test_val = 21;
};

struct FOC_t {
    // float2D test;
    float i_alpha;
    float i_beta;
};

class Dodo : public ODriveIntf::DodoIntf {
   public:
    Dodo();

    uint32_t test_val_;

    Config_t config_;
    Test_t test_;

    FOC_t foc_ = {
        10.0,
        20.0
    };

    void test_incr();
};

#endif /* __DODO_HPP */
