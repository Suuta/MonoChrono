
#pragma once


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <unordered_map>
#include <comip.h>
#include <xaudio2.h>
#include <mfidl.h>
#include <mfreadwrite.h>


//-------- マクロ
#ifdef COMPTR
#undef COMPTR
#endif

#define COMPTR(type)         _com_ptr_t<_com_IIID<type,&__uuidof(type)>>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)      if((x)){ (x)->Release(); (x) = nullptr; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) if((x)){ delete[] (x); (x) = nullptr; }
#endif

#ifndef SAFE_DESTROYVOICE
#define SAFE_DESTROY_VOICE(x) if((x)){(x)->DestroyVoice(); (x) = nullptr;}
#endif

#define MAX_BUFFER_COUNT      3 // ストリーム再生用バッファ数
#define MAX_DUP              16 // 最大多重再生数


FORCEINLINE float SemitonesToFrequencyRatio(float Semitones)
{
    // FrequencyRatio = 2 ^ Octaves
    //                = 2 ^ (Semitones / 12)
    return powf(2.0f, Semitones / 12.0f);
}


typedef uint32_t SoundHandle;

enum SoundFileType
{
    Stream = 0,
    Effect = 1,

    MAX_SoundFileType
};

enum SoundStatus
{
    SS_STOP  = 0,
    SS_PLAY  = 1,
    SS_PAUSE = 2,

    MAX_SOUNDSTATUS
};


//************************************************
//	BGMクラス
//************************************************
struct SoundStream
{
public:

    SoundStream(const std::wstring& name);
    virtual ~SoundStream();

    HRESULT Open(IXAudio2* pXAudio2, LPCWSTR pszPath);
    void    Update();
    void    Close();
    void    Play();
    void    Stop();
    void    Pause();
    void    Resume();
    bool    IsPlaying();
    void    SetVolume(float fVol);
    void    SetPitch(float pitch);
    float   GetVolume() const;


    const std::wstring& GetName() const { return m_Name; }

private:

    std::wstring m_Name;

    IXAudio2SourceVoice*    m_pSourceVoice;
    COMPTR(IMFSourceReader)	m_reader;
    DWORD					m_currentStreamBuffer;
    std::vector<BYTE>		m_buffers[MAX_BUFFER_COUNT];
    bool					m_endOfStream;
    SoundStatus				m_status;
};


//************************************************
//	SEクラス
//************************************************
struct SoundEffect
{
public:

    SoundEffect(const std::wstring& name);
    virtual ~SoundEffect();

    HRESULT Open(IXAudio2* pXAudio2, LPCWSTR pszPath);
    void    Close();
    void    Play();
    void    Stop(int n = -1);
    void    Pause();
    void    Resume();
    bool    IsPlaying();
    void    SetVolume(float fVol);
    float   GetVolume() const;

    const std::wstring& GetName() const { return m_Name; }

private:

    std::wstring m_Name;

    IXAudio2SourceVoice* m_pSourceVoice[MAX_DUP];
    SoundStatus          m_status[MAX_DUP];
    std::vector<BYTE>    m_buffer;
    DWORD                m_bufferLength;
};



//************************************************
//	オーディオマネージャー
//************************************************
class AudioManager
{
public:

    AudioManager()          = default;
    virtual ~AudioManager() = default;

    static SoundHandle LoadFromFile(const std::wstring& filePath, SoundFileType type);

    static void Init();
    static void Update();
    static void Fin();

    static void PlayStream(SoundHandle hSound)      { m_BGMList[hSound]->Play(); }
    static void StopStresm(SoundHandle hSound)      { m_BGMList[hSound]->Stop(); }
    static bool IsPlaying(SoundHandle hSound) { return m_BGMList[hSound]->IsPlaying(); }

    static void PlaySE(SoundHandle hSound) { m_SEList[hSound]->Play(); }
    static void StopSE(SoundHandle hSound) { m_SEList[hSound]->Stop(); }
                                                     
    static void  Pause()
    {
        for (const auto& bgm : m_BGMList) bgm.second->Pause();
        for (const auto& se : m_BGMList)  se.second->Pause();
    }

    static void  Resume()
    {
        for (const auto& bgm : m_BGMList) bgm.second->Resume();
        for (const auto& se : m_BGMList)  se.second->Resume();
    }

    static void  SetVolume(float fVol) { if (m_pMasteringVoice) m_pMasteringVoice->SetVolume(fVol); }

    static float GetVolume()
    {
        float fVol = 0.0f;
        if (m_pMasteringVoice) m_pMasteringVoice->GetVolume(&fVol);

        return fVol;
    }

    static void SetPitch(SoundHandle hSound, float pitch) { m_BGMList[hSound]->SetPitch(pitch); }

    static void  SetStreamVolume(SoundHandle hSound, float fVol) { m_BGMList[hSound]->SetVolume(fVol); }
    static float GetStreamVolume(SoundHandle hSound)             { m_BGMList[hSound]->GetVolume();     }
    static void  SetSEVolume(SoundHandle hSound, float fVol)     { m_SEList[hSound]->SetVolume(fVol);  }
    static float GetSEVolume(SoundHandle hSound)                 { m_SEList[hSound]->GetVolume();      }

private:

    static IXAudio2*               m_pXAudio2;
    static IXAudio2MasteringVoice* m_pMasteringVoice;


    static std::unordered_map<SoundHandle, SoundStream*> m_BGMList;
    static std::unordered_map<SoundHandle, SoundEffect*> m_SEList;

    static uint64_t s_SoundID;
};