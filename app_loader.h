#ifndef APP_LOADER_H
#define APP_LOADER_H

class AppLoader
{
    public:
        AppLoader() = default;
        unsigned long getStackPtr();
};

#endif