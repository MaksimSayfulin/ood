#pragma once
#include <iostream>
#include <ostream>

#include "IDanceBehavior.h"


class Waltz : public IDanceBehavior {
public:
    void Dance() override {
        std::cout << "Waltz" << std::endl;
    };
};