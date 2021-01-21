
#include <utils/startup_args.h>
#include <files/FileWatcher.h>
#include <audio/audio.h>
#include <graphics/texture.h>
#include <audio/WavLoader.h>
#include <audio/OggLoader.h>
#include <utils/code_editor/CodeEditor.h>
#include "../ecs/EntityInspector.h"
#include "../rendering/ImGuiStyle.h"
#include "dibidab.h"
#include "../lua_asset.h"

dibidab::EngineSettings dibidab::settings;

std::map<std::string, std::string> dibidab::startupArgs;

Session &dibidab::getCurrentSession()
{
    auto *s = tryGetCurrentSession();
    if (s == NULL)
        throw gu_err("There's no Session active at the moment");
    return *s;
}

delegate<void()> dibidab::onSessionChange;
Session *currSession = NULL;

Session *dibidab::tryGetCurrentSession()
{
    return currSession;
}

void dibidab::setCurrentSession(Session *s)
{
    delete currSession;
    currSession = s;

    if (s)
        luau::getLuaState()["saveGame"] = s->saveGame.luaTable;

    onSessionChange();
}


void showDeveloperOptionsMenuBar()
{
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("Game"))
    {
        ImGui::MenuItem(
                "Show developer options",
                KeyInput::getKeyName(dibidab::settings.keyInput.toggleDeveloperOptions),
                &dibidab::settings.showDeveloperOptions);

        ImGui::SetNextItemWidth(120);
        ImGui::SliderFloat("Master volume", &dibidab::settings.audio.masterVolume, 0.0f, 3.0f);

        if (ImGui::BeginMenu("Graphics"))
        {
            static bool vsync = gu::config.vsync;
            ImGui::MenuItem("VSync", "", &vsync);
            gu::setVSync(vsync);

            ImGui::MenuItem("Fullscreen", KeyInput::getKeyName(dibidab::settings.keyInput.toggleFullscreen), &gu::fullscreen);

            if (ImGui::BeginMenu("Edit shader"))
            {
                for (auto &[name, asset] : AssetManager::getLoadedAssetsForType<std::string>())
                {
                    if (!stringEndsWith(name, ".frag") && !stringEndsWith(name, ".vert"))
                        continue;
                    if (ImGui::MenuItem(name.c_str()))
                    {
                        auto &tab = CodeEditor::tabs.emplace_back();
                        tab.title = asset->fullPath;

                        tab.code = *((std::string *)asset->obj);
                        tab.languageDefinition = TextEditor::LanguageDefinition::C();
                        tab.save = [] (auto &tab) {

                            File::writeBinary(tab.title.c_str(), tab.code);

                            AssetManager::loadFile(tab.title, "assets/");
                        };
                        tab.revert = [] (auto &tab) {
                            tab.code = File::readString(tab.title.c_str());
                        };
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            ImGui::MenuItem(reinterpret_cast<const char *>(glGetString(GL_VERSION)), NULL, false, false);
            ImGui::MenuItem(reinterpret_cast<const char *>(glGetString(GL_RENDERER)), NULL, false, false);
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    EntityInspector::drawInspectingDropDown();

    ImGui::EndMainMenuBar();

    CodeEditor::drawGUI(
            ImGui::GetIO().Fonts->Fonts.back()  // default monospace font (added by setImGuiStyle())
    );
}

void addStandardAssetLoaders()
{
    addLuaAssetLoader<Texture>("texture_asset", ".png|.jpg|.jpeg|.tga|.bmp|.psd|.gif|.hdr", [&](auto path) {

        return new Texture(Texture::fromImageFile(path.c_str()));
    });
    addLuaAssetLoader<std::string>("shader_asset", ".frag|.vert", [](auto path) {

        return new std::string(File::readString(path.c_str()));
    });
    addLuaAssetLoader<json>("json_asset", ".json", [](auto path) {

        return new json(json::parse(File::readString(path.c_str())));
    });
    addLuaAssetLoader<au::Sound>("sound_asset", ".wav", [](auto path) {

        auto sound = new au::Sound;
        au::WavLoader(path.c_str(), *sound);
        return sound;
    });
    addLuaAssetLoader<au::Sound>("sound_asset", ".ogg", [](auto path) {

        auto sound = new au::Sound;
        au::OggLoader::load(path.c_str(), *sound);
        return sound;
    });
    addLuaAssetLoader<luau::Script>("script_asset", ".lua", [](auto path) {

        return new luau::Script(path);
    });
}

std::mutex assetToReloadMutex;
std::string assetToReload;
FileWatcher assetWatcher;

void dibidab::init(int argc, char **argv)
{
    startupArgsToMap(argc, argv, dibidab::startupArgs);


    gu::Config config;
    config.width = dibidab::settings.graphics.windowSize.x;
    config.height = dibidab::settings.graphics.windowSize.y;
    config.title = "dibidabidab";
    config.vsync = dibidab::settings.graphics.vsync;
    config.samples = 0;
    config.printOpenGLMessages = dibidab::settings.graphics.printOpenGLMessages;
    config.printOpenGLErrors = dibidab::settings.graphics.printOpenGLErrors;
    config.openGLMajorVersion = dibidab::settings.graphics.openGLMajorVersion;
    config.openGLMinorVersion = dibidab::settings.graphics.openGLMinorVersion;
    gu::fullscreen = dibidab::settings.graphics.fullscreen; // dont ask me why this is not in config
    if (!gu::init(config))
        throw gu_err("Error while initializing gu");
    au::init();
    setImGuiStyleAndConfig();

    std::cout << "Running game with\n - GL_VERSION: " << glGetString(GL_VERSION) << "\n";
    std::cout << " - GL_RENDERER: " << glGetString(GL_RENDERER) << "\n";

    std::vector<std::string> audioDevices;
    if (!au::getAvailableDevices(audioDevices, NULL))
        throw gu_err("could not get audio devices");

    std::cout << "Available audio devices:\n";
    for (auto &dev : audioDevices)
        std::cout << " - " << dev << std::endl;



    #ifdef linux
    assetWatcher.addDirectoryToWatch("assets", true);

    assetWatcher.onChange = [&] (auto path)
    {
        assetToReloadMutex.lock();
        assetToReload = path;
        assetToReloadMutex.unlock();
    };
    assetWatcher.startWatchingAsync();
    #endif

    addStandardAssetLoaders();
    AssetManager::load("assets");

    // save window size in settings:
    static auto _ = gu::onResize += [] {
        if (!gu::fullscreen) // dont save fullscreen-resolution
            dibidab::settings.graphics.windowSize = ivec2(
                    gu::width, gu::height
            );
    };

    gu::beforeRender = [&](double deltaTime) {
        if (currSession)
            currSession->update(KeyInput::pressed(GLFW_KEY_KP_SUBTRACT) ? deltaTime * .03 : deltaTime);

        if (KeyInput::justPressed(dibidab::settings.keyInput.reloadAssets))
            AssetManager::load("assets", true);

        {
            assetToReloadMutex.lock();
            if (!assetToReload.empty())
                AssetManager::loadFile(assetToReload, "assets/", true);
            assetToReload.clear();
            assetToReloadMutex.unlock();
        }

        {
            dibidab::settings.graphics.vsync = gu::config.vsync;
            dibidab::settings.graphics.fullscreen = gu::fullscreen;
        }

        if (KeyInput::justPressed(dibidab::settings.keyInput.toggleDeveloperOptions))
            dibidab::settings.showDeveloperOptions ^= 1;

        if (KeyInput::justPressed(dibidab::settings.keyInput.toggleFullscreen))
            gu::fullscreen = !gu::fullscreen;

        if (dibidab::settings.showDeveloperOptions)
            showDeveloperOptionsMenuBar();
        gu::profiler::showGUI = dibidab::settings.showDeveloperOptions;
    };

}

void dibidab::run()
{
    gu::run();
    dibidab::setCurrentSession(NULL);
    au::terminate();
}
