// ============================================================================
// NEON REQUIEM — Dialog System (Implementation)
// ============================================================================
// Реализация диалоговой системы. Загружает JSON-диалоги с SD, управляет
// typewriter-эффектом, выбором реплик и ветвлением по маршрутам.
// ============================================================================

#include "dialog_system.h"
#include "../../core/input/input_manager.h"
#include "../../core/display/display_manager.h"
#include "../../core/storage/sd_manager.h"

// ============================================================================
// КОНСТРУКТОР
// ============================================================================

DialogScene::DialogScene()
    : Scene(SceneType::DIALOG)
    , m_nodes(nullptr)
    , m_nodeCount(0)
    , m_currentNodeId(0)
    , m_currentSpeaker(Speaker::NARRATOR)
    , m_textLength(0)
    , m_textTargetLength(0)
    , m_typewriterTimer(0)
    , m_selectedChoice(-1)
    , m_dialogLoaded(false)
    , m_waitingForInput(false)
    , m_onEndCallback(nullptr)
    , m_initialized(false)
{
    m_displayedText[0] = '\0';
    setFlag(SCENE_TRANSPARENT);  // Диалог поверх overworld
    setFlag(SCENE_PAUSABLE);
}

// ============================================================================
// ЗАГРУЗКА ДИАЛОГА
// ============================================================================

bool DialogScene::loadDialog(uint16_t dialogId) {
    // Формируем путь к файлу: /dialogs/dialog_XXXX.json
    char path[64];
    snprintf(path, sizeof(path), "%sdialog_%04d.json", SD_PATH_DIALOGS, dialogId);

    LOG_INFO_F("Dialog: loading %s", path);

    // Загружаем JSON
    JsonDocument doc;
    if (!SDManager::getInstance().readJSON(path, doc)) {
        LOG_ERROR_F("Dialog: failed to load %s", path);
        return false;
    }

    // Парсим узлы
    if (!parseNodes(doc)) {
        LOG_ERROR("Dialog: failed to parse nodes");
        return false;
    }

    m_dialogLoaded = true;
    LOG_INFO_F("Dialog: loaded %d nodes from %s", m_nodeCount, path);
    return true;
}

// ============================================================================
// ПАРСИНГ УЗЛОВ
// ============================================================================

bool DialogScene::parseNodes(const JsonDocument& doc) {
    JsonArrayConst nodesArray = doc["nodes"].as<JsonArrayConst>();
    m_nodeCount = nodesArray.size();

    if (m_nodeCount == 0) {
        LOG_ERROR("Dialog: no nodes in dialog file");
        return false;
    }

    // Выделяем память под узлы
    m_nodes = (DialogNode*)ps_malloc(m_nodeCount * sizeof(DialogNode));
    if (!m_nodes) {
        LOG_ERROR("Dialog: PSRAM allocation failed for nodes");
        return false;
    }

    // Парсим каждый узел
    uint16_t idx = 0;
    for (JsonObjectConst nodeObj : nodesArray) {
        DialogNode& node = m_nodes[idx];

        // Тип узла
        node.type = static_cast<DialogNodeType>(nodeObj["type"].as<uint8_t>());

        // Говорящий
        node.speaker = static_cast<Speaker>(nodeObj["speaker"].as<uint8_t>());

        // Текст
        const char* text = nodeObj["text"].as<const char*>();
        if (text) {
            strncpy(node.text, text, sizeof(node.text) - 1);
        } else {
            node.text[0] = '\0';
        }

        // ID следующего узла
        node.nextNodeId = nodeObj["next"].as<uint16_t>();

        // Варианты выбора
        if (node.type == DialogNodeType::CHOICE) {
            JsonArrayConst choices = nodeObj["choices"].as<JsonArrayConst>();
            node.choiceCount = min((uint8_t)choices.size(), (uint8_t)DIALOG_MAX_CHOICES);

            uint8_t c = 0;
            for (JsonObjectConst choiceObj : choices) {
                if (c >= node.choiceCount) break;

                const char* choiceText = choiceObj["text"];
                strncpy(node.choices[c].text,
                        choiceText ? choiceText : "",
                        sizeof(node.choices[c].text) - 1);
                node.choices[c].nextNodeId = choiceObj["next"].as<uint16_t>();
                node.choices[c].routeRequirement = (int8_t)(choiceObj["route"] | -1);
                node.choices[c].questRequirement = (int8_t)(choiceObj["quest"] | -1);
                c++;
            }
        } else {
            node.choiceCount = 0;
        }

        // Параметр действия
        node.actionParam = nodeObj["action"] | 0;

        idx++;
    }

    return true;
}

// ============================================================================
// НАЧАЛО ДИАЛОГА
// ============================================================================

void DialogScene::start(uint16_t startNodeId) {
    if (!m_dialogLoaded) return;

    m_currentNodeId = startNodeId;
    m_waitingForInput = false;
    m_selectedChoice = -1;

    advanceToNode(startNodeId);
}

// ============================================================================
// ПЕРЕХОД К УЗЛУ
// ============================================================================

void DialogScene::advanceToNode(uint16_t nodeId) {
    if (nodeId >= m_nodeCount) {
        // Конец диалога
        m_active = false;
        if (m_onEndCallback) {
            m_onEndCallback();
        }
        return;
    }

    DialogNode& node = m_nodes[nodeId];
    m_currentNodeId = nodeId;
    m_currentSpeaker = node.speaker;

    switch (node.type) {
        case DialogNodeType::SPEECH:
            // Начинаем typewriter
            strncpy(m_displayedText, node.text, sizeof(m_displayedText) - 1);
            m_textTargetLength = strlen(node.text);
            m_textLength = 0;
            m_typewriterTimer = 0;
            m_waitingForInput = false;
            break;

        case DialogNodeType::CHOICE:
            // Показываем текст и варианты
            strncpy(m_displayedText, node.text, sizeof(m_displayedText) - 1);
            m_textTargetLength = strlen(node.text);
            m_textLength = m_textTargetLength;  // Сразу весь текст
            m_selectedChoice = 0;
            m_waitingForInput = false;
            break;

        case DialogNodeType::CONDITION:
            // Проверяем условия и переходим
            // TODO: проверка маршрута/квеста
            advanceToNode(node.nextNodeId);
            break;

        case DialogNodeType::ACTION:
            // Выполняем действие (бой, катсцена)
            LOG_INFO_F("Dialog: action triggered, param=%d", node.actionParam);
            // TODO: запуск боя/катсцены
            advanceToNode(node.nextNodeId);
            break;

        case DialogNodeType::END:
            m_active = false;
            if (m_onEndCallback) {
                m_onEndCallback();
            }
            break;
    }
}

// ============================================================================
// ОБНОВЛЕНИЕ
// ============================================================================

void DialogScene::update() {
    if (!m_active || !m_dialogLoaded) return;

    InputManager& input = InputManager::getInstance();
    DialogNode& node = m_nodes[m_currentNodeId];

    // Typewriter-эффект
    if (m_textLength < m_textTargetLength) {
        updateTypewriter();
        return;  // Не принимаем ввод пока печатается текст
    }

    // Текст напечатан — ждём ввод
    switch (node.type) {
        case DialogNodeType::SPEECH:
            // A = продолжить, B = пропустить (если не монолог)
            if (input.justPressed(Button::A)) {
                advanceToNode(node.nextNodeId);
            }
            if (input.justPressed(Button::B) &&
                node.speaker != Speaker::MONOLOGUE) {
                // Пропускаем до конца диалога
                // TODO: найти конец ветки
                advanceToNode(node.nextNodeId);
            }
            break;

        case DialogNodeType::CHOICE:
            // Навигация по вариантам
            if (input.justPressed(Button::UP) && m_selectedChoice > 0) {
                m_selectedChoice--;
            }
            if (input.justPressed(Button::DOWN) &&
                m_selectedChoice < node.choiceCount - 1) {
                m_selectedChoice++;
            }
            if (input.justPressed(Button::A)) {
                selectChoice(m_selectedChoice);
            }
            break;

        default:
            break;
    }
}

// ============================================================================
// TYPEWRITER
// ============================================================================

void DialogScene::updateTypewriter() {
    m_typewriterTimer += FRAME_TIME_MS;

    if (m_typewriterTimer >= TYPEWRITER_DELAY_MS) {
        m_typewriterTimer = 0;
        if (m_textLength < m_textTargetLength) {
            m_textLength++;
        }
    }
}

// ============================================================================
// ВЫБОР
// ============================================================================

void DialogScene::selectChoice(uint8_t index) {
    DialogNode& node = m_nodes[m_currentNodeId];
    if (index >= node.choiceCount) return;

    DialogChoice& choice = node.choices[index];

    // Проверяем требования маршрута
    if (choice.routeRequirement >= 0) {
        // TODO: проверка текущего маршрута
    }

    // Проверяем требования квеста
    if (choice.questRequirement >= 0) {
        // TODO: проверка квеста
    }

    // Переходим к следующему узлу
    advanceToNode(choice.nextNodeId);
}

// ============================================================================
// ОТРИСОВКА
// ============================================================================

