#include "lib/Duck/MallardDuck.h"

int main()
{
    MallardDuck duck;

    duck.Display();
    duck.Dance();

    std::cout << "\n--- Flying ---" << std::endl;
    duck.Fly();
    duck.Fly();
    duck.Fly();
    duck.Fly();
}