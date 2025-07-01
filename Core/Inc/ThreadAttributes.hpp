#pragma once
#include "cmsis_os.h"

struct ThreadAttributes {
    osThreadAttr_t attr = {};

    ThreadAttributes(const char* name, uint32_t stacKSize, osPriority_t priority) {
        attr.name = name;
        attr.stack_size = stacKSize;
        attr.priority = priority;
    }

    const osThreadAttr_t* operator &() const {
        return &attr;
    }
};