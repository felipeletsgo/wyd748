#include "../internal/platform/windows/DeviceFrame.h"
#include <cstdio>
#include <string>

namespace {
struct Device {
    HRESULT cooperative = S_OK;
    HRESULT begin = S_OK;
    HRESULT end = S_OK;
    std::string order;
    HRESULT TestCooperativeLevel() { order += 't'; return cooperative; }
    HRESULT BeginScene() { order += 'b'; return begin; }
    HRESULT EndScene() { order += 'e'; return end; }
};
struct Renderer {
    int lockResult = 1;
    int unlockResult = 1;
    std::string order;
    int Lock(int clear) { order += clear ? 'B' : 'b'; return lockResult; }
    int Unlock(int present) { order += present ? 'P' : 'e'; return unlockResult; }
};
}

int RunDeviceFrameTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::fprintf(stderr, "DeviceFrame: %s\n", name); }
    };
    const HRESULT results[] = {S_OK, S_FALSE, E_FAIL, D3DERR_INVALIDCALL,
        D3DERR_DEVICELOST, D3DERR_DEVICENOTRESET};
    for (HRESULT clearResult : results) {
        for (HRESULT beginResult : results) {
            Device device;
            device.begin = beginResult;
            bool lost = false;
            const bool begun = device_frame::Begin(&device, lost, true,
                [&]() -> HRESULT { device.order += 'p'; return E_FAIL; },
                [&]() -> HRESULT { device.order += 'r'; return E_FAIL; },
                [&] { device.order += 'c'; return clearResult; });
            check(begun == (SUCCEEDED(clearResult) && SUCCEEDED(beginResult)), "strict begin success");
            check(device.order == (SUCCEEDED(clearResult) ? "cb" : "c"), "no begin after failed clear");
            const HRESULT executed = FAILED(clearResult) ? clearResult : beginResult;
            check(lost == (executed == D3DERR_DEVICELOST || executed == D3DERR_DEVICENOTRESET),
                "only loss and not-reset schedule recovery");
        }
    }
    for (HRESULT status : results) {
        for (bool restored : {false, true}) {
            for (HRESULT prepareResult : {S_OK, E_FAIL}) {
                for (HRESULT resetResult : {S_OK, E_OUTOFMEMORY}) {
                    Device device;
                    device.cooperative = status;
                    bool lost = true;
                    const bool begun = device_frame::Begin(&device, lost, restored,
                        [&] { device.order += 'p'; return prepareResult; },
                        [&] { device.order += 'r'; return resetResult; },
                        [&]() -> HRESULT { device.order += 'c'; return S_OK; });
                    const bool recoverable = SUCCEEDED(status) || status == D3DERR_DEVICENOTRESET;
                    const bool needsReset = status == D3DERR_DEVICENOTRESET || !restored;
                    std::string expected = "t";
                    bool expectedLost = true;
                    bool expectedBegin = false;
                    if (recoverable) {
                        if (needsReset) {
                            expected += 'p';
                            if (SUCCEEDED(prepareResult)) {
                                expected += 'r';
                                expectedLost = FAILED(resetResult);
                            }
                        } else {
                            expected += "cb";
                            expectedLost = false;
                            expectedBegin = true;
                        }
                    }
                    check(begun == expectedBegin, "recovery never returns raw HRESULT as frame success");
                    check(device.order == expected, "recovery order and failure gates");
                    check(lost == expectedLost, "failed recovery retained for retry");
                }
            }
        }
    }
    for (HRESULT endResult : results) {
        for (HRESULT presentResult : results) {
            Device device;
            device.end = endResult;
            bool lost = false;
            const bool ended = device_frame::End(&device, lost,
                [&] { device.order += 'p'; return presentResult; });
            check(ended == (SUCCEEDED(endResult) && SUCCEEDED(presentResult)), "present failures propagated");
            check(device.order == (SUCCEEDED(endResult) ? "ep" : "e"), "no present after failed end");
            const HRESULT executed = FAILED(endResult) ? endResult : presentResult;
            check(lost == (executed == D3DERR_DEVICELOST || executed == D3DERR_DEVICENOTRESET),
                "end and present both signal recovery");
        }
    }
    Device device;
    bool lost = true;
    int resets = 0;
    device.cooperative = D3DERR_DEVICENOTRESET;
    const auto recover = [&](bool restored) {
        return device_frame::Begin(&device, lost, restored,
            []() -> HRESULT { return S_OK; },
            [&]() -> HRESULT { return ++resets == 1 ? E_FAIL : S_OK; },
            []() -> HRESULT { return S_OK; });
    };
    check(!recover(true) && lost && resets == 1, "failed reset skips frame");
    device.cooperative = S_OK;
    check(!recover(false) && !lost && resets == 2, "unrestored resources retried on cooperative device");
    check(recover(true) && resets == 2, "drawing resumes once restoration succeeds");
    bool callback = false;
    check(!device_frame::Begin(static_cast<Device*>(nullptr), lost, true,
        [&]() -> HRESULT { callback = true; return S_OK; },
        [&]() -> HRESULT { callback = true; return S_OK; },
        [&]() -> HRESULT { callback = true; return S_OK; }), "missing begin device rejected");
    check(!device_frame::End(static_cast<Device*>(nullptr), lost,
        [&]() -> HRESULT { callback = true; return S_OK; }), "missing end device rejected");
    check(!callback, "missing device never invokes callbacks");

    for (int result : {0, 1, static_cast<int>(E_FAIL)}) {
        Renderer renderer;
        renderer.lockResult = result;
        {
            device_frame::Scene<Renderer> scene(renderer);
            check(scene.Begin(1) == (result == 1), "scene requires explicit begin success");
        }
        check(renderer.order == (result == 1 ? "Be" : "B"), "early exit closes only opened scene");
    }
    for (int result : {0, 1, static_cast<int>(E_FAIL)}) {
        Renderer renderer;
        renderer.unlockResult = result;
        {
            device_frame::Scene<Renderer> scene(renderer);
            check(!scene.End(0), "cannot end unopened scene");
            check(scene.Begin(1), "first scene opened");
            check(!scene.Begin(0), "cannot begin twice");
            check(scene.End(1) == (result == 1), "scene propagates explicit end failure");
        }
        check(renderer.order == "BP", "failed end is not repeated or followed by another present");
    }
    Renderer renderer;
    {
        device_frame::Scene<Renderer> scene(renderer);
        check(scene.Begin(1) && scene.End(0) && scene.Begin(0) && scene.End(0) &&
            scene.Begin(0) && scene.End(1), "all existing frame segments remain supported");
    }
    check(renderer.order == "BebebP", "multi-scene frame presents exactly once");
    return failures;
}
