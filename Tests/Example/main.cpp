
#include "SDIL.hpp"
#include <iostream>
#include <utility>

struct Audio
{
    virtual ~Audio() = default;

    virtual void PlayAudio(std::string_view file) = 0;
};

struct DirectXAudio : public Audio
{
    void PlayAudio(std::string_view file) override
    {
        std::cout << "DirectX play audio: " << file << std::endl;
    }
};

struct ALSAAudio : public Audio
{
    void PlayAudio(std::string_view file) override
    {
        std::cout << "Linux play audio: " << file << std::endl;
    }
};

struct Button
{
    explicit Button(std::shared_ptr<Audio> audio): audio(std::move(audio))
    {

    }

    void Click()
    {
        audio->PlayAudio("Click!");
    }

private:
    std::shared_ptr<Audio> audio;
};

template<>
struct sdil::SDILTypeTraits<DirectXAudio> : SDILTypeTraitsBase
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;

    static DirectXAudio* Create()
    {
        return new DirectXAudio();
    }
};

template<>
struct sdil::SDILTypeTraits<ALSAAudio> : SDILTypeTraitsBase
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;

    static ALSAAudio* Create()
    {
        return new ALSAAudio();
    }

};

template<>
struct sdil::SDILTypeTraits<Button> : SDILTypeTraitsBase //, Constructor<Button, std::shared_ptr<Audio>>
{
    static constexpr sdil::LifeTimeScope LifeTime = LifeTimeScope::Singleton;

   static Button* Create(std::shared_ptr<Audio> audio)
   {
       return new Button(audio);
   }
};

int main(int argc, char* args[])
{
    sdil::Container container;
    container.Register<DirectXAudio, Audio>();
    container.Register<ALSAAudio, Audio>("Linux");
    container.Register<Button>();
    container.Register<Button>("AnotherBtn", { {sdil::GetTypeId<Audio>(), "Linux"} });

    std::shared_ptr<Button> btn = container.Resolve<Button>();
    Button& btn_ref = container.Resolve<Button, sdil::Reference>();
    btn->Click();
    btn_ref.Click();

    auto linux_btn = container.Resolve<Button>("AnotherBtn");
    linux_btn->Click();
}

