
#include "SDIL.hpp"
#include <iostream>
#include <utility>
#include <iomanip>
#include <set>

struct A
{
    static int AllocationsCount;

    void* operator new(size_t size)
    {
        std::cout << "Allocate A. Count: " << ++AllocationsCount << std::endl;
        return ::operator new(size);
    }

    void operator delete(void* ptr)
    {
        std::cout << "Dellocate A. Count: " << --AllocationsCount << std::endl;
        ::operator delete(ptr);
    }
};

int A::AllocationsCount = 0;

template<>
struct sdil::SDILTypeTraits<A> 
: SDILTypeTraitsBase,
  sdil::Constructor<A>
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::NotControlled;
};

struct Client
{
    Client(std::shared_ptr<A> shared, std::unique_ptr<A> unique, A* ptr)
    {
        std::set<A*> pointers;
        pointers.insert(shared.get());
        pointers.insert(unique.get());
        pointers.insert(ptr);

        if (pointers.size() != 3)
        {
            exit(1);
        }

        delete ptr;
    }
};

template<>
struct sdil::SDILTypeTraits<Client> 
: SDILTypeTraitsBase,
  sdil::Constructor<Client, std::shared_ptr<A>, std::unique_ptr<A>, A*>
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::NotControlled;
};

struct Client1
{
    Client1(std::weak_ptr<A> ptr) {}
};

template<>
struct sdil::SDILTypeTraits<Client1> 
: SDILTypeTraitsBase,
  sdil::Constructor<Client1, std::weak_ptr<A>>
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::NotControlled;
};

struct Client2
{
    Client2(A& ptr) {}
};

template<>
struct sdil::SDILTypeTraits<Client2> 
: SDILTypeTraitsBase,
  sdil::Constructor<Client2, A&>
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::NotControlled;
};


int main(int argc, char* args[])
{
    sdil::Container container;
    container.Register<A>();

    {
        std::set<A*> pointers;

        auto shared = container.Resolve<A>();
        pointers.insert(shared.get());

        auto unique = container.Resolve<A, sdil::UniquePtr>();
        pointers.insert(unique.get());

        auto ptr = container.Resolve<A, sdil::Pointer>();
        pointers.insert(ptr);

        if (pointers.size() != 3)
        {
            return 1;
        }

        pointers.clear();
        shared.reset();
        unique.reset();
        delete ptr;

        std::cout << "Allocations: " << A::AllocationsCount << std::endl;
        if (A::AllocationsCount != 0)
        {
            return 1;
        }
    }

    std::cout << "\nDependecies resolving test" << std::endl;
    container.Register<Client>();
    container.Resolve<Client, sdil::UniquePtr>();

    std::cout << "\nExceptions test" << std::endl;
    try
    {
        container.Resolve<A, sdil::WeakPtr>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "WeakPtr - Exception caught as expected: " << std::quoted(ex.what()) << std::endl;
    }

    try
    {
        container.Resolve<A, sdil::Reference>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "Reference - Exception caught as expected: " << std::quoted(ex.what()) << std::endl;
    }


    container.Register<Client1>();
    container.Register<Client2>();

    try
    {
        container.Resolve<Client1>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "Client 1 weak ptr dependency - Exception caught as expected: " << std::quoted(ex.what()) << std::endl;
    }

    try
    {
        container.Resolve<Client2>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "Client 2 reference dependency - Exception caught as expected: " << std::quoted(ex.what()) << std::endl;
    }

    return 0;
}

