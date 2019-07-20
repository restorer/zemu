#ifndef HOST__STAGE_H__INCLUDED
#define HOST__STAGE_H__INCLUDED

#include <cstdint>
#include <SDL_mouse.h>
#include "ZEmuConfig.h"
#include "keycode.h"

#define STAGE_MAKERGB(r, g, b) (((r) << 0x10) | ((g) << 8) | (b))
#define STAGE_GETR(c) ((c) >> 0x10)
#define STAGE_GETG(c) (((c) >> 0x8) & 0xFF)
#define STAGE_GETB(c) ((c) & 0xFF)

#ifdef ZEMU_BIG_ENDIAN
    #define STAGE_FLIPPED_ARGB
#endif

#define STAGE_MOUSE_LMASK SDL_BUTTON_LMASK
#define STAGE_MOUSE_RMASK SDL_BUTTON_MMASK
#define STAGE_MOUSE_MMASK SDL_BUTTON_RMASK

enum StageSoundDriver {
    STAGE_SOUND_DRIVER_NONE,
    STAGE_SOUND_DRIVER_GENERIC,
    STAGE_SOUND_DRIVER_NATIVE
};

enum StageRenderMode {
    STAGE_RENDER_MODE_1X,
    STAGE_RENDER_MODE_2X,
    STAGE_RENDER_MODE_2X_SCANLINES
};

struct StageConfig {
    int initialWidth;
    int initialHeight;
    StageSoundDriver soundDriver;
    StageRenderMode renderMode;
    bool isFullscreen;
};

enum StageEventType {
    STAGE_EVENT_QUIT,
    STAGE_EVENT_KEYDOWN,
    STAGE_EVENT_KEYUP,
    STAGE_EVENT_JOYDOWN,
    STAGE_EVENT_JOYUP,
    STAGE_EVENT_MOUSEWHEEL
};

enum StageJoystickButton {
    STAGE_JOYSTICK_UP,
    STAGE_JOYSTICK_DOWN,
    STAGE_JOYSTICK_LEFT,
    STAGE_JOYSTICK_RIGHT,
    STAGE_JOYSTICK_FIRE
};

struct StageEvent {
    StageEventType type;
    int keyCode;
    StageJoystickButton joyButton;
    int mouseWheelDirection;
};

struct StageMouseState {
    int x;
    int y;
    int buttons;
};

class Stage {
public:

    Stage() {}
    virtual ~Stage() {}

    virtual StageRenderMode getRenderMode() = 0;
    virtual void setRenderMode(StageRenderMode mode) = 0;

    virtual bool isKeyRepeat() = 0;
    virtual void setKeyRepeat(bool repeat) = 0;

    virtual bool isFullscreen() = 0;
    virtual void setFullscreen(bool fullscreen) = 0;

    virtual bool isSoundEnabled() = 0;
    virtual void setSoundEnabled(bool enabled) = 0;

    virtual bool pollEvent(StageEvent* into) = 0;
    virtual void getMouseState(StageMouseState* info) = 0;

    virtual void renderFrame(uint32_t* pixels, int width, int height) = 0; // In ARGB format
    virtual void renderSound(uint32_t* buffer, int samples) = 0; // 2 x int16_t (stereo) for each sample

private:

    Stage(const Stage&);
    Stage& operator=(const Stage&);
};

#endif
