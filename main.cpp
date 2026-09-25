#include "./include/Magma/Magma.h"
using namespace Magma;
int main() {
    constexpr unsigned int windowSize[2] ={1920,1080};
    MInstance magmaInstance("Test",windowSize,WINDOW_MODE::BORDERLESS);
magmaInstance.start();
    return 0;
};