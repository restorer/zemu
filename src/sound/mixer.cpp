// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <assert.h>
#include "mixer.h"

#define AUDIO_NATIVE_BUFFER 1024
#define MIXER_FULL_VOL_MASK 1
#define MIXER_SMART_MASK 2

C_SoundMixer soundMixer;

C_SoundMixer::~C_SoundMixer() {
    if (!initialized || !wavFileWriter) {
        return;
    }

    try {
        wavFileWriter = nullptr;

        // TODO: use appropriate library for writing wave files where available
        long wavSz = (long)wavFileTempPath->fileSize();

        wavFileWriter = wavFilePath->fileWriter();
        wavFileWriter->writeDword(0x46464952);
        wavFileWriter->writeDword(wavSz + 0x40 - 8);
        wavFileWriter->writeDword(0x45564157);
        wavFileWriter->writeDword(0x20746D66);
        wavFileWriter->writeDword(0x10);
        wavFileWriter->writeWord(1);
        wavFileWriter->writeWord(2);
        wavFileWriter->writeDword(SOUND_FREQ);
        wavFileWriter->writeDword(SOUND_FREQ * 4);
        wavFileWriter->writeWord(4);
        wavFileWriter->writeWord(16);
        wavFileWriter->writeDword(0x61746164);
        wavFileWriter->writeDword(wavSz);

        auto wavTmpReader = wavFileTempPath->fileReader();

        while (!wavTmpReader->isEof()) {
            wavFileWriter->writeByte(wavTmpReader->readByte());
        }

        wavFileWriter = nullptr;
        wavFileTempPath->remove();
    } catch (...) {
        printf("Unknown error occurred while finishing .wav\n");
    }
}

void C_SoundMixer::InitBackendDefault(int bufferSize) {
    sndBackend = new CSndBackendDefault(SND_CALLBACK_BUFFER_SIZE * bufferSize);
    sndBackend->Init();
}

#ifdef _WIN32
    void C_SoundMixer::InitBackendWin32(int soundParam) {
        sndBackend = new CSndBackendWIN32(soundParam, AUDIO_NATIVE_BUFFER);
        sndBackend->Init();
    }
#endif

#ifdef __unix__
    void C_SoundMixer::InitBackendOSS(int soundParam) {
        sndBackend = new CSndBackendOSS(soundParam, AUDIO_NATIVE_BUFFER);
        sndBackend->Init();
    }
#endif

void C_SoundMixer::Init(int mixerMode, bool recordWav, const char* wavFileName) { //-V688
    assert(sndBackend != nullptr);

    this->mixerMode = mixerMode;

    if (recordWav && *wavFileName) {
        wavFilePath = hostEnv->fileSystem()->path(wavFileName);
        wavFileTempPath = wavFilePath->concat(".tmp");
        wavFileWriter = wavFileTempPath->fileWriter();
    }

    initialized = true;
}

void C_SoundMixer::AddSource(C_SndRenderer* source) {
    sources.push_back(source);
    source->mixBuffer = mixBuffer;
}

void C_SoundMixer::FlushFrame(bool soundEnabled) {
    assert(initialized);
    int sourcesCount = sources.size();

    if (sourcesCount == 0) {
        return;
    }

    if (!soundEnabled) {
        for (int i = 0; i < sourcesCount; i++) {
            sources[i]->samples = 0;
        }

        return;
    }

    unsigned minSamples = sources[0]->samples;
    unsigned maxSamples = sources[0]->samples;

    for (int i = 1; i < sourcesCount; i++) {
        unsigned samples = sources[i]->samples;

        if (samples < minSamples) {
            minSamples = samples;
        }

        if (samples > maxSamples) {
            maxSamples = samples;
        }
    }

    if (minSamples) {
        s_Sample* p = mixBuffer;

        if (mixerMode & MIXER_SMART_MASK) {
            unsigned long divider = 0L;

            for (int i = 0; i < sourcesCount; i++) {
                if (sources[i]->activeCnt) {
                    divider++;
                    sources[i]->activeCnt--;
                }
            }

            if (divider) {
                p = mixBuffer;

                for (int i = minSamples; i--; p++) {
                    p->left /= divider;
                    p->right /= divider;
                }
            }
        } else if (sourcesCount > 1) {
            for (int i = minSamples; i--; p++) {
                p->left /= sourcesCount;
                p->right /= sourcesCount;
            }
        }

        p = mixBuffer;
        uint16_t *o = audioBuffer;

        if (mixerMode & MIXER_FULL_VOL_MASK) {
            for (int i = minSamples; i--; p++) {
                *(o++) = (uint16_t)((long)p->left - 0x8000L);
                *(o++) = (uint16_t)((long)p->right - 0x8000L);
            }
        } else {
            for (int i = minSamples; i--; p++) {
                *(o++) = (uint16_t)(p->left >> 1);
                *(o++) = (uint16_t)(p->right >> 1);
            }
        }

        if (wavFileWriter) {
            o = audioBuffer;

            for (int i = minSamples; i--;) {
                wavFileWriter->writeWord(*(o++));
                wavFileWriter->writeWord(*(o++));
            }
        }

        sndBackend->Write((uint8_t*)audioBuffer, minSamples * sizeof(uint16_t) * 2);
    }

    if (maxSamples > minSamples) {
        memmove(mixBuffer, &mixBuffer[minSamples], (maxSamples - minSamples) * sizeof(s_Sample));
    }

    memset(&mixBuffer[maxSamples - minSamples], 0, (MIX_BUFFER_SIZE) * sizeof(s_Sample));

    for (int i = 0; i < sourcesCount; i++) {
        sources[i]->samples -= minSamples;
    }
}
