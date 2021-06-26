/*
 * Defines several utility helper functions, like safely releasing pointers.
 */

#pragma once

#include "framework.h"

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
