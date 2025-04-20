
#include "Audio.h"
#include "conioex2.h"
#include <cassert>
#include <mmreg.h>
#include <comdef.h>
#include <initguid.h>
#include <mfapi.h>

#include <filesystem>

// 静的リンク ライブラリ
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")



namespace
{
    // グローバル変数
    LPCWSTR g_pszDirSound = L"data/";
    LPCWSTR g_pszDirBGM = L"sound/";
    LPCWSTR g_pszDirSE = L"sound/";

    HRESULT CreateMFReader(LPCWSTR mediaFile, COMPTR(IMFSourceReader)& reader, WAVEFORMATEX* wfx, size_t maxwfx)
    {
        if (!mediaFile || !wfx)
        {
            return E_INVALIDARG;
        }

        HRESULT hr;
        COMPTR(IMFAttributes) attr;
        hr = MFCreateAttributes(&attr, 1);
        if (FAILED(hr))
            return hr;

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
        hr = attr->SetUINT32(MF_LOW_LATENCY, TRUE);
        if (FAILED(hr))
            return hr;
#endif

        hr = MFCreateSourceReaderFromURL(mediaFile, attr, &reader);
        if (FAILED(hr))
            return hr;

        COMPTR(IMFMediaType) mediaType;
        hr = MFCreateMediaType(&mediaType);
        if (FAILED(hr))
            return hr;

        hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        if (FAILED(hr))
            return hr;

        hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        if (FAILED(hr))
            return hr;

        hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaType);
        if (FAILED(hr))
            return hr;

        COMPTR(IMFMediaType) outputMediaType;
        hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outputMediaType);
        if (FAILED(hr))
            return hr;

        UINT32 waveFormatSize = 0;
        WAVEFORMATEX* waveFormat = nullptr;
        hr = MFCreateWaveFormatExFromMFMediaType(outputMediaType, &waveFormat, &waveFormatSize);
        if (FAILED(hr))
            return hr;

        memcpy_s(wfx, maxwfx, waveFormat, waveFormatSize);
        CoTaskMemFree(waveFormat);

        return S_OK;
    }
};




//************************************************
// 静的メンバ
//************************************************
IXAudio2* AudioManager::m_pXAudio2 = nullptr;
IXAudio2MasteringVoice* AudioManager::m_pMasteringVoice = nullptr;

uint64_t AudioManager::s_SoundID = 0;

std::unordered_map<SoundHandle, SoundStream*> AudioManager::m_BGMList;
std::unordered_map<SoundHandle, SoundEffect*> AudioManager::m_SEList;



//************************************************
//	BGMクラス
//************************************************

// BGMクラス コンストラクタ
SoundStream::SoundStream(const std::wstring& name)
    : m_pSourceVoice(nullptr)
    , m_reader(nullptr)
    , m_currentStreamBuffer(0)
    , m_endOfStream(false)
    , m_status(SS_STOP)
    , m_Name(name)
{
    for (int i = 0; i < MAX_BUFFER_COUNT; ++i)
    {
        m_buffers[i].resize(32768);
    }
}

// BGMクラス デストラクタ
SoundStream::~SoundStream()
{
    SAFE_DESTROY_VOICE(m_pSourceVoice);
}

// BGMオープン
HRESULT SoundStream::Open(IXAudio2* pXAudio2, LPCWSTR pszPath)
{
    if (!pszPath || !*pszPath)
        return E_INVALIDARG;

    WAVEFORMATEX wfx;
    ::ZeroMemory(&wfx, sizeof(wfx));

    HRESULT hr = CreateMFReader(pszPath, m_reader, &wfx, sizeof(wfx));
    if (FAILED(hr)) {
        return hr;
    }

    hr = pXAudio2->CreateSourceVoice(&m_pSourceVoice, &wfx, XAUDIO2_VOICE_USEFILTER, 1.0f, nullptr);
    return hr;
}

// BGMポーリング
void SoundStream::Update()
{
    if (m_status != SS_PLAY) return;

    if (m_endOfStream)
    {
        Stop();
        Play();
        return;
    }

    if (!m_pSourceVoice) return;


    XAUDIO2_VOICE_STATE state = { 0 };
    m_pSourceVoice->GetState(&state);

    if (state.BuffersQueued >= MAX_BUFFER_COUNT - 1) return;
    if (!m_reader) return;

    COMPTR(IMFSample) sample;
    DWORD flags = 0;
    HRESULT hr = m_reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, &sample);
    if (FAILED(hr))
        return;

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
    {
        m_endOfStream = true;
        return;
    }

    COMPTR(IMFMediaBuffer) mediaBuffer;
    hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
    if (FAILED(hr))
        return;

    BYTE* audioData = nullptr;
    DWORD sampleBufferLength = 0;
    hr = mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength);
    if (FAILED(hr))
        return;

    std::vector<BYTE>& buffer = m_buffers[m_currentStreamBuffer];
    m_currentStreamBuffer++;
    m_currentStreamBuffer %= MAX_BUFFER_COUNT;
    buffer.resize(sampleBufferLength);
    memcpy_s(&buffer[0], sampleBufferLength, audioData, sampleBufferLength);
    hr = mediaBuffer->Unlock();
    if (FAILED(hr))
        return;

    XAUDIO2_BUFFER buf = { 0 };
    buf.AudioBytes = sampleBufferLength;
    buf.pAudioData = &buffer[0];
    m_pSourceVoice->SubmitSourceBuffer(&buf);
}

// BGMクローズ
void SoundStream::Close()
{
    Stop();
    if (m_reader)
    {
        m_reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
    }
    SAFE_DESTROY_VOICE(m_pSourceVoice);
}

// BGM再生
void SoundStream::Play()
{
    if (m_pSourceVoice && m_status != SS_PLAY)
    {
        m_pSourceVoice->Start();
        m_status = SS_PLAY;
    }
}

// BGM停止
void SoundStream::Stop()
{
    if (m_pSourceVoice && m_status != SS_STOP)
    {
        m_status = SS_STOP;
        m_pSourceVoice->Stop();
        m_pSourceVoice->FlushSourceBuffers();

        if (m_reader)
        {
            PROPVARIANT var = { 0 };
            var.vt = VT_I8;
            m_reader->SetCurrentPosition(GUID_NULL, var);
        }

        m_endOfStream = false;
    }
}

// BGM一時停止
void SoundStream::Pause()
{
    if (m_pSourceVoice && m_status == SS_PLAY)
    {
        m_status = SS_PAUSE;
        m_pSourceVoice->Stop();
    }
}

// BGM再開
void SoundStream::Resume()
{
    if (m_pSourceVoice && m_status == SS_PAUSE) Play();
}

// BGM再生中チェック
bool SoundStream::IsPlaying()
{
    return m_status == SS_PLAY && !m_endOfStream;
}

// BGMボリューム設定
void SoundStream::SetVolume(float fVol)
{
    if (m_pSourceVoice) m_pSourceVoice->SetVolume(fVol);
}

void SoundStream::SetPitch(float pitch)
{
    float freqRatio = SemitonesToFrequencyRatio(pitch * 12.f);
    if (m_pSourceVoice) m_pSourceVoice->SetFrequencyRatio(freqRatio);
}

// BGMボリューム取得
float SoundStream::GetVolume() const
{
    float fVol = 0.0f;
    if (m_pSourceVoice) m_pSourceVoice->GetVolume(&fVol);

    return fVol;
}





//************************************************
//	SEクラス
//************************************************

// SEクラス コンストラクタ
SoundEffect::SoundEffect(const std::wstring& name)
    : m_bufferLength(0)
    , m_Name(name)
{
    for (int i = 0; i < MAX_DUP; ++i)
    {
        m_pSourceVoice[i] = nullptr;
        m_status[i] = SS_STOP;
    }
}

// SEクラス デストラクタ
SoundEffect::~SoundEffect()
{
    for (int i = 0; i < MAX_DUP; ++i)
    {
        SAFE_DESTROY_VOICE(m_pSourceVoice[i]);
    }
}

// SEオープン
HRESULT SoundEffect::Open(IXAudio2* pXAudio2, LPCWSTR pszPath)
{
    if (!pszPath || !*pszPath)
        return E_INVALIDARG;

    HRESULT hr;
    COMPTR(IMFSourceReader) reader;
    WAVEFORMATEX wfx;
    ::ZeroMemory(&wfx, sizeof(wfx));

    hr = CreateMFReader(pszPath, reader, &wfx, sizeof(wfx));
    if (FAILED(hr))
        return hr;

    for (int i = 0; i < MAX_DUP; ++i)
    {
        hr = pXAudio2->CreateSourceVoice(&m_pSourceVoice[i], &wfx, XAUDIO2_VOICE_NOPITCH, 1.0f, nullptr);
        if (FAILED(hr))
            return hr;
    }

    m_bufferLength = 0;

    for (;;)
    {
        COMPTR(IMFSample) sample;
        DWORD flags = 0;

        hr = reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, &sample);
        if (FAILED(hr))
            return hr;

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
            break;

        COMPTR(IMFMediaBuffer) mediaBuffer;
        hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
        if (FAILED(hr))
            return hr;

        BYTE* audioData = nullptr;
        DWORD sampleBufferLength = 0;
        hr = mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength);
        if (FAILED(hr))
            return hr;

        m_buffer.resize(m_bufferLength + sampleBufferLength);
        memcpy_s(&m_buffer[m_bufferLength], sampleBufferLength, audioData, sampleBufferLength);
        m_bufferLength += sampleBufferLength;
        hr = mediaBuffer->Unlock();
        if (FAILED(hr))
            return hr;

    }

    reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);

    return hr;
}

// SEクローズ
void SoundEffect::Close()
{
    Stop();
    for (int i = 0; i < MAX_DUP; ++i)
    {
        SAFE_DESTROY_VOICE(m_pSourceVoice[i]);
    }
}

// SE再生
void SoundEffect::Play()
{
    int i;
    for (i = 0; i < MAX_DUP; ++i)
    {
        if (!m_pSourceVoice[i])
            continue;

        if (m_status[i] == SS_STOP)
            break;

    }

    if (i >= MAX_DUP)
    {
        for (i = 0; i < MAX_DUP; ++i)
        {
            if (!m_pSourceVoice[i])
                continue;

            if (m_status[i] == SS_PAUSE)
            {
                Stop(i);
                break;
            }
        }

        if (i >= MAX_DUP)
        {
            XAUDIO2_VOICE_STATE state = { 0 };
            for (i = 0; i < MAX_DUP; ++i)
            {
                if (!m_pSourceVoice[i])
                    continue;

                m_pSourceVoice[i]->GetState(&state);
                if (state.BuffersQueued > 0)
                    continue;

                Stop(i);
                break;
            }
        }

        if (i >= MAX_DUP) // 最大まで多重再生中
        {
            i = rand() % MAX_DUP;
            if (!m_pSourceVoice[i])
                return;     // 再生不可

            Stop(i);        // 乱数で適当にどれかを止める
        }
    }

    XAUDIO2_BUFFER buf = { 0 };
    {
        buf.AudioBytes = m_bufferLength;
        buf.pAudioData = &m_buffer[0];
        buf.Flags = XAUDIO2_END_OF_STREAM;
    }

    m_pSourceVoice[i]->SubmitSourceBuffer(&buf);
    m_pSourceVoice[i]->Start();
    m_status[i] = SS_PLAY;
}

// SE停止
void SoundEffect::Stop(int n)
{
    if (n < 0 || n >= MAX_DUP)
    {
        for (int i = 0; i < MAX_DUP; ++i)
        {
            if (m_pSourceVoice[i] && m_status[i] != SS_STOP)
            {
                m_status[i] = SS_STOP;
                m_pSourceVoice[i]->Stop();
                m_pSourceVoice[i]->FlushSourceBuffers();
            }
        }
        return;
    }

    if (m_pSourceVoice[n] && m_status[n] != SS_STOP)
    {
        m_status[n] = SS_STOP;
        m_pSourceVoice[n]->Stop();
        m_pSourceVoice[n]->FlushSourceBuffers();
    }
}

// SE一時停止
void SoundEffect::Pause()
{
    for (int i = 0; i < MAX_DUP; ++i)
    {
        if (m_pSourceVoice[i] && m_status[i] == SS_PLAY)
        {
            m_status[i] = SS_PAUSE;
            m_pSourceVoice[i]->Stop();
        }
    }
}

// SE再開
void SoundEffect::Resume()
{
    for (int i = 0; i < MAX_DUP; ++i)
    {
        if (m_pSourceVoice[i] && m_status[i] == SS_PAUSE)
        {
            m_status[i] = SS_PLAY;
            m_pSourceVoice[i]->Start();
        }
    }
}

// SE再生中チェック
bool SoundEffect::IsPlaying()
{
    for (int i = 0; i < MAX_DUP; ++i)
    {
        if (m_pSourceVoice[i] && m_status[i] == SS_PLAY)
            return true;
    }
    return false;
}

// SEボリューム設定
void SoundEffect::SetVolume(float fVol)
{
    for (int i = 0; i < MAX_DUP; ++i)
    {
        if (m_pSourceVoice[i])
            m_pSourceVoice[i]->SetVolume(fVol);
    }
}

// SEボリューム取得
float SoundEffect::GetVolume() const
{
    float fVol = 0.0f;
    for (int i = 0; i < MAX_DUP; ++i)
    {
        if (m_pSourceVoice[i])
        {
            m_pSourceVoice[i]->GetVolume(&fVol);
            break;
        }
    }
    return fVol;
}





//************************************************
//  オーディオマネージャー
//************************************************

SoundHandle AudioManager::LoadFromFile(const std::wstring& filePath, SoundFileType type)
{
    switch (type)
    {
    case Effect:
    {
        auto* se = new SoundEffect(filePath);
        if (FAILED(se->Open(m_pXAudio2, filePath.c_str()))) {
            ::MessageBoxW(nullptr, filePath.c_str(), L"BGM:error", MB_OK);
        }

        m_SEList[++s_SoundID] = se;
        return s_SoundID;
    }
    case Stream:
    {
        auto* bgm = new SoundStream(filePath);
        if (FAILED(bgm->Open(m_pXAudio2, filePath.c_str()))) {
            ::MessageBoxW(nullptr, filePath.c_str(), L"BGM:error", MB_OK);
        }

        m_BGMList[++s_SoundID] = bgm;
        return s_SoundID;
    }
    }
    return NULL;
}


void AudioManager::Init()
{
    HRESULT hr = S_OK;
    if (!m_pXAudio2)
    {
        UINT32 flags = 0;
#if defined(_DEBUG)
        //flags |= XAUDIO2_DEBUG_ENGINE;
#endif

        hr = XAudio2Create(&m_pXAudio2, flags);
        if (FAILED(hr))
        {
            m_pXAudio2 = nullptr;
        }//渡す
        else
        {
#if defined(_DEBUG)
            XAUDIO2_DEBUG_CONFIGURATION debug = { 0 };
            debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
            debug.BreakMask = XAUDIO2_LOG_ERRORS;
            m_pXAudio2->SetDebugConfiguration(&debug, 0);
#endif

            hr = m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);
            if (FAILED(hr))
            {
                m_pMasteringVoice = nullptr;
                SAFE_RELEASE(m_pXAudio2);
            }
            else
            {
                hr = MFStartup(MF_VERSION);
                if (FAILED(hr))
                {
                    SAFE_DESTROY_VOICE(m_pMasteringVoice);
                    SAFE_RELEASE(m_pXAudio2);
                }
            }
        }
    }
}

// ポーリング
void AudioManager::Update(void)
{
    for (const auto bgm : m_BGMList)
    {
        bgm.second->Update();
    }
}

// 解放
void AudioManager::Fin(void)
{
    for (auto bgm : m_BGMList)
    {
        bgm.second->Close();
        delete bgm.second;
        bgm.second = nullptr;
    }

    for (auto se : m_SEList)
    {
        se.second->Close();
        delete se.second;
        se.second = nullptr;
    }

    m_BGMList.clear();
    m_SEList.clear();

    MFShutdown();

    SAFE_DESTROY_VOICE(m_pMasteringVoice);
    SAFE_RELEASE(m_pXAudio2);
}

