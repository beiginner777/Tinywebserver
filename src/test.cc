#include <iostream>
#include "Logger.h"

int main()
{
    std::cout << Logger::getInstance().get() << std::endl;
    std::cout << Logger::getInstance().get() << std::endl;
    std::cout << Logger::getInstance().get() << std::endl;
    return 0;
}