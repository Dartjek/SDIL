#include "SDIL.hpp"

namespace sdil
{
#define TO_STRING(symbol) #symbol

    Container::VariantPtr Container::Resolve(const internal::TypeKey &type_key) {
        auto type_record_it = type_registry.find(type_key);
        if (type_record_it == std::cend(type_registry))
        {
            throw SDILException("Type is not registered. Use " TO_STRING(Container::Register) " to register the type");
        }

        internal::TypeRecord& type_record = type_record_it->second;
        if(type_record.lifetime != LifeTimeScope::NotControlled)
        {
            auto variant_it = instance_registry.find(type_key);
            if (variant_it != std::cend(instance_registry))
            {
                VariantPtr& variant_ptr = variant_it->second;

                if (WeakPtr<void>* weak_ptr = std::get_if<WeakPtr<void>>(&variant_ptr))
                {
                    SharedPtr<void> instance = weak_ptr->lock();
                    if (instance != nullptr)
                    {
                        return instance;
                    }
                    else
                    {
                        instance.reset(type_record.create(this, type_key), type_record.deleter);
                        switch (type_record.lifetime) {
                            case LifeTimeScope::Singleton:
                                instance_registry.insert_or_assign(variant_it, type_key, VariantPtr{ instance });
                                break;
                            case LifeTimeScope::ReferenceCounting:
                                *weak_ptr = instance;
                                break;
                            case LifeTimeScope::NotControlled:
                            default:
                                throw SDILException("Unexpected lifetime scope");
                        }
                        return instance;
                    }
                }
                else if (std::holds_alternative<SharedPtr<void>>(variant_ptr))
                {
                    return variant_ptr;
                }
                else
                {
                    throw SDILException("Unexpected content of variant");
                }
            }
            else
            {
                SharedPtr<void> instance {type_record.create(this, type_key) , type_record.deleter} ;
                VariantPtr variant_ptr;
                switch (type_record.lifetime) {
                    case LifeTimeScope::Singleton:
                        variant_ptr = instance;
                        break;
                    case LifeTimeScope::ReferenceCounting:
                        variant_ptr = WeakPtr<void>{ instance };
                        break;
                    case LifeTimeScope::NotControlled:
                    default:
                        throw SDILException("Impossible situation. 2ed if-clause above must not allow achieve this code");
                }

                instance_registry.emplace(type_key, variant_ptr);
                return instance;
            }
        }
        else
        {
            return type_record.create(this, type_key);
        }
    }

    std::string_view Container::GetOverride(const internal::TypeKey& interface, TypeId dependency) {
        auto type_record_it = type_registry.find(interface);
        if (type_record_it == std::cend(type_registry)) return "";

        internal::TypeRecord& type_record = type_record_it->second;
        auto override_it = type_record.overrides.find(dependency);
        if (override_it == std::cend(type_record.overrides)) return "";
        else return override_it->second;
    }

    void Container::CheckWrapperType(internal::WrapperType wrapper_type, const internal::TypeKey &interface)
    {
        using internal::WrapperType;
        auto type_record_it = type_registry.find(interface);
        if (type_record_it == std::cend(type_registry))
        {
            throw SDILException("Type is not registered. Use " TO_STRING(Container::Register) " to register the type");
        }

        internal::TypeRecord& type_record = type_record_it->second;
        switch (type_record.lifetime) {
            case LifeTimeScope::Singleton:
                if (wrapper_type == WrapperType::Unique)
                {
                    throw SDILException("For singleton lifetime scope unique ptr can not be requested");
                }
                break;
            case LifeTimeScope::ReferenceCounting:
                if (wrapper_type != WrapperType::Shared && wrapper_type != WrapperType::Weak)
                {
                    throw SDILException("For reference counting lifetime scope should be requested shared or weak");
                }
                break;
            case LifeTimeScope::NotControlled:
                if (wrapper_type != WrapperType::Unique && wrapper_type != WrapperType::Raw && wrapper_type != WrapperType::Shared)
                {
                    throw SDILException("For NotControlled lifetime scope should be requested UniquePtr, SharedPtr, or Raw");
                }
                break;
        }
    }
}
