/*
 * NEON REQUIEM - Sprite Animation
 * Manages sprite animation states, frame timing, and transitions
 * Supports looping, ping-pong, one-shot, and blend animations
 */

#ifndef SPRITE_ANIMATION_H
#define SPRITE_ANIMATION_H

#include <Arduino.h>
#include "config/config.h"

// Animation modes
enum AnimMode : uint8_t {
    ANIM_LOOP = 0,        // Loop continuously
    ANIM_PING_PONG = 1,   // Forward then reverse
    ANIM_ONCE = 2,         // Play once then stop
    ANIM_ONCE_HOLD = 3,    // Play once then hold last frame
    ANIM_RANDOM = 4        // Random frame selection
};

// Animation state
enum AnimState : uint8_t {
    ANIM_STOPPED = 0,
    ANIM_PLAYING = 1,
    ANIM_PAUSED = 2,
    ANIM_FINISHED = 3
};

// Animation frame
struct AnimFrame {
    uint8_t spriteFrame;    // Index into sprite sheet
    uint16_t duration;      // Frame duration in ms
    int16_t offsetX;        // X offset from base position
    int16_t offsetY;        // Y offset from base position
    uint8_t flags;          // Per-frame flags
};

// Animation clip
struct AnimClip {
    char id[20];            // Animation identifier
    char spriteId[20];      // Associated sprite
    AnimMode mode;          // Playback mode
    uint8_t frameCount;     // Number of frames
    AnimFrame* frames;      // Frame array (in PSRAM)
    bool interruptible;     // Can be interrupted by another anim?
    uint8_t priority;       // 0=low, 255=high
    uint8_t blendTime;      // Cross-fade time in frames
};

// Active animation instance
struct AnimInstance {
    AnimClip* clip;         // Pointer to clip definition
    AnimState state;        // Current state
    uint8_t currentFrame;   // Current frame index
    uint16_t frameTimer;    // Time in current frame (ms)
    int16_t direction;      // 1=forward, -1=reverse (for ping-pong)
    uint8_t loopCount;      // Number of loops completed
    uint8_t priority;       // Current priority
    bool active;
};

class SpriteAnimation {
public:
    // Singleton access
    static SpriteAnimation& getInstance();
    
    // Initialization
    void begin();
    void reset();
    
    // Animation clip management
    bool loadClip(const char* clipId, const char* spriteId, AnimMode mode, 
                  const AnimFrame* frames, uint8_t frameCount);
    void unloadClip(const char* clipId);
    AnimClip* getClip(const char* clipId);
    
    // Playback control
    bool play(const char* clipId, uint8_t priority = 0);
    bool playOnLayer(const char* clipId, uint8_t layer, uint8_t priority = 0);
    void stop(const char* clipId);
    void stopAll();
    void pause(const char* clipId);
    void resume(const char* clipId);
    void setSpeed(const char* clipId, float speed);
    
    // Update and render
    void update(uint16_t deltaTime);
    void render(int16_t baseX, int16_t baseY);
    void renderOnLayer(int16_t baseX, int16_t baseY, uint8_t layer);
    
    // State queries
    bool isPlaying(const char* clipId) const;
    bool isFinished(const char* clipId) const;
    uint8_t getCurrentFrame(const char* clipId) const;
    uint8_t getLoopCount(const char* clipId) const;
    
    // Animation layers
    void setLayerCount(uint8_t count) { m_layerCount = min(count, (uint8_t)8); }
    uint8_t getLayerCount() const { return m_layerCount; }

private:
    SpriteAnimation() : m_clipCount(0), m_instanceCount(0), m_layerCount(4) {}
    ~SpriteAnimation() {}
    SpriteAnimation(const SpriteAnimation&) = delete;
    SpriteAnimation& operator=(const SpriteAnimation&) = delete;
    
    int8_t findClipIndex(const char* clipId) const;
    int8_t findInstanceIndex(const char* clipId) const;
    int8_t findFreeInstance() const;
    void advanceFrame(AnimInstance& instance);
    
    static const uint8_t MAX_CLIPS = 32;
    static const uint8_t MAX_INSTANCES = 16;
    static const uint8_t MAX_FRAMES_PER_CLIP = 16;
    
    AnimClip m_clips[MAX_CLIPS];
    uint8_t m_clipCount;
    
    AnimInstance m_instances[MAX_INSTANCES];
    uint8_t m_instanceCount;
    
    uint8_t m_layerCount;
};

#endif // SPRITE_ANIMATION_H