#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include "common.h"

#include "graphics.h"

int main() {
    graphics::initWindow(1920, 1080, true); 

    while (graphics::updateWindow()) {} // TODO: make it so that there is a start and end thing so i can put stuff here perhaps

    return 0;
}
