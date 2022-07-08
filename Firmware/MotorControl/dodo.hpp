#ifndef __DODO_HPP
#define __DODO_HPP

#include <autogen/interfaces.hpp>

// Example of how to extend the ODrive API, and create our own interface class
// Note that this class does not have direct access to the Motor Interface variables,
// hence we have some example API there as well (just search dodo in odrive-interface.yaml)

// Configuration settings of the Dodo Class
struct Config_t {
    uint32_t config_val;
};

// Used for testing
struct Test_t {
    uint32_t test_val;
};

// FOC data, stored compactly in a struct
struct FOC_t {
    float i_alpha;
    float i_beta;
};

// DodoIntf is autogenerated when Make is called
class Dodo : public ODriveIntf::DodoIntf {
   public:
    // Dodo();

    uint32_t test_val_{21};

    Config_t config_{
        .config_val{42}};
    Test_t test_{
        .test_val{21}};

    FOC_t foc_ = {
        .i_alpha{10.0f},
        .i_beta{20.0f}};

    void test_incr();
};

#endif /* __DODO_HPP */
