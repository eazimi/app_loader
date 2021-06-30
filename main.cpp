#include <iostream>
#include "app_loader.h"

using namespace std;

int main(int argc, char **argv, char **env)
{
    std::cout << "Hello World!" << std::endl;
    unique_ptr<AppLoader> appLoader(new AppLoader());     
    pair<void *, void *> range;
    appLoader->getReservedMemRange(range);
    std::cout << "getReservedArea(): start = 0x" << std::hex << range.first << " , end = 0x" << range.second << std::endl;
    return 0;
}