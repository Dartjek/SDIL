SDIL: Simple Depandancy Injection Library
=========================================

SDIL was developed with purpose provide library which possible easily modify. The library doesn't follow a aim to provide rich functionality such as injection via method or to build a container during compile time. Instead to provide a code that quite easy to understand and container that can be modified during runtime. It uses MIT licence to allow use it everyone as template.

API was inspired by Unity Container for C#

How is it organized?
--------------------
There are three primary files:
* includes/SDIL.hpp - contains public interface of library
* includes/SDIL_internal.hpp - hide all symbols(structs, methods and etc.) needed for work the library
* source/SDIL.cpp - contains non-template code.

And two namespaces:
* *sdil* - here all classes and methods needed for work
* *sdil::internal* - hides everything that is neccessary to work the library

There are also tests folder that contains tests, which can be used as reference.

Where it works?
---------------
It was checked against three compilers: MSVC, Cland, MinGW-w64 gcc and uses C++17 standard.

How to use it?
---------------
In order to register arbitary class in container you define special class(SDILTypeTraits) provides information to container how to create a instance of the class.

Below is example

1. Specialize template SDILTypeTraits
    ```
    class Implementation : public Interface { /* code, declaration, etc. */ };

    template<>
    struct sdil::SDILTypeTraits<Implementation> : SDILTypeTraitsBase {

        // Other possible values: Not controlled, ReferenceCounting
        static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;
        
        static Implementation* Create(std::shared_ptr<Dependency1> d1, 
                                    std::weak_ptr<Dependency2> d2,
                                    std::unique_ptr<Dependency3> d3,
                                    Dependency4* d4,
                                    Dependency5& d5,
                                    );

        // Optionally. Set to provide custom delete function, also SDILTypeTraitsBase can be removed.
        // it just defines variable below with false value
        static constexpr bool HasDeleteFunction = true;

        // Optinally too.
        static void Delete(Implementation* instance);
    }

    // Another possible way
    //template<>
    //struct sdil::SDILTypeTraits<Implementation> 
    //: SDILTypeTraitsBase, 
    //  sdil::Constructor<Implementation, std::shared_ptr<Dependency1> d1, ...>
    //{
    //    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;
    //}

    ```

2. Then you need to register type in container. The register method accepts arbitary name, and overrides(aka map<TypeId, std::string>)

    ```
    void ConfigureContainer(sdil::container& container) {
        container.Register<Implementation, Interface>();
        container.Register<AnotherImplementaion, Interface>("second_impl");
        container.Register<AnotherImplementaion, Interface>("third_impl", {
            { sdil::GetTypeId<Dependency1>(), "another_implementation_dependency1" },
            { sdil::GetTypeId<Dependency2>(), "another_implementation_dependency2" }
        });
    }
    ```
3. Resolve you class
    ```
    container.Resolve<Interface>(); // Can accept a name
    ```

Limitaions
----------
Arguments of constructor can be only smart or raw pointers and references. What can be used depends on type of lifetime scope. For example, **Singleton** dependency cannot be passed as unique_ptr. **ReferenceCounting** type can be passed only as shared_ptr and weak_ptr. **NotControlled** type cannot be passed as reference and weak_ptr. There is check that throws **SDILException** if conditions are violated.

Tags: cpp; ioc; dependency injection; c++17