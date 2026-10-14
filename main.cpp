#include "./include/Magma/Magma.h"
using namespace Magma;
int main() {
    constexpr unsigned int windowSize[2] ={1920,1080};
    MInstance magmaInstance("Magma test app",windowSize,WINDOW_MODE::WINDOWED);
    magmaInstance.start();
    return 0;
};