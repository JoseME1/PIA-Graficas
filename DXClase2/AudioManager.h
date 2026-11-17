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
    AudioManager* musicaCarrera = nullptr;
    float volumenActual = 0.3f;
    bool comInicializado = false;

    // Lee un WAV PCM y llena wfx + pAudioData
    bool LoadWav(const wchar_t* filename, bool repetir)
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
                ZeroMemory(&wfx, sizeof(wfx));
                DWORD toRead = size < sizeof(WAVEFORMATEX) ? size : sizeof(WAVEFORMATEX);
                file.read((char*)&wfx, toRead);
                wfx.cbSize = 0;
                if (size > toRead)
                    file.seekg(size - toRead, std::ios::cur);
                if (size & 1) file.seekg(1, std::ios::cur);
            }

            else if (strncmp(id, "data", 4) == 0)
            {
                pAudioData = new BYTE[size];
                file.read((char*)pAudioData, size);
                buffer.AudioBytes = size;
                buffer.pAudioData = pAudioData;
                buffer.Flags = XAUDIO2_END_OF_STREAM;
                buffer.LoopCount = repetir ? XAUDIO2_LOOP_INFINITE : 0;
                return true;
            }
            else
            {
                file.seekg(size, std::ios::cur);
                if (size & 1) file.seekg(1, std::ios::cur);
            }
        }
        return false;
    }

public:
    void Pausar() { if (pSource) pSource->Stop(0); }
    void Reanudar() { if (pSource) pSource->Start(0); }
    float Progreso() const
    {
        if (!pSource || !wfx.nBlockAlign || !buffer.AudioBytes) return 1.0f;
        XAUDIO2_VOICE_STATE estado = {};
        pSource->GetState(&estado);
        if (estado.BuffersQueued == 0) return 1.0f;
        float progreso = (float)((double)estado.SamplesPlayed / (buffer.AudioBytes / wfx.nBlockAlign));
        return progreso < 1.0f ? progreso : 1.0f;
    }
    bool Init(const wchar_t* filename, float volume = 0.3f, bool repetir = true)
    {
        volumenActual = volume;
        if (!LoadWav(filename, repetir)) return false;

        HRESULT com = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        comInicializado = SUCCEEDED(com);
        if (FAILED(XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
            return false;
        if (FAILED(pXAudio2->CreateMasteringVoice(&pMastering)))
            return false;
        if (FAILED(pXAudio2->CreateSourceVoice(&pSource, &wfx)))
            return false;

        if (FAILED(pSource->SetVolume(volume)) ||
            FAILED(pSource->SubmitSourceBuffer(&buffer)) || FAILED(pSource->Start(0)))
            return false;
        return true;
    }

    void SetVolume(float volume)
    {
        volumenActual = volume;
        if (pSource) pSource->SetVolume(volume);
        if (musicaCarrera) musicaCarrera->SetVolume(volume);
    }

    bool IniciarMusicaCarrera()
    {
        DetenerMusicaCarrera();
        musicaCarrera = new AudioManager();
        if (!musicaCarrera->Init(L"Sonidos/umapyoi.wav", volumenActual, false))
        {
            musicaCarrera->Shutdown();
            delete musicaCarrera;
            musicaCarrera = nullptr;
            return false;
        }
        if (pSource) pSource->Stop(0);
        return true;
    }

    void DetenerMusicaCarrera()
    {
        if (!musicaCarrera) return;
        musicaCarrera->Shutdown();
        delete musicaCarrera;
        musicaCarrera = nullptr;
        if (pSource) pSource->Start(0);
    }

    void ActualizarMusica()
    {
        if (!musicaCarrera || !musicaCarrera->pSource) return;
        XAUDIO2_VOICE_STATE estado = {};
        musicaCarrera->pSource->GetState(&estado);
        if (estado.BuffersQueued == 0) DetenerMusicaCarrera();
    }

    void Shutdown()
    {
        DetenerMusicaCarrera();
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
        if (comInicializado) { CoUninitialize(); comInicializado = false; }
        delete[] pAudioData;
        pAudioData = nullptr;
    }
};

#endif
