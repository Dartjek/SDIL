
#include "SDIL.hpp"
#include <iostream>
#include <iomanip>
#include <utility>

struct Singleton { };

template<>
struct sdil::SDILTypeTraits<Singleton> : SDILTypeTraitsBase
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;

    static Singleton* Create()
    {
        return new Singleton();
    }
};

struct Client
{
    Client(std::shared_ptr<Singleton> shared, sdil::SharedPtr<Singleton> shared2, std::weak_ptr<Singleton> weak, Singleton& ref, Singleton* ptr)
    {
        std::cout << "Constructor of Client" << std::endl;
        if (ptr != shared.get() || ptr != shared2.get() || ptr != weak.lock().get() || ptr != &ref)
        {
            exit(1);
        }
    }
};

template<>
struct sdil::SDILTypeTraits<Client> : SDILTypeTraitsBase
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;

    static Client* Create(std::shared_ptr<Singleton> shared, sdil::SharedPtr<Singleton> shared2, std::weak_ptr<Singleton> weak, Singleton& ref, Singleton* ptr)
    {
        std::cout << "Creating instance of Client" << std::endl;
        return new Client(shared, shared2, weak, ref, ptr);
    }
};

struct Client2
{
    Client2(std::unique_ptr<Singleton> singleton)
    {
        
    }
};

template<>
struct sdil::SDILTypeTraits<Client2> : SDILTypeTraitsBase
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;

    static Client2* Create(std::unique_ptr<Singleton> unique)
    {
        return new Client2(std::move(unique));
    }
};

int main(int argc, char* args[])
{
    sdil::Container container;
    container.Register<Singleton>();

    auto* ptr = container.Resolve<Singleton, sdil::Pointer>();
    auto weak = container.Resolve<Singleton, sdil::WeakPtr>();

    {    
        auto shared = container.Resolve<Singleton>();
        auto& ref = container.Resolve<Singleton, sdil::Reference>();

        if (ptr != &ref || ptr != shared.get() || ptr != weak.lock().get())
        {
            return 1;
        }
    }

    if (ptr != weak.lock().get())
    {
        return 1;
    }

    try
    {
        container.Resolve<Singleton, sdil::UniquePtr>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "Exception caught as expected: " << std::quoted(ex.what()) << std::endl;
    }
    
    container.Register<Client>();
    container.Resolve<Client>();

    container.Register<Client2>();
    
    try
    {
        std::cout << "Construct instance of Client2" << std::endl;
        container.Resolve<Client2>();
        return 1;
    }
    catch(sdil::SDILException& ex)
    {
        std::cout << "Exception caught as expected: " << std::quoted(ex.what()) << std::endl;
    }
    
    
    return 0;
}