void DialogScene::render() {
    if (!m_active || !m_dialogLoaded) return;

    DisplayManager& display = DisplayManager::getInstance();
    DialogNode& node = m_nodes[m_currentNodeId];

    // Фон диалогового окна (нижняя треть экрана)
    int boxY = DISPLAY_HEIGHT - 80;
    int boxH = 80;

    display.fillRect(0, boxY, DISPLAY_WIDTH, boxH, Palette::DIALOG_BG);
    display.drawRect(0, boxY, DISPLAY_WIDTH, boxH, Palette::DIALOG_BORDER);

    // Имя говорящего
    const char* speakerName = getSpeakerName(m_currentSpeaker);
    uint16_t speakerColor = getSpeakerColor(m_currentSpeaker);

    // TODO: отрисовка текста (после создания шрифтовой системы)
    // Пока placeholder — рисуем цветную полоску под имя

    display.fillRect(4, boxY + 4, strlen(speakerName) * 8, 10, speakerColor);

    // Текст (typewriter)
    // TODO: отрисовка текста с typewriter-эффектом

    // Варианты выбора
    if (node.type == DialogNodeType::CHOICE) {
        for (int i = 0; i < node.choiceCount; i++) {
            int choiceY = boxY + 20 + i * 14;
            uint16_t choiceColor = (i == m_selectedChoice) ?
                                    Palette::MENU_SELECT : Palette::MENU_NORMAL;

            // TODO: отрисовка текста выбора
            display.fillRect(8, choiceY, strlen(node.choices[i].text) * 6, 8, choiceColor);
        }
    }

    // Индикатор продолжения (мигающий треугольник)
    if (m_textLength >= m_textTargetLength &&
        node.type == DialogNodeType::SPEECH) {
        static uint32_t blinkTimer = 0;
        blinkTimer += FRAME_TIME_MS;
        if ((blinkTimer / 500) % 2 == 0) {
            // Рисуем треугольник в правом нижнем углу
            display.fillTriangle(
                DISPLAY_WIDTH - 16, boxY + boxH - 8,
                DISPLAY_WIDTH - 8,  boxY + boxH - 4,
                DISPLAY_WIDTH - 16, boxY + boxH,
                Palette::TEXT_WHITE
            );
        }
    }
}

// ============================================================================
// ВХОД / ВЫХОД
// ============================================================================

void DialogScene::onEnter() {
    Scene::onEnter();
    LOG_INFO("Dialog: scene entered");
}

void DialogScene::onExit() {
    Scene::onExit();

    // Освобождаем память
    if (m_nodes) {
        free(m_nodes);
        m_nodes = nullptr;
    }
    m_nodeCount = 0;
    m_dialogLoaded = false;

    LOG_INFO("Dialog: scene exited");
}

// ============================================================================
// ЦВЕТА ГОВОРЯЩИХ
// ============================================================================

uint16_t DialogScene::getSpeakerColor(Speaker speaker) const {
    switch (speaker) {
        case Speaker::KAI:       return Palette::KAI_CORE;
        case Speaker::ECHO:      return Palette::ECHO_GHOST;
        case Speaker::RAZOR:     return Palette::AMBER;
        case Speaker::LYRA:      return Palette::LYRA_COAT;
        case Speaker::ZERO:      return Palette::ZERO_SUIT;
        case Speaker::CORTANA:   return Palette::CORTANA_BODY;
        case Speaker::COLLECTOR: return Palette::COLLECTOR_ROBE;
        case Speaker::SIREN:     return Palette::SIREN_DRESS;
        case Speaker::KANE:      return Palette::KANE_SUIT;
        case Speaker::MONOLOGUE: return Palette::CYAN;
        case Speaker::SYSTEM:    return Palette::TEXT_GRAY;
        default:                 return Palette::TEXT_WHITE;
    }
}

const char* DialogScene::getSpeakerName(Speaker speaker) const {
    switch (speaker) {
        case Speaker::NARRATOR:  return "NARRATOR";
        case Speaker::KAI:       return "KAI";
        case Speaker::ECHO:      return "ECHO";
        case Speaker::RAZOR:     return "RAZOR";
        case Speaker::LYRA:      return "LYRA";
        case Speaker::ZERO:      return "ZERO";
        case Speaker::CORTANA:   return "CORTANA-9";
        case Speaker::COLLECTOR: return "THE COLLECTOR";
        case Speaker::SIREN:     return "SIREN";
        case Speaker::KANE:      return "DIRECTOR KANE";
        case Speaker::SYSTEM:    return "SYSTEM";
        case Speaker::MONOLOGUE: return ">>";
        default:                 return "???";
    }
}