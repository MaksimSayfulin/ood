#pragma once
#include <iostream>

#include "IDanceBehavior.h"

class Minuet: public IDanceBehavior {
public:
    void Dance() override {
        std::cout << "Minuet" << std::endl;
    }
};