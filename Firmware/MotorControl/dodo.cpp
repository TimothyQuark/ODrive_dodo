#include "dodo.hpp"

Dodo::Dodo() {
    test_val_ = 1000;

}

void Dodo::test_incr() {
    test_val_ += 1;
    foc_.i_alpha += 1.0;
    foc_.i_beta += 1.0;
}
