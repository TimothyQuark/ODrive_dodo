#ifndef __DODO_HPP
#define __DODO_HPP

#include <autogen/interfaces.hpp>

class Dodo : public ODriveIntf::DodoIntf {
public:
    Dodo();

    struct Config_t {
        uint32_t config_val = 42;
    };

    struct Test_t {
        uint32_t test_val = 21;
    };

    uint32_t test_val_;

    Config_t config_;
    Test_t test_;

    void test_incr();
};

#endif /* __DODO_HPP */
