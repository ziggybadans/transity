#include "UI.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "../Logger.h"
#include "../input/InteractionMode.h"
#include <cstdlib>

UI::UI(sf::RenderWindow& window, WorldGenerationSystem* worldGenSystem)
    : m_window(window), 
    m_currentInteractionMode(InteractionMode::SELECT), 
    _worldGenerationSystem(worldGenSystem) {
    LOG_INFO("UI", "UI instance created.");

    if (_worldGenerationSystem) {
        _worldGenSeed = _worldGenerationSystem->getSeed();
        _worldGenFrequency = _worldGenerationSystem->getFrequency();
        _worldGenNoiseType = static_cast<int>(_worldGenerationSystem->getNoiseType());
        _worldGenFractalType = static_cast<int>(_worldGenerationSystem->getFractalType());
        _worldGenOctaves = _worldGenerationSystem->getOctaves();
        _worldGenLacunarity = _worldGenerationSystem->getLacunarity();
        _worldGenGain = _worldGenerationSystem->getGain();
    }
}

UI::~UI() {
    LOG_INFO("UI", "UI instance destroyed.");
}

void UI::initialize() {
    LOG_INFO("UI", "Initializing ImGui.");
    ImGui::CreateContext();
    if (!ImGui::SFML::Init(m_window)) {
        LOG_FATAL("UI", "Failed to initialize ImGui-SFML");
        exit(EXIT_FAILURE);
    }
    ImGui::StyleColorsDark();
    LOG_INFO("UI", "ImGui initialized successfully.");
}

void UI::processEvent(const sf::Event& sfEvent) {
    ImGui::SFML::ProcessEvent(m_window, sfEvent);
}

