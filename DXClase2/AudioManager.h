#pragma once
#ifndef _AudioManager_
#define _AudioManager_

#include <xaudio2.h>
#include <fstream>
#pragma comment(lib, "xaudio2.lib")

class AudioManager
{
private:
    IXAudio2* pXAudio2 = nullptr;
    IXAudio2MasteringVoice* pMastering = nullptr;
    IXAudio2SourceVoice* pSource = nullptr;
    WAVEFORMATEX wfx = {};
    BYTE* pAudioData = nullptr;
    XAUDIO2_BUFFER buffer = {};

    // Lee un WAV PCM y llena wfx + pAudioData
    bool LoadWav(const wchar_t* filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;

        char chunkId[4];
        file.read(chunkId, 4);              // "RIFF"
        if (strncmp(chunkId, "RIFF", 4) != 0) return false;
        file.seekg(4, std::ios::cur);       // tamano RIFF
        file.read(chunkId, 4);              // "WAVE"
        if (strncmp(chunkId, "WAVE", 4) != 0) return false;

        // Recorre los chunks hasta encontrar "fmt " y "data"
        while (file)
        {
            char id[4];
            DWORD size = 0;
            file.read(id, 4);
            file.read((char*)&size, 4);
            if (!file) break;

            if (strncmp(id, "fmt ", 4) == 0)
            {
                file.read((char*)&wfx, sizeof(WAVEFORMATEX));
                if (size > sizeof(WAVEFORMATEX))
                    file.seekg(size - sizeof(WAVEFORMATEX), std::ios::cur);
            }
            else if (strncmp(id, "data", 4) == 0)
            {
                pAudioData = new BYTE[size];
                file.read((char*)pAudioData, size);
                buffer.AudioBytes = size;
                buffer.pAudioData = pAudioData;
                buffer.Flags = XAUDIO2_END_OF_STREAM;
                buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
                return true;
            }
            else
            {
                file.seekg(size, std::ios::cur);
            }
        }
        return false;
    }

public:
    bool Init(const wchar_t* filename, float volume = 0.3f)
    {
        if (!LoadWav(filename)) return false;

        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
            return false;
        if (FAILED(pXAudio2->CreateMasteringVoice(&pMastering)))
            return false;
        if (FAILED(pXAudio2->CreateSourceVoice(&pSource, &wfx)))
            return false;

        pSource->SetVolume(volume);
        pSource->SubmitSourceBuffer(&buffer);
        pSource->Start(0);
        return true;
    }

    void SetVolume(float volume)
    {
        if (pSource) pSource->SetVolume(volume);
    }

    void Shutdown()
    {
        if (pSource)
        {
            pSource->Stop(0);
            pSource->FlushSourceBuffers();
            pSource->DestroyVoice();
            pSource = nullptr;
        }
        if (pMastering)
        {
            pMastering->DestroyVoice();
            pMastering = nullptr;
        }
        if (pXAudio2)
        {
            pXAudio2->Release();
            pXAudio2 = nullptr;
        }
        CoUninitialize();
        delete[] pAudioData;
        pAudioData = nullptr;
    }
};

#endif
