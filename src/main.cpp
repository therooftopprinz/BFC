#include <iostream>
#include <cstdlib>
#include <Buffer.hpp>

using namespace bfc;

class Test
{
public:
    Test()
    {
        std::cout << __PRETTY_FUNCTION__ << "\n";
    }
    ~Test()
    {
        std::cout << __PRETTY_FUNCTION__ << "\n";
    }
private:
    int a = 1;
    int b = 2;
};

uint8_t allocbuff[1024];
size_t allocbuffindex = 0;
void* operator new(size_t size)
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
    uint8_t* rv = &allocbuff[allocbuffindex];
    allocbuffindex += size;
    return rv;
}

void operator delete(void* ptr)
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
    // free(ptr);
}

void* operator new[](size_t size)
{
    std::cout << __PRETTY_FUNCTION__ << "size :" << size << "\n";
    uint8_t* rv = &allocbuff[allocbuffindex];
    allocbuffindex += size;
    return rv;
}

void operator delete[](void* ptr)
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
    // free(ptr);
}

int main()
{
    Buffer a(0,0);
    ConstBuffer b(0,0);
    uint8_t* ga = a.data();
    // uint8_t* gb = b.data(); // error
    BufferView c(a);
    ConstBufferView d(a);
    // BufferView e(b); // error
    BufferView f(c);
    // BufferView g(d); // error
    ConstBufferView h(c);
    ConstBufferView i(d);

    return 0;
}