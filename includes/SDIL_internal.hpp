#ifndef SDIL_SDIL_INTERNAL_HPP
#define SDIL_SDIL_INTERNAL_HPP

#include <map>
#include <memory>
#include <string>
#include <type_traits>
namespace sdil
{
    // Forward Declarations
    template<class Type> struct SDILTypeTraits;
    class Container;
    using TypeId = size_t;
    using Overrides = std::map<TypeId, std::string>;

    struct SDILTypeTraitsBase
    {
        static constexpr bool HasDeleteFunction = false;
    };

    template<class ReturnType, class ... Args>
    struct Constructor
    {
        static ReturnType* Create(Args... args)
        {
            return new ReturnType(std::forward<Args>(args)...);
        }
    };

    enum class LifeTimeScope
    {
        NotControlled,
        Singleton,
        ReferenceCounting,
    };

    // Templates
    template<class T, class ... Args>
    using SharedPtr = std::shared_ptr<T>;

    template<class T, class D = std::default_delete<T>>
    using UniquePtr = std::unique_ptr<T, D>;

    template<class T, class ... Args>
    using WeakPtr = std::weak_ptr<T>;

    template<class T, class ... Args>
    using Reference = T&;

    template<class T, class ... Args>
    using Pointer = T*;

    template<class T, class ... Args>
    using NoWrapper = T;

    namespace internal
    {
        enum class WrapperType
        {
            None,
            Raw,
            Reference,
            Unique,
            Shared,
            Weak
        };

        template<class T>
        struct WrapperInfo
        {
            using Type = T;

            template<class P, class ... Args>
            using Wrapper = NoWrapper<P>;

            constexpr static WrapperType GetWrapperType() { return WrapperType::None; }
        };

        template<class T, template <class P, class ... PArgs> class TWrapper, class ... TArgs>
        struct WrapperInfo<TWrapper<T, TArgs...>>
        {
            using Type = T;
        
            template<class P, class ... PArgs>
            using Wrapper = TWrapper<P>;

            constexpr static WrapperType GetWrapperType()
            {
                if constexpr(std::is_same_v<Wrapper<T>, UniquePtr<T>>)
                {
                    return WrapperType::Unique;
                }
                else if constexpr(std::is_same_v<Wrapper<T>, SharedPtr<T>>)
                {
                    return WrapperType::Shared;
                }
                else if constexpr(std::is_same_v<Wrapper<T>, WeakPtr<T>>)
                {
                    return WrapperType::Weak;
                }
            };
        };

        template<class T>
        struct WrapperInfo<T*>
        {
            using Type = T;

            template<class P, class ... PArgs>
            using Wrapper = Pointer<P>;

            constexpr static WrapperType GetWrapperType()
            {
                return WrapperType::Raw;
            };
        };

        template<class T>
        struct WrapperInfo<T&>
        {
            using Type = T;

            template<class P, class ... PArgs>
            using Wrapper = Reference<P>;

            constexpr static WrapperType GetWrapperType()
            {
                return WrapperType::Reference;
            };
        };

    }
}

namespace sdil::internal
{
    template<class Type>
    constexpr bool AlwaysFalse = false;

    struct TypeKey
    {
        TypeId type_id;
        std::string type_name;
    };

    using FactoryMethod = void*(Container*, const TypeKey&);
    using DeleteMethod = void(void*);

    struct TypeRecord
    {
        LifeTimeScope lifetime;
        Overrides overrides;
        FactoryMethod* create;
        DeleteMethod* deleter;
    };

    inline bool operator<(const TypeKey& left, const TypeKey& right) noexcept
    {
        if (left.type_id != right.type_id) return left.type_id < right.type_id;
        return left.type_name < right.type_name;
    }

    template<class Interface, class FactoryFunctionType>
    struct Factory
    {
        static_assert(AlwaysFalse<FactoryFunctionType>, "Type should be functional type");
    };
}

namespace std
{
    template<>
    struct hash<sdil::internal::TypeKey>
    {
        inline size_t operator()(sdil::internal::TypeKey const & type_record) const noexcept
        {
            return hash<sdil::TypeId>()(type_record.type_id) ^ hash<string>()(type_record.type_name);
        }
    };
}

#endif //SDIL_SDIL_INTERNAL_HPP
