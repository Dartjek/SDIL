#ifndef SDIL_SDIL_HPP
#define SDIL_SDIL_HPP

#include <string_view>
#include <functional>
#include <variant>
#include <stdexcept>

#include "SDIL_internal.hpp"

namespace sdil
{
    template<class Type>
    inline TypeId GetTypeId()
    {
        return reinterpret_cast<TypeId>(&GetTypeId<Type>);
    }

    template<class Type>
    struct SDILTypeTraits
    {
        static_assert(internal::AlwaysFalse<Type>, "SDILTypeTraits should be specialized for Type");

        /// LifeTime defines whether container store Shared or Weak pointer to object
        static constexpr LifeTimeScope LifeTime = LifeTimeScope::NotControlled;

        /// Create creates instance of type which will be handled by container.
        /// Can contains any number parameters which be reference, raw or smart pointer to type registered in container.
        static void* Create() { return {}; }
    };

    class SDILException : public std::logic_error
    {
        using logic_error::logic_error;
    };

    class Container
    {
    public:
        template<class Type, class Interface = Type>
        inline bool Register(std::string_view name = "", const Overrides& overrides = {})
        {
            const auto type_key = GetTypeKey<Interface>(name);

            using Factory = internal::Factory<Interface, decltype(SDILTypeTraits<Type>::Create)>;
            internal::TypeRecord type_record {
                SDILTypeTraits<Type>::LifeTime,
                overrides,
                &Factory::Create,
                &Factory::Delete
            };
            auto insertion_result = type_registry.emplace(type_key, std::move(type_record));
            return insertion_result.second;
        }

        template<class Interface, template <class P, class ... PArgs> class Wrapper = SharedPtr>
        inline Wrapper<Interface> Resolve(std::string_view name = "")
        {
            const auto type_key = GetTypeKey<Interface>(name);

            CheckWrapperType(internal::WrapperInfo<Wrapper<Interface>>::GetWrapperType(), type_key);
            VariantPtr variant_ptr = Resolve(type_key);

            return CastVariantPtrTo<Interface, Wrapper>(variant_ptr, type_key);
        }

        template<class Dependency>
        inline std::string_view GetOverride(const internal::TypeKey& interface)
        {
            return GetOverride(interface, GetTypeId<Dependency>());
        }

    private:
        using VariantPtr = std::variant<void*, std::shared_ptr<void>, std::weak_ptr<void>>;

        void CheckWrapperType(internal::WrapperType wrapper_type, const internal::TypeKey& interface);
        VariantPtr Resolve(const internal::TypeKey& type_key);
        std::string_view GetOverride(const internal::TypeKey& interface, TypeId dependency);

        template<class Type>
        inline internal::TypeKey GetTypeKey(std::string_view name)
        {
            return internal::TypeKey{ GetTypeId<Type>(), static_cast<std::string>(name) };
        }

        std::map<internal::TypeKey, internal::TypeRecord> type_registry;
        std::map<internal::TypeKey, VariantPtr> instance_registry;

        template<class Interface, template <class P, class ... PArgs> class Wrapper>
        Wrapper<Interface> CastVariantPtrTo(VariantPtr& variant_ptr, const internal::TypeKey& type_key)
        {
            if (Pointer<void>* pointer = std::get_if<Pointer<void>>(&variant_ptr))
            {
                if constexpr(std::is_same_v<Wrapper<Interface>, UniquePtr<Interface>>)
                {
                    return UniquePtr<Interface>(static_cast<Interface*>(*pointer));
                }
                else if constexpr(std::is_same_v<Wrapper<Interface>, Pointer<Interface>>)
                {
                    return static_cast<Pointer<Interface>>(*pointer);
                }
                if constexpr(std::is_same_v<Wrapper<Interface>, SharedPtr<Interface>>)
                {
                    return SharedPtr<Interface>(static_cast<Interface*>(*pointer), type_registry.find(type_key)->second.deleter);
                }
            }
            else if (WeakPtr<void>* weak_ptr = std::get_if<WeakPtr<void>>(&variant_ptr))
            {
                if constexpr(std::is_same_v<Wrapper<Interface>, WeakPtr<Interface>>)
                {
                    return WeakPtr<Interface>(std::static_pointer_cast<Interface>(SharedPtr<void>(*weak_ptr)));
                }
                else if constexpr(std::is_same_v<Wrapper<Interface>, SharedPtr<Interface>>)
                {
                    return std::static_pointer_cast<Interface>(SharedPtr<void>(*weak_ptr));
                }
            }
            else if (SharedPtr<void>* shared_ptr = std::get_if<SharedPtr<void>>(&variant_ptr))
            {
                if constexpr(std::is_same_v<Wrapper<Interface>, SharedPtr<Interface>>)
                {
                    return std::static_pointer_cast<Interface>(*shared_ptr);
                }
                else if constexpr(std::is_same_v<Wrapper<Interface>, WeakPtr<Interface>>)
                {
                    return WeakPtr<Interface>{ std::static_pointer_cast<Interface>(*shared_ptr) };
                }
                else if constexpr(std::is_same_v<Wrapper<Interface>, Pointer<Interface>>)
                {
                    return static_cast<Pointer<Interface>>(shared_ptr->get());
                }
                else if constexpr(std::is_same_v<Wrapper<Interface>, Reference<Interface>>)
                {
                    return *static_cast<Pointer<Interface>>(shared_ptr->get());
                }
            }
            throw SDILException("Impossible situation");
        }
    };

    namespace internal
    {
        template<class Interface, class ReturnType, class ... Args>
        struct Factory<Interface, ReturnType(Args ...)>
        {
            static void* Create(Container* container, const TypeKey& type_key)
            {
                Instance* instance = SDILTypeTraits<Instance>::Create(
                        container->Resolve<typename WrapperInfo<Args>::Type, WrapperInfo<Args>::template Wrapper>(
                                container->GetOverride<typename WrapperInfo<Args>::Type>(type_key))...
                                );

                auto interface = static_cast<Interface*>(instance);
                return interface;
            }

            static void Delete(void* ptr)
            {
                auto interface = static_cast<Interface*>(ptr);
                auto instance = static_cast<Instance*>(interface);
                // TODO: Can be replaced with requires C++20
                if constexpr (SDILTypeTraits<Instance>::HasDeleteFunction)
                {
                    SDILTypeTraits<ReturnType>::Delete(instance);
                }
                else
                {
                    delete instance;
                }
            }

            private:
            using Instance = typename WrapperInfo<ReturnType>::Type;
        };
    }
}

#endif //SDIL_SDIL_HPP