void UI::update(sf::Time deltaTime, size_t numberOfStationsInActiveLine) {
    ImGui::SFML::Update(m_window, deltaTime);

    const float windowPadding = 10.0f;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    ImVec2 debugWindowPos = ImVec2(windowPadding, windowPadding);
    ImGui::SetNextWindowPos(debugWindowPos, ImGuiCond_Always);
        ImGui::Begin("Profiling", nullptr, size_flags);
        ImGui::Text("FPS: %.1f", 1.f / deltaTime.asSeconds());
    ImGui::End();

    float worldGenSettingsWidth = 300.0f;
    ImVec2 worldGenSettingsPos = ImVec2(displaySize.x - worldGenSettingsWidth - windowPadding, windowPadding);
    ImGui::SetNextWindowPos(worldGenSettingsPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(worldGenSettingsWidth, 0.0f), ImGuiCond_Always);
    ImGui::Begin("World Generation Settings", nullptr, window_flags);
        bool valueChanged = false;

        if (ImGui::InputInt("Seed", &_worldGenSeed)) {
            LOG_INFO("UI", "World generation seed changed to: %d", _worldGenSeed);
            if (_worldGenerationSystem) {
                _worldGenerationSystem->setSeed(_worldGenSeed);
                valueChanged = true;
            }
        };
        if (ImGui::InputFloat("Frequency", &_worldGenFrequency, 0.001f, 0.1f, "%.4f")) {
            LOG_INFO("UI", "World generation frequency changed to: %.4f", _worldGenFrequency);
            if (_worldGenerationSystem) {
                _worldGenerationSystem->setFrequency(_worldGenFrequency);
                valueChanged = true;
            }
        };
        const char* noiseTypes[] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
        if (ImGui::Combo("Noise Type", &_worldGenNoiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes))) {
            LOG_INFO("UI", "World generation noise type changed to: %s", noiseTypes[_worldGenNoiseType]);
            if (_worldGenerationSystem) {
                _worldGenerationSystem->setNoiseType(static_cast<FastNoiseLite::NoiseType>(_worldGenNoiseType));
                valueChanged = true;
            }
        };

        const char* fractalTypes[] = { "None", "FBm", "Ridged", "PingPong", "DomainWarpProgressive", "DomainWarpIndependent" };
        if (ImGui::Combo("Fractal Type", &_worldGenFractalType, fractalTypes, IM_ARRAYSIZE(fractalTypes))) {
            LOG_INFO("UI", "World generation fractal type changed to: %s", fractalTypes[_worldGenFractalType]);
            if (_worldGenerationSystem) {
                _worldGenerationSystem->setFractalType(static_cast<FastNoiseLite::FractalType>(_worldGenFractalType));
                valueChanged = true;
            }
        };

        if (ImGui::SliderInt("Octaves", &_worldGenOctaves, 1, 10)) {
            LOG_INFO("UI", "World generation octaves changed to: %d", _worldGenOctaves);
            if (_worldGenerationSystem) {
                _worldGenerationSystem->setOctaves(_worldGenOctaves);
                valueChanged = true;
            }
        };
        if (ImGui::SliderFloat("Lacunarity", &_worldGenLacunarity, 0.1f, 4.0f)) {
            LOG_INFO("UI", "World generation lacunarity changed to: %.2f", _worldGenLacunarity);
            if (_worldGenerationSystem) {
                _worldGenerationSystem->setLacunarity(_worldGenLacunarity);
                valueChanged = true;
            }
        };
        if (ImGui::SliderFloat("Gain", &_worldGenGain, 0.1f, 1.0f)) {
            LOG_INFO("UI", "World generation gain changed to: %.2f", _worldGenGain);
            if (_worldGenerationSystem) {
                _worldGenerationSystem->setGain(_worldGenGain);
                valueChanged = true;
            }
        };

        if (valueChanged && _autoRegenerate && _worldGenerationSystem) {
            LOG_INFO("UI", "World generation settings changed, auto-regenerating world.");
            _worldGenerationSystem->configureNoise();
        }

        ImGui::Separator();

        if (ImGui::Button("Regenerate World")) {
            LOG_INFO("UI", "Regenerate World button clicked.");
            if (_worldGenerationSystem) {
                _worldGenerationSystem->configureNoise();
            }
        }

        ImGui::Checkbox("Visualize Noise", &_visualizeNoise);
        ImGui::Checkbox("Auto Regenerate", &_autoRegenerate);

    ImGui::End();

    float interactionModesWidth = 400.0f;
    float interactionModesHeight = 100.0f;
    ImVec2 interactionModesPos = ImVec2((displaySize.x - interactionModesWidth) * 0.5f, 
        displaySize.y - interactionModesHeight - windowPadding);
    ImGui::SetNextWindowPos(interactionModesPos, ImGuiCond_Always);
    ImGui::Begin("Interaction Modes", nullptr, size_flags);
        int currentMode = static_cast<int>(m_currentInteractionMode);

        if (ImGui::RadioButton("None", &currentMode, static_cast<int>(InteractionMode::SELECT))) {
            m_currentInteractionMode = InteractionMode::SELECT;
            LOG_INFO("UI", "Interaction mode changed to: None");
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Station Placement", &currentMode, static_cast<int>(InteractionMode::CREATE_STATION))) {
            m_currentInteractionMode = InteractionMode::CREATE_STATION;
            LOG_INFO("UI", "Interaction mode changed to: StationPlacement");
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Line Creation", &currentMode, static_cast<int>(InteractionMode::CREATE_LINE))) {
            m_currentInteractionMode = InteractionMode::CREATE_LINE;
            LOG_INFO("UI", "Interaction mode changed to: LineCreation");
        }
        if (m_currentInteractionMode == InteractionMode::CREATE_LINE && numberOfStationsInActiveLine >= 2) {
            if (ImGui::Button("Finalize Line")) {
                FinalizeLineEvent finalizeLineEvent;
                m_uiEvents.emplace_back(finalizeLineEvent);
                LOG_INFO("UI", "Finalize Line button clicked.");
            }
        }
    ImGui::End();
}

void UI::renderFrame() {
    ImGui::SFML::Render(m_window);
}

void UI::cleanupResources() {
    LOG_INFO("UI", "Shutting down ImGui.");
    ImGui::SFML::Shutdown();
    LOG_INFO("UI", "ImGui shutdown complete.");
}

InteractionMode UI::getInteractionMode() const {
    return m_currentInteractionMode;
}

const std::vector<FinalizeLineEvent>& UI::getUiEvents() const {
    return m_uiEvents;
}

void UI::clearUiEvents() {
    m_uiEvents.clear();
    LOG_INFO("UI", "UI events cleared.");
}
