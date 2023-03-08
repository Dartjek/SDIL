#include "SDIL.hpp"
#include <iostream>
#include <utility>

struct ReferenceCounted 
{
    ReferenceCounted()
    {
        ++Count;
        std::cout << "ReferenceCointed is created. Count: " << Count << std::endl;
    }

    ~ReferenceCounted()
    {
        --Count;
        std::cout << "ReferenceCointed is destroyed. Count: " << Count << std::endl;
    }
    static int Count;
};

int ReferenceCounted::Count = 0;


template<>
struct sdil::SDILTypeTraits<ReferenceCounted> : SDILTypeTraitsBase
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::ReferenceCounting;

    static ReferenceCounted* Create()
    {
        return new ReferenceCounted();
    }
};


int main(int argc, char* args[])
{
    sdil::Container container;
    container.Register<ReferenceCounted>();

    auto shared = container.Resolve<ReferenceCounted>();
    auto weak = container.Resolve<ReferenceCounted, sdil::WeakPtr>();
    
    try
    {
        auto& ref = container.Resolve<ReferenceCounted, sdil::Reference>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "Exception caught as expected for reference resolving" << std::endl;
    }

    try
    {
        auto* ref = container.Resolve<ReferenceCounted, sdil::Pointer>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "Exception caught as expected for pointer resolving" << std::endl;
    }

    if (shared.get() != weak.lock().get())
    {
        return 1;
    }
    std::cout << "Shared and weak pointers point to the same object" << std::endl;

    // Test instance release and recreating;
    if (ReferenceCounted::Count != 1)
    {
        return 1;
    }
    
    auto ptr = shared.get();
    std::cout << "Reset shared pointer" << std::endl;
    shared.reset();
    
    if (ReferenceCounted::Count != 0 || !weak.expired())
    {
        std::cout << "Instance was not released" << std::endl;
        return 1;
    }
    
    //auto temp = new char[4];
    std::cout << "Resolve again" << std::endl;
    shared = container.Resolve<ReferenceCounted>();

    if (ptr == shared.get())
    {
        std::cout << "Allocated the same space. Is it Mingw-w64 compiler?" << std::endl;
    }

    return 0;
}

