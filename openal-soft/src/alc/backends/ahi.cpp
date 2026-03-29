/**
 * OpenAL Soft — AHI backend for AmigaOS4
 *
 * Double-buffered audio output via ahi.device. The mixer thread fills one
 * buffer while the other is being played by AHI.
 *
 * Reference: salass00/openal_lib (1.18.2, Alc/backends/ahi.c)
 */

#include "config.h"

#include "ahi.h"

#include <atomic>
#include <cstring>
#include <exception>
#include <functional>
#include <thread>
#include <vector>

#include "althrd_setname.h"
#include "almalloc.h"
#include "core/device.h"
#include "core/helpers.h"
#include "core/logging.h"

#include <exec/exec.h>
#include <devices/ahi.h>
#include <proto/exec.h>

namespace {

constexpr char ahiDevice[] = "AHI Default";

struct AHIPlayback final : public BackendBase {
    AHIPlayback(DeviceBase *device) noexcept : BackendBase{device} {}
    ~AHIPlayback() override;

    void open(std::string_view name) override;
    bool reset() override;
    void start() override;
    void stop() override;

private:
    int mixerProc();

    struct MsgPort  *mAHIPort{nullptr};
    struct AHIRequest mAHIReq[2]{};
    bool             mAHIOpen{false};
    int              mCurrentBuffer{0};

    uint32_t         mAHIType{0};      /* AHIST_* format */
    uint32_t         mFrameSize{0};

    std::vector<std::byte> mMixBuf[2]; /* double buffer */

    std::atomic<bool> mKillNow{true};
    std::thread mThread;

public:
    DEF_NEWDEL(AHIPlayback)
};

AHIPlayback::~AHIPlayback()
{
    if(mAHIOpen)
    {
        IExec->CloseDevice(reinterpret_cast<struct IORequest*>(&mAHIReq[0]));
        mAHIOpen = false;
    }
    if(mAHIPort)
    {
        IExec->FreeSysObject(ASOT_PORT, mAHIPort);
        mAHIPort = nullptr;
    }
}

int AHIPlayback::mixerProc()
{
    SetRTPriority();
    althrd_setname(MIXER_THREAD_NAME);

    const uint32_t updateLen = mDevice->UpdateSize;
    const uint32_t bufBytes  = updateLen * mFrameSize;

    /* Prime both buffers */
    for(int i = 0; i < 2; i++)
    {
        mDevice->renderSamples(mMixBuf[i].data(), updateLen, mFrameSize);
    }

    /* Send first buffer */
    auto &req0 = mAHIReq[0];
    req0.ahir_Std.io_Command = CMD_WRITE;
    req0.ahir_Std.io_Data    = mMixBuf[0].data();
    req0.ahir_Std.io_Length  = bufBytes;
    req0.ahir_Type           = mAHIType;
    req0.ahir_Frequency      = mDevice->Frequency;
    req0.ahir_Volume         = 0x10000;  /* Unity gain */
    req0.ahir_Position       = 0x8000;   /* Center pan */
    req0.ahir_Link           = nullptr;
    IExec->SendIO(reinterpret_cast<struct IORequest*>(&req0));

    mCurrentBuffer = 1;

    while(!mKillNow.load(std::memory_order_acquire)
        && mDevice->Connected.load(std::memory_order_acquire))
    {
        int cur  = mCurrentBuffer;
        int prev = 1 - cur;

        /* Fill current buffer while previous plays */
        mDevice->renderSamples(mMixBuf[cur].data(), updateLen, mFrameSize);

        /* Set up and send current buffer, linked to previous */
        auto &reqCur = mAHIReq[cur];
        reqCur.ahir_Std.io_Command = CMD_WRITE;
        reqCur.ahir_Std.io_Data    = mMixBuf[cur].data();
        reqCur.ahir_Std.io_Length  = bufBytes;
        reqCur.ahir_Type           = mAHIType;
        reqCur.ahir_Frequency      = mDevice->Frequency;
        reqCur.ahir_Volume         = 0x10000;
        reqCur.ahir_Position       = 0x8000;
        reqCur.ahir_Link           = &mAHIReq[prev];

        IExec->SendIO(reinterpret_cast<struct IORequest*>(&reqCur));

        /* Wait for previous buffer to finish playing */
        IExec->WaitIO(reinterpret_cast<struct IORequest*>(&mAHIReq[prev]));

        mCurrentBuffer = 1 - cur;
    }

    /* Wait for any outstanding request */
    IExec->WaitIO(reinterpret_cast<struct IORequest*>(&mAHIReq[0]));
    IExec->WaitIO(reinterpret_cast<struct IORequest*>(&mAHIReq[1]));

    return 0;
}

void AHIPlayback::open(std::string_view name)
{
    if(name.empty())
        name = ahiDevice;
    else if(name != ahiDevice)
        throw al::backend_exception{al::backend_error::NoDevice,
            "Device name \"%.*s\" not found",
            static_cast<int>(name.length()), name.data()};

    mAHIPort = static_cast<struct MsgPort*>(
        IExec->AllocSysObject(ASOT_PORT, nullptr));
    if(!mAHIPort)
        throw al::backend_exception{al::backend_error::OutOfMemory,
            "Failed to create AHI message port"};

    std::memset(&mAHIReq[0], 0, sizeof(struct AHIRequest));
    mAHIReq[0].ahir_Std.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    mAHIReq[0].ahir_Std.io_Message.mn_ReplyPort = mAHIPort;
    mAHIReq[0].ahir_Std.io_Message.mn_Length = sizeof(struct AHIRequest);
    mAHIReq[0].ahir_Version = 6;

    if(IExec->OpenDevice(AHINAME, AHI_DEFAULT_UNIT,
        reinterpret_cast<struct IORequest*>(&mAHIReq[0]), 0) != 0)
    {
        throw al::backend_exception{al::backend_error::NoDevice,
            "Failed to open ahi.device"};
    }
    mAHIOpen = true;

    /* Copy first request to second for double-buffering */
    mAHIReq[1] = mAHIReq[0];

    mDevice->DeviceName = name;
}

bool AHIPlayback::reset()
{
    /* Map OpenAL format to AHI type */
    switch(mDevice->FmtType)
    {
    case DevFmtByte:
        mDevice->FmtType = DevFmtShort;
        /* fall through */
    case DevFmtShort:
        if(mDevice->FmtChans == DevFmtMono)
            mAHIType = AHIST_M16S;
        else
        {
            mDevice->FmtChans = DevFmtStereo;
            mAHIType = AHIST_S16S;
        }
        break;

    case DevFmtUByte:
        mDevice->FmtType = DevFmtShort;
        if(mDevice->FmtChans == DevFmtMono)
            mAHIType = AHIST_M16S;
        else
        {
            mDevice->FmtChans = DevFmtStereo;
            mAHIType = AHIST_S16S;
        }
        break;

    case DevFmtUShort:
        mDevice->FmtType = DevFmtShort;
        if(mDevice->FmtChans == DevFmtMono)
            mAHIType = AHIST_M16S;
        else
        {
            mDevice->FmtChans = DevFmtStereo;
            mAHIType = AHIST_S16S;
        }
        break;

    case DevFmtInt:
    case DevFmtUInt:
    case DevFmtFloat:
        /* AHI supports 32-bit signed via AHIST_S32S/AHIST_M32S */
        mDevice->FmtType = DevFmtInt;
        if(mDevice->FmtChans == DevFmtMono)
            mAHIType = AHIST_M32S;
        else
        {
            mDevice->FmtChans = DevFmtStereo;
            mAHIType = AHIST_S32S;
        }
        break;
    }

    mFrameSize = mDevice->frameSizeFromFmt();

    /* Allocate double buffers */
    const uint32_t bufBytes = mDevice->UpdateSize * mFrameSize;
    mMixBuf[0].resize(bufBytes, std::byte{0});
    mMixBuf[1].resize(bufBytes, std::byte{0});

    setDefaultWFXChannelOrder();
    return true;
}

void AHIPlayback::start()
{
    try {
        mKillNow.store(false, std::memory_order_release);
        mThread = std::thread{std::mem_fn(&AHIPlayback::mixerProc), this};
    }
    catch(std::exception &e) {
        throw al::backend_exception{al::backend_error::DeviceError,
            "Failed to start mixing thread: %s", e.what()};
    }
}

void AHIPlayback::stop()
{
    if(mKillNow.exchange(true, std::memory_order_acq_rel) || !mThread.joinable())
        return;

    /* Signal the port to unblock any pending WaitIO */
    IExec->Signal(IExec->FindTask(nullptr),
        1 << mAHIPort->mp_SigBit);

    mThread.join();
}

BackendPtr AHIPlaybackCreate(DeviceBase *device)
{ return BackendPtr{new AHIPlayback{device}}; }

} // namespace


bool AHIBackendFactory::init()
{
    /* Quick check that ahi.device can be opened */
    struct MsgPort *port = static_cast<struct MsgPort*>(
        IExec->AllocSysObject(ASOT_PORT, nullptr));
    if(!port) return false;

    struct AHIRequest req{};
    req.ahir_Std.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    req.ahir_Std.io_Message.mn_ReplyPort = port;
    req.ahir_Std.io_Message.mn_Length = sizeof(struct AHIRequest);
    req.ahir_Version = 6;

    bool ok = (IExec->OpenDevice(AHINAME, AHI_DEFAULT_UNIT,
        reinterpret_cast<struct IORequest*>(&req), 0) == 0);
    if(ok)
        IExec->CloseDevice(reinterpret_cast<struct IORequest*>(&req));

    IExec->FreeSysObject(ASOT_PORT, port);

    if(ok)
        TRACE("AHI backend: ahi.device available\n");
    else
        ERR("AHI backend: failed to open ahi.device\n");

    return ok;
}

bool AHIBackendFactory::querySupport(BackendType type)
{
    return (type == BackendType::Playback);
}

std::string AHIBackendFactory::probe(BackendType type)
{
    std::string outnames;
    switch(type)
    {
    case BackendType::Playback:
        outnames.append(ahiDevice, sizeof(ahiDevice));
        break;
    case BackendType::Capture:
        break;
    }
    return outnames;
}

BackendPtr AHIBackendFactory::createBackend(DeviceBase *device, BackendType type)
{
    if(type == BackendType::Playback)
        return AHIPlaybackCreate(device);
    return nullptr;
}

BackendFactory &AHIBackendFactory::getFactory()
{
    static AHIBackendFactory factory{};
    return factory;
}
