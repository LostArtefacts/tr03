#ifndef GLOBAL_H
#define GLOBAL_H

#define ArraySize(x) (sizeof(x) / sizeof((x)[0]))

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

#endif
