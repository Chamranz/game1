// ============================================================================
// NEON REQUIEM — Cutscene System (Implementation)
// ============================================================================

#include "cutscene_system.h"
#include "../../core/input/input_manager.h"
#include "../../core/display/display_manager.h"
#include "../../core/audio/audio_manager.h"
#include "../../core/storage/sd_manager.h"

CutsceneScene::CutsceneScene()
    : Scene(SceneType::CUTSCENE)
    , m_currentFrame(0), m_frameTimer(0), m_playing(false), m_skipped(false)
    , m_onEnd(nullptr)
{
    m_data.frameCount = 0;
    m_data.musicId = 0;
    m_data.skippable = true;
    m_data.frames = nullptr;
    setFlag(SCENE_SKIPPABLE);
}

CutsceneScene::~CutsceneScene() {
    if (m_data.frames) {
        free(m_data.frames);
    }
}

bool CutsceneScene::loadCutscene(uint16_t cutsceneId) {
    char path[64];
    snprintf(path, sizeof(path), "%scutscene_%04d.json", SD_PATH_DIALOGS, cutsceneId);

    JsonDocument doc;
    if (!SDManager::getInstance().readJSON(path, doc)) return false;

    m_data.frameCount = doc["frames"].as<uint16_t>();
    m_data.musicId = doc["music"] | 0;
    m_data.skippable = doc["skippable"] | true;

    m_data.frames = (CutsceneFrame*)ps_malloc(m_data.frameCount * sizeof(CutsceneFrame));
    if (!m_data.frames) return false;

    JsonArrayConst frames = doc["frames_data"].as<JsonArrayConst>();
    int i = 0;
    for (JsonObjectConst f : frames) {
        m_data.frames[i].duration = f["duration"].as<uint16_t>();
        m_data.frames[i].bgColor = HEX565(f["bg"].as<uint32_t>());
        m_data.frames[i].effectType = f["effect"] | 0;
        m_data.frames[i].effectIntensity = f["intensity"] | 0;
        i++;
    }

    return true;
}

void CutsceneScene::play() {
    m_currentFrame = 0;
    m_frameTimer = millis();
    m_playing = true;
    m_skipped = false;

    if (m_data.musicId > 0) {
        char path[64];
        snprintf(path, sizeof(path), "%smusic_%04d.wav", SD_PATH_MUSIC, m_data.musicId);
        int ch = AudioManager::getInstance().loadWAV(path, AudioType::MUSIC);
        if (ch >= 0) AudioManager::getInstance().play(ch, true);
    }
}

void CutsceneScene::update() {
    if (!m_playing) return;

    if (m_data.skippable && InputManager::getInstance().justPressed(Button::A)) {
        m_playing = false;
        m_skipped = true;
        AudioManager::getInstance().stopAll();
        if (m_onEnd) m_onEnd();
        return;
    }

    if (millis() - m_frameTimer >= m_data.frames[m_currentFrame].duration) {
        m_currentFrame++;
        m_frameTimer = millis();

        if (m_currentFrame >= m_data.frameCount) {
            m_playing = false;
            AudioManager::getInstance().stopAll();
            if (m_onEnd) m_onEnd();
        }
    }
}

void CutsceneScene::render() {
    if (!m_playing || m_currentFrame >= m_data.frameCount) return;

    DisplayManager& display = DisplayManager::getInstance();
    CutsceneFrame& frame = m_data.frames[m_currentFrame];

    display.clearBuffer(frame.bgColor);

    // Эффекты
    if (frame.effectType == 1) {  // Дождь
        for (int i = 0; i < 40; i++) {
            int rx = rand() % DISPLAY_WIDTH;
            int ry = rand() % DISPLAY_HEIGHT;
            display.drawPixel(rx, ry, Palette::RAIN);
        }
    }
}

void CutsceneScene::onEnter() { Scene::onEnter(); }
void CutsceneScene::onExit() {
    Scene::onExit();
    AudioManager::getInstance().stopAll();
    if (m_data.frames) { free(m_data.frames); m_data.frames = nullptr; }
}