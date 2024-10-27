//
// Created by Amir on 10/23/2024.
//
#pragma once
#ifndef TREE_NODE_UTILS_H
#define TREE_NODE_UTILS_H

#endif //TREE_NODE_UTILS_H

namespace generic_cfr
{

    template <typename T>
    T GetValFromMem(const Byte* addr)
    {
        T* T_ptr = static_cast<T*>(addr);
        return *T_ptr;
    }

    template <typename T>
    void SetValInMem(Byte* addr, T val)
    {
        T* T_ptr = static_cast<T*>(addr);
        *T_ptr = val;
    }

    template <typename T>
    void AddValInMem(Byte* addr, T val)
    {
        T* T_ptr = static_cast<T*>(addr);
        *T_ptr += val;
    }


}

