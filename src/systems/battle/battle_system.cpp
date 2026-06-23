// ============================================================================
// NEON REQUIEM — Battle System (Implementation)
// ============================================================================
// Undertale-style боевая система. Управляет фазами боя, действиями игрока,
// bullet-hell атаками врага и COUNTER-HACK механикой.
// ============================================================================

#include "battle_system.h"
#include "../../core/input/input_manager.h"
#include "../../core/display/display_manager.h"

// ============================================================================
// КОНСТРУКТОР
// ============================================================================

BattleScene::BattleScene()
    : Scene(SceneType::BATTLE)
    , m_battleType(BattleType::NORMAL)
    , m_phase(BattlePhase::INTRO)
    , m_selectedAction(BattleAction::HACK)
    , m_lastAction(BattleAction::NONE)
    , m_turnCount(0)
    , m_playerBattleHP(PLAYER_MAX_HP)
    , m_playerMaxBattleHP(PLAYER_MAX_HP)
    , m_scanned(false)
    , m_scanLevel(0)
    , m_counterHackAvailable(false)
    , m_counterHackTimer(0)
    , m_animTimer(0)
    , m_animFrame(0)
    , m_onVictory(nullptr)
    , m_onDefeat(nullptr)
    , m_initialized(false)
{
    // Инициализация врага
    m_enemy.name[0] = '\0';
    m_enemy.maxHP = 0;
    m_enemy.hp = 0;
    m_enemy.attack = 0;
    m_enemy.defense = 0;
    m_enemy.color = Palette::RED;
    m_enemy.phaseCount = 1;
    m_enemy.currentPhase = 0;
    m_enemy.attackType = AttackType::REACTION;
    m_enemy.dialogId = 0;
}

// ============================================================================
// НАЧАЛО БОЯ
// ============================================================================

void BattleScene::startBattle(const EnemyData& enemyData, BattleType battleType) {
    m_enemy = enemyData;
    m_battleType = battleType;
    m_phase = BattlePhase::INTRO;
    m_selectedAction = BattleAction::HACK;
    m_turnCount = 0;
    m_playerBattleHP = PLAYER_MAX_HP;
    m_scanned = false;
    m_scanLevel = 0;
    m_counterHackAvailable = false;
    m_animTimer = 0;
    m_animFrame = 0;

    m_active = true;
    LOG_INFO_F("Battle: started vs %s (HP: %d, Type: %d)",
               m_enemy.name, m_enemy.hp, static_cast<int>(battleType));
}

// ============================================================================
// ОБНОВЛЕНИЕ
// ============================================================================

void BattleScene::update() {
    if (!m_active) return;

    switch (m_phase) {
        case BattlePhase::INTRO:
            updateIntro();
            break;
        case BattlePhase::PLAYER:
            updatePlayerTurn();
            break;
        case BattlePhase::ENEMY:
            updateEnemyTurn();
            break;
        case BattlePhase::RESULT:
            updateResult();
            break;
        case BattlePhase::COUNTER:
            updateCounterHack();
            break;
        case BattlePhase::VICTORY:
        case BattlePhase::DEFEAT:
            // Ждём нажатия A для выхода
            if (InputManager::getInstance().justPressed(Button::A)) {
                m_active = false;
                if (m_phase == BattlePhase::VICTORY && m_onVictory) {
                    m_onVictory();
                } else if (m_phase == BattlePhase::DEFEAT && m_onDefeat) {
                    m_onDefeat();
                }
            }
            break;
    }

    // Анимация
    m_animTimer++;
    if (m_animTimer >= 8) {
        m_animTimer = 0;
        m_animFrame = (m_animFrame + 1) % 4;
    }
}

// ============================================================================
// ФАЗА INTRO
// ============================================================================

void BattleScene::updateIntro() {
    // Показываем имя врага на 2 секунды
    static uint32_t introTimer = 0;
    if (introTimer == 0) {
        introTimer = millis();
        LOG_INFO_F("Battle: % appeared!", m_enemy.name);
    }

    if (millis() - introTimer >= 2000) {
        introTimer = 0;
        m_phase = BattlePhase::PLAYER;
    }
}

// ============================================================================
// ФАЗА PLAYER (ВЫБОР ДЕЙСТВИЯ)
// ============================================================================

void BattleScene::updatePlayerTurn() {
    InputManager& input = InputManager::getInstance();

    // Навигация по меню (4 действия в ряд)
    if (input.justPressed(Button::LEFT) && (int)m_selectedAction > 0) {
        m_selectedAction = static_cast<BattleAction>((int)m_selectedAction - 1);
    }
    if (input.justPressed(Button::RIGHT) && m_selectedAction < BattleAction::MERCY) {
        m_selectedAction = static_cast<BattleAction>((int)m_selectedAction + 1);
    }

    // Подтверждение действия
    if (input.justPressed(Button::A)) {
        executeAction(m_selectedAction);
    }
}

// ============================================================================
// ФАЗА ENEMY (BULLET-HELL)
// ============================================================================

void BattleScene::updateEnemyTurn() {
    // Временно — просто ждём 1.5 секунды
    static uint32_t enemyTimer = 0;
    if (enemyTimer == 0) {
        enemyTimer = millis();
    }

    if (millis() - enemyTimer >= 1500) {
        enemyTimer = 0;

        // Наносим урон игроку (если не COUNTER-HACK)
        if (!m_counterHackAvailable) {
            int damage = std::max<int>(1, (int)(m_enemy.attack - random(0, 3)));
            m_playerBattleHP = std::max<int>(0, m_playerBattleHP - damage);
            LOG_INFO_F("Battle: player took %d damage (%d/%d)",
                       damage, m_playerBattleHP, m_playerMaxBattleHP);
        } else {
            // COUNTER-HACK успешен — враг получает урон
            int damage = m_enemy.attack * 2;
            m_enemy.hp = std::max<int>(0, m_enemy.hp - damage);
            LOG_INFO_F("Battle: COUNTER-HACK! %d damage to %s",
                       damage, m_enemy.name);
            m_counterHackAvailable = false;
        }

        m_phase = BattlePhase::RESULT;
        checkBattleEnd();
    }
}

// ============================================================================
// ФАЗА RESULT
// ============================================================================

void BattleScene::updateResult() {
    static uint32_t resultTimer = 0;
    if (resultTimer == 0) {
        resultTimer = millis();
    }

    if (millis() - resultTimer >= 1000) {
        resultTimer = 0;
        m_turnCount++;

        if (m_phase != BattlePhase::VICTORY && m_phase != BattlePhase::DEFEAT) {
            m_phase = BattlePhase::PLAYER;
        }
    }
}

// ============================================================================
// COUNTER-HACK
// ============================================================================

void BattleScene::updateCounterHack() {
    // Окно для COUNTER-HACK (200 мс)
    if (m_counterHackTimer > 0) {
        if (millis() - m_counterHackTimer >= COUNTER_HACK_WINDOW_MS) {
            m_counterHackAvailable = false;
            m_counterHackTimer = 0;
            m_phase = BattlePhase::ENEMY;
        }

        // Кнопка B для COUNTER-HACK
        if (InputManager::getInstance().justPressed(Button::B)) {
            m_counterHackAvailable = true;
            m_counterHackTimer = 0;
            LOG_INFO("Battle: COUNTER-HACK activated!");
            m_phase = BattlePhase::ENEMY;
        }
    }
}

// ============================================================================
// ВЫПОЛНЕНИЕ ДЕЙСТВИЯ
// ============================================================================

void BattleScene::executeAction(BattleAction action) {
    m_lastAction = action;

    switch (action) {
        case BattleAction::HACK:
            doHack();
            break;
        case BattleAction::SCAN:
            doScan();
            break;
        case BattleAction::LINK:
            doLink();
            break;
        case BattleAction::MERCY:
            doMercy();
            break;
        default:
            break;
    }
}

// ============================================================================
// HACK — АТАКА
// ============================================================================

void BattleScene::doHack() {
    int baseDamage = 3 + m_scanLevel;  // SCAN увеличивает урон
    int damage = max(1, baseDamage - m_enemy.defense / 2);
    m_enemy.hp = max(0, m_enemy.hp - damage);

    LOG_INFO_F("Battle: HACK! %d damage to %s", damage, m_enemy.name);

    // Переход к фазе COUNTER-HACK
    m_counterHackTimer = millis();
    m_phase = BattlePhase::COUNTER;

    checkBattleEnd();
}

// ============================================================================
// SCAN — СКАНИРОВАНИЕ
// ============================================================================

void BattleScene::doScan() {
    m_scanned = true;
    m_scanLevel = min(m_scanLevel + 1, 5);

    LOG_INFO_F("Battle: SCAN! %s analyzed (level %d)", m_enemy.name, m_scanLevel);

    // SCAN не тратит ход врага
    m_phase = BattlePhase::PLAYER;
}

// ============================================================================
// LINK — ВОССТАНОВЛЕНИЕ
// ============================================================================

void BattleScene::doLink() {
    int heal = 3 + random(0, 3);
    m_playerBattleHP = min(m_playerMaxBattleHP, m_playerBattleHP + heal);

    LOG_INFO_F("Battle: LINK! +%d HP (%d/%d)", heal, m_playerBattleHP, m_playerMaxBattleHP);

    // Переход к фазе врага
    m_phase = BattlePhase::ENEMY;
}

// ============================================================================
// MERCY — ПОЩАДА
// ============================================================================

void BattleScene::doMercy() {
    // MERCY работает только если HP врага ниже 50%
    if (m_enemy.hp <= m_enemy.maxHP / 2) {
        LOG_INFO("Battle: MERCY! Enemy spared");
        m_phase = BattlePhase::VICTORY;
    } else {
        LOG_INFO("Battle: MERCY failed (enemy too strong)");
        m_phase = BattlePhase::ENEMY;
    }
}

// ============================================================================
// ПРОВЕРКА ОКОНЧАНИЯ БОЯ
// ============================================================================

void BattleScene::checkBattleEnd() {
    if (m_enemy.hp <= 0) {
        // Проверка смены фазы босса
        if (m_battleType == BattleType::BOSS &&
            m_enemy.currentPhase < m_enemy.phaseCount - 1) {
            checkPhaseTransition();
        } else {
            m_phase = BattlePhase::VICTORY;
            LOG_INFO("Battle: VICTORY!");
        }
    }

    if (m_playerBattleHP <= 0) {
        m_phase = BattlePhase::DEFEAT;
        LOG_INFO("Battle: DEFEAT!");
    }
}

void BattleScene::checkPhaseTransition() {
    m_enemy.currentPhase++;
    m_enemy.hp = m_enemy.maxHP;  // Восстанавливаем HP для новой фазы

    LOG_INFO_F("Battle: %s phase %d/%d",
               m_enemy.name, m_enemy.currentPhase + 1, m_enemy.phaseCount);

    m_phase = BattlePhase::INTRO;
}

// ============================================================================
// ОТРИСОВКА
// ============================================================================

void BattleScene::render() {
    DisplayManager& display = DisplayManager::getInstance();

    // Фон боя
    display.clearBuffer(Palette::DEEP_BG);

    // Верхняя часть (меню + враг)
    renderTop();

    // Нижняя часть (SOUL BOX)
    renderBottom();

    // HP бары
    renderEnemyHP();
    renderPlayerHP();

    // Меню действий
    if (m_phase == BattlePhase::PLAYER) {
        renderActionMenu();
    }

    // Текст фазы
    switch (m_phase) {
        case BattlePhase::INTRO: {
            // Имя врага крупно
            int textX = DISPLAY_WIDTH / 2 - strlen(m_enemy.name) * 4;
            display.fillRect(textX, 60, strlen(m_enemy.name) * 8, 16, m_enemy.color);
            break;
        }
        case BattlePhase::VICTORY:
            display.fillRect(100, 100, 120, 20, Palette::MERCY_COLOR);
            break;
        case BattlePhase::DEFEAT:
            display.fillRect(100, 100, 120, 20, Palette::HP_LOW);
            break;
        case BattlePhase::COUNTER:
            // Мигающий индикатор COUNTER-HACK
            if ((millis() / 100) % 2 == 0) {
                display.fillRect(120, 110, 80, 16, Palette::YELLOW);
            }
            break;
        default:
            break;
    }
}

void BattleScene::renderTop() {
    DisplayManager& display = DisplayManager::getInstance();

    // Верхняя половина экрана (0-140)
    // Враг — цветной прямоугольник
    int enemyX = DISPLAY_WIDTH / 2 - 40;
    int enemyY = 30;
    int enemyW = 80;
    int enemyH = 80;

    display.fillRect(enemyX, enemyY, enemyW, enemyH, m_enemy.color);
    display.drawRect(enemyX, enemyY, enemyW, enemyH, Palette::WHITE);

    // Анимация врага (лёгкое покачивание)
    if (m_animFrame % 2 == 0) {
        display.drawRect(enemyX + 1, enemyY + 1, enemyW - 2, enemyH - 2,
                        Palette::darkenColor(m_enemy.color, 0.3f));
    }
}

void BattleScene::renderBottom() {
    DisplayManager& display = DisplayManager::getInstance();

    // SOUL BOX — зона уклонения (нижняя часть)
    int boxX = DISPLAY_WIDTH / 2 - SOUL_BOX_WIDTH / 2;
    int boxY = 150;
    int boxW = SOUL_BOX_WIDTH;
    int boxH = SOUL_BOX_HEIGHT;

    display.drawRect(boxX, boxY, boxW, boxH, Palette::TEXT_WHITE);

    // SOUL (сердце игрока) — маленький квадрат
    int soulX = boxX + boxW / 2 - 3;
    int soulY = boxY + boxH / 2 - 3;
    display.fillRect(soulX, soulY, 6, 6, Palette::HP_FULL);
}

void BattleScene::renderActionMenu() {
    DisplayManager& display = DisplayManager::getInstance();

    const char* actions[] = {"HACK", "SCAN", "LINK", "MERCY"};
    uint16_t colors[] = {
        Palette::HACK_COLOR,
        Palette::SCAN_COLOR,
        Palette::LINK_COLOR,
        Palette::MERCY_COLOR
    };

    int menuY = 130;
    int menuW = DISPLAY_WIDTH / 4;

    for (int i = 0; i < 4; i++) {
        int x = i * menuW;
        uint16_t bgColor = (i == (int)m_selectedAction) ?
                           colors[i] : Palette::darkenColor(colors[i], 0.7f);

        display.fillRect(x, menuY, menuW - 2, 14, bgColor);
        display.drawRect(x, menuY, menuW - 2, 14, Palette::WHITE);
    }
}

void BattleScene::renderEnemyHP() {
    DisplayManager& display = DisplayManager::getInstance();

    // HP бар врага (вверху)
    int barX = 10;
    int barY = 10;
    int barW = 150;
    int barH = 8;

    // Фон
    display.fillRect(barX, barY, barW, barH, Palette::BLACK);
    display.drawRect(barX, barY, barW, barH, Palette::TEXT_WHITE);

    // Текущее HP
    if (m_enemy.maxHP > 0) {
        int hpW = (barW - 2) * m_enemy.hp / m_enemy.maxHP;
        uint16_t hpColor = (m_enemy.hp > m_enemy.maxHP / 2) ?
                           Palette::HP_FULL : Palette::HP_LOW;
        display.fillRect(barX + 1, barY + 1, hpW, barH - 2, hpColor);
    }
}

void BattleScene::renderPlayerHP() {
    DisplayManager& display = DisplayManager::getInstance();

    // HP бар игрока (внизу, слева от SOUL BOX)
    int barX = 10;
    int barY = DISPLAY_HEIGHT - 20;
    int barW = 100;
    int barH = 8;

    display.fillRect(barX, barY, barW, barH, Palette::BLACK);
    display.drawRect(barX, barY, barW, barH, Palette::TEXT_WHITE);

    int hpW = (barW - 2) * m_playerBattleHP / m_playerMaxBattleHP;
    uint16_t hpColor = (m_playerBattleHP > m_playerMaxBattleHP / 2) ?
                       Palette::HP_FULL : Palette::HP_LOW;
    display.fillRect(barX + 1, barY + 1, hpW, barH - 2, hpColor);
}

// ============================================================================
// ВХОД / ВЫХОД
// ============================================================================

void BattleScene::onEnter() {
    Scene::onEnter();
    LOG_INFO("Battle: scene entered");
}

void BattleScene::onExit() {
    Scene::onExit();
    LOG_INFO("Battle: scene exited");
}