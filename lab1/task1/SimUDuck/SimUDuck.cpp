#include <cstdlib>
#include "lib/DuckFunctions.h"
#include "lib/Duck/DecoyDuck.h"
#include "lib/Duck/MallardDuck.h"
#include "lib/Duck/ModelDuck.h"
#include "lib/Duck/RedHeadDuck.h"
#include "lib/Duck/RubberDuck.h"


int main()
{
    MallardDuck mallardDuck;
    PlayWithDuck(mallardDuck);

    RedheadDuck redheadDuck;
    PlayWithDuck(redheadDuck);

    RubberDuck rubberDuck;
    PlayWithDuck(rubberDuck);

    DecoyDuck decoyDuck;
    PlayWithDuck(decoyDuck);

    ModelDuck modelDuck;
    PlayWithDuck(modelDuck);

    modelDuck.SetFlyBehavior(std::make_unique<FlyWithWings>());
    PlayWithDuck(modelDuck);

    redheadDuck.Dance();
    mallardDuck.Dance();
    redheadDuck.Fly();
    redheadDuck.Fly();
    redheadDuck.Fly();
    redheadDuck.Fly();
    redheadDuck.Fly();
}
