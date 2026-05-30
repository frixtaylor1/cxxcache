#pragma once

#include "../common/common.hpp"

interface IMutex {
    virtual ~IMutex() {}
    virtual void lock(void) = 0;
    virtual void unlock(void) = 0;
};
