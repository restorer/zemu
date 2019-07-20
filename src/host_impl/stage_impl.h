#ifndef HOST_IMPL__STAGE_IMPL_H__INCLUDED
#define HOST_IMPL__STAGE_IMPL_H__INCLUDED

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_mutex.h>
#include "ZEmuConfig.h"
#include "host/stage.h"
#include "host/logger.h"
#include "driver/sound_driver.h"

#ifdef _WIN32
    #include <windows.h>
#endif

class StageImpl : public Stage {
public:
    StageImpl(const StageConfig& stageConfig, Logger* logger);
    virtual ~StageImpl();

    StageRenderMode getRenderMode();
    void setRenderMode(StageRenderMode mode);

    bool isKeyRepeat();
    void setKeyRepeat(bool keyRepeat);

    bool isFullscreen();
    void setFullscreen(bool fullscreen);

    bool isSoundEnabled();
    void setSoundEnabled(bool soundEnabled);

    bool pollEvent(StageEvent* into);
    void getMouseState(StageMouseState* into);

    void renderFrame(uint32_t* pixels, int width, int height);
    void renderSound(uint32_t* buffer, int samples);

private:

    int hints;
    volatile StageRenderMode renderMode;
    volatile int lastFrameWidth;
    volatile int lastFrameHeight;
    bool keyRepeat = false;
    bool fullscreen;
    bool soundEnabled;
    SDL_Surface* nativeSurface = nullptr;
    // SDL_Event nativeEvent;

    volatile bool isRenderThreadActive = true;
    volatile bool isRenderThreadPixelsConsumed = true;
    SDL_sem* renderThreadPixelsReadySem;
    SDL_Thread* renderThread;
    uint32_t* volatile renderThreadPixels = nullptr;

    std::unique_ptr<SoundDriver> soundDriver;

    #ifndef USE_SDL1
        std::string stageTitle;
        SDL_Window* nativeWindow = nullptr;
        SDL_Renderer* nativeRenderer = nullptr;
        SDL_Texture* nativeTexture = nullptr;
    #endif

    #ifdef _WIN32
        HICON windowsIcon = nullptr;
    #endif

    void refreshVideoSubsystem();
    void renderThreadLoop();
    void renderThreadUpdateSurface1x();
    void renderThreadUpdateSurface2x();
    void renderThreadUpdateSurface2xScanlines();

    friend int stageImplRenderThreadFunction(void* data);
};

#endif