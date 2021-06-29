#include <iostream>
#include <memory>
#include "app_loader.h"

using namespace std;

int main(int argc, char **argv, char **env)
{
    std::cout << "Hello World!" << std::endl;
    std::cout << "sp: " << std::hex << "0x" << unique_ptr<AppLoader>(new AppLoader())->getStackPtr() << std::endl;
    return 0;
}