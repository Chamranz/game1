/*
 * NEON REQUIEM - Sprite Animation Implementation
 * Manages sprite animation states, frame timing, and transitions
 */

#include "sprite_animation.h"
#include "../sprite_renderer.h"

// Singleton instance
SpriteAnimation& SpriteAnimation::getInstance() {
    static SpriteAnimation instance;
    return instance;
}

void SpriteAnimation::begin() {
    reset();
    LOG_INFO("SpriteAnimation: Initialized (%d clips, %d instances)", 
             MAX_CLIPS, MAX_INSTANCES);
}

void SpriteAnimation::reset() {
    m_clipCount = 0;
    m_instanceCount = 0;
    
    for (int i = 0; i < MAX_INSTANCES; i++) {
        m_instances[i].active = false;
    }
}

// ===== Animation Clip Management =====

bool SpriteAnimation::loadClip(const char* clipId, const char* spriteId, AnimMode mode,
                                const AnimFrame* frames, uint8_t frameCount) {
    if (m_clipCount >= MAX_CLIPS) {
        LOG_WARN("SpriteAnimation: Max clips reached");
        return false;
    }
    
    if (frameCount > MAX_FRAMES_PER_CLIP) {
        LOG_WARN("SpriteAnimation: Too many frames (%d > %d)", frameCount, MAX_FRAMES_PER_CLIP);
        return false;
    }
    
    // Allocate frame data in PSRAM
    AnimFrame* frameData = (AnimFrame*)ps_malloc(sizeof(AnimFrame) * frameCount);
    if (!frameData) {
        LOG_ERROR("SpriteAnimation: Failed to allocate frames for '%s'", clipId);
        return false;
    }
    
    memcpy(frameData, frames, sizeof(AnimFrame) * frameCount);
    
    AnimClip& clip = m_clips[m_clipCount];
    strncpy(clip.id, clipId, sizeof(clip.id) - 1);
    clip.id[sizeof(clip.id) - 1] = '\0';
    strncpy(clip.spriteId, spriteId, sizeof(clip.spriteId) - 1);
    clip.spriteId[sizeof(clip.spriteId) - 1] = '\0';
    clip.mode = mode;
    clip.frameCount = frameCount;
    clip.frames = frameData;
    clip.interruptible = true;
    clip.priority = 0;
    clip.blendTime = 0;
    m_clipCount++;
    
    LOG_VERBOSE("SpriteAnimation: Loaded clip '%s' (%d frames, mode=%d)", 
                clipId, frameCount, mode);
    return true;
}

void SpriteAnimation::unloadClip(const char* clipId) {
    int8_t idx = findClipIndex(clipId);
    if (idx < 0) return;
    
    // Stop any active instances
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (m_instances[i].active && m_instances[i].clip == &m_clips[idx]) {
            m_instances[i].active = false;
            m_instanceCount--;
        }
    }
    
    // Free frame data
    if (m_clips[idx].frames) {
        free(m_clips[idx].frames);
    }
    
    // Shift clips
    for (int i = idx; i < m_clipCount - 1; i++) {
        m_clips[i] = m_clips[i + 1];
    }
    m_clipCount--;
}

AnimClip* SpriteAnimation::getClip(const char* clipId) {
    int8_t idx = findClipIndex(clipId);
    if (idx < 0) return nullptr;
    return &m_clips[idx];
}

// ===== Playback Control =====

bool SpriteAnimation::play(const char* clipId, uint8_t priority) {
    int8_t clipIdx = findClipIndex(clipId);
    if (clipIdx < 0) return false;
    
    AnimClip* clip = &m_clips[clipIdx];
    
    // Check if already playing
    int8_t existingIdx = findInstanceIndex(clipId);
    if (existingIdx >= 0) {
        // Restart
        m_instances[existingIdx].currentFrame = 0;
        m_instances[existingIdx].frameTimer = 0;
        m_instances[existingIdx].state = ANIM_PLAYING;
        m_instances[existingIdx].direction = 1;
        m_instances[existingIdx].loopCount = 0;
        return true;
    }
    
    // Find free instance
    int8_t instIdx = findFreeInstance();
    if (instIdx < 0) {
        // Stop lowest priority instance
        uint8_t lowestPrio = 255;
        int8_t lowestIdx = -1;
        for (int i = 0; i < MAX_INSTANCES; i++) {
            if (m_instances[i].active && m_instances[i].priority < lowestPrio) {
                lowestPrio = m_instances[i].priority;
                lowestIdx = i;
            }
        }
        if (lowestIdx >= 0 && lowestPrio <= priority) {
            m_instances[lowestIdx].active = false;
            instIdx = lowestIdx;
        } else {
            return false;
        }
    }
    
    AnimInstance& inst = m_instances[instIdx];
    inst.clip = clip;
    inst.state = ANIM_PLAYING;
    inst.currentFrame = 0;
    inst.frameTimer = 0;
    inst.direction = 1;
    inst.loopCount = 0;
    inst.priority = priority;
    inst.active = true;
    m_instanceCount++;
    
    return true;
}

bool SpriteAnimation::playOnLayer(const char* clipId, uint8_t layer, uint8_t priority) {
    // Layer-based playback (for multi-layer animations)
    return play(clipId, priority);
}

void SpriteAnimation::stop(const char* clipId) {
    int8_t idx = findInstanceIndex(clipId);
    if (idx < 0) return;
    
    m_instances[idx].active = false;
    m_instances[idx].state = ANIM_STOPPED;
    m_instanceCount--;
}

void SpriteAnimation::stopAll() {
    for (int i = 0; i < MAX_INSTANCES; i++) {
        m_instances[i].active = false;
        m_instances[i].state = ANIM_STOPPED;
    }
    m_instanceCount = 0;
}

void SpriteAnimation::pause(const char* clipId) {
    int8_t idx = findInstanceIndex(clipId);
    if (idx < 0) return;
    m_instances[idx].state = ANIM_PAUSED;
}

void SpriteAnimation::resume(const char* clipId) {
    int8_t idx = findInstanceIndex(clipId);
    if (idx < 0) return;
    m_instances[idx].state = ANIM_PLAYING;
}

void SpriteAnimation::setSpeed(const char* clipId, float speed) {
    // Speed control via frame timer scaling
    // Implementation would scale deltaTime for this instance
}

// ===== Update and Render =====

void SpriteAnimation::update(uint16_t deltaTime) {
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (!m_instances[i].active) continue;
        
        AnimInstance& inst = m_instances[i];
        if (inst.state != ANIM_PLAYING) continue;
        
        inst.frameTimer += deltaTime;
        
        AnimClip* clip = inst.clip;
        if (!clip || clip->frameCount == 0) continue;
        
        // Check if current frame duration has elapsed
        if (inst.frameTimer >= clip->frames[inst.currentFrame].duration) {
            inst.frameTimer -= clip->frames[inst.currentFrame].duration;
            advanceFrame(inst);
        }
    }
}

void SpriteAnimation::render(int16_t baseX, int16_t baseY) {
    SpriteRenderer& renderer = SpriteRenderer::getInstance();
    
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (!m_instances[i].active) continue;
        
        AnimInstance& inst = m_instances[i];
        if (inst.state == ANIM_STOPPED) continue;
        
        AnimClip* clip = inst.clip;
        if (!clip) continue;
        
        AnimFrame& frame = clip->frames[inst.currentFrame];
        
        renderer.drawSpriteFrame(clip->spriteId, 
                                 baseX + frame.offsetX, 
                                 baseY + frame.offsetY, 
                                 frame.spriteFrame);
    }
}

void SpriteAnimation::renderOnLayer(int16_t baseX, int16_t baseY, uint8_t layer) {
    // Layer-specific rendering
    render(baseX, baseY);
}

// ===== State Queries =====

bool SpriteAnimation::isPlaying(const char* clipId) const {
    int8_t idx = findInstanceIndex(clipId);
    if (idx < 0) return false;
    return m_instances[idx].state == ANIM_PLAYING;
}

bool SpriteAnimation::isFinished(const char* clipId) const {
    int8_t idx = findInstanceIndex(clipId);
    if (idx < 0) return true;
    return m_instances[idx].state == ANIM_FINISHED;
}

uint8_t SpriteAnimation::getCurrentFrame(const char* clipId) const {
    int8_t idx = findInstanceIndex(clipId);
    if (idx < 0) return 0;
    return m_instances[idx].currentFrame;
}

uint8_t SpriteAnimation::getLoopCount(const char* clipId) const {
    int8_t idx = findInstanceIndex(clipId);
    if (idx < 0) return 0;
    return m_instances[idx].loopCount;
}

// ===== Internal =====

void SpriteAnimation::advanceFrame(AnimInstance& instance) {
    AnimClip* clip = instance.clip;
    if (!clip || clip->frameCount == 0) return;
    
    switch (clip->mode) {
        case ANIM_LOOP:
            instance.currentFrame = (instance.currentFrame + 1) % clip->frameCount;
            if (instance.currentFrame == 0) instance.loopCount++;
            break;
            
        case ANIM_PING_PONG:
            instance.currentFrame += instance.direction;
            if (instance.currentFrame >= clip->frameCount - 1) {
                instance.direction = -1;
            } else if (instance.currentFrame == 0) {
                instance.direction = 1;
                instance.loopCount++;
            }
            break;
            
        case ANIM_ONCE:
            if (instance.currentFrame < clip->frameCount - 1) {
                instance.currentFrame++;
            } else {
                instance.state = ANIM_FINISHED;
            }
            break;
            
        case ANIM_ONCE_HOLD:
            if (instance.currentFrame < clip->frameCount - 1) {
                instance.currentFrame++;
            } else {
                instance.state = ANIM_FINISHED;
                // Hold on last frame
            }
            break;
            
        case ANIM_RANDOM:
            instance.currentFrame = random(clip->frameCount);
            break;
    }
}

int8_t SpriteAnimation::findClipIndex(const char* clipId) const {
    for (int i = 0; i < m_clipCount; i++) {
        if (strcmp(m_clips[i].id, clipId) == 0) {
            return i;
        }
    }
    return -1;
}

int8_t SpriteAnimation::findInstanceIndex(const char* clipId) const {
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (m_instances[i].active && m_instances[i].clip && 
            strcmp(m_instances[i].clip->id, clipId) == 0) {
            return i;
        }
    }
    return -1;
}

int8_t SpriteAnimation::findFreeInstance() const {
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (!m_instances[i].active) {
            return i;
        }
    }
    return -1;
}