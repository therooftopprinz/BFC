#pragma once

#include <typeinfo>
#include <exception>

class Node
{
public:
    template <typename T>
    T& data()
    {
        
    }
private:
    void* data;
    const std::type_info *dataType;
};