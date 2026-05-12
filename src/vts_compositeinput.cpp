/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2026 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file vts_compositeinput.cpp
 */

#include "vts_compositeinput.h"
#include "vts_compositeinput_init.h"
#include <thread>

// ─── CompositeInputEventListener implementations ────────────────────────────

::android::binder::Status CompositeInputEventListener::onStateChanged(
        ::com::rdk::hal::compositeinput::State oldState,
        ::com::rdk::hal::compositeinput::State newState)
{
    {
        std::lock_guard<std::mutex> lock(mStateChangeMutex);
        mStateChangeQueue.emplace(oldState, newState);
    }
    UT_LOG_INFO("CompositeInputEventListener::onStateChanged called with oldState[%d], newState[%d]\n",
                static_cast<int>(oldState), static_cast<int>(newState));
    return ::android::binder::Status::ok();
}

::android::binder::Status CompositeInputEventListener::onPropertyChanged(
        ::com::rdk::hal::compositeinput::PortProperty property,
        const ::com::rdk::hal::PropertyValue& value)
{
    UT_LOG_INFO("CompositeInputEventListener::onPropertyChanged called with property[%d]\n",
                static_cast<int>(property));
    {
        std::lock_guard<std::mutex> lock(mPropertyChangeMutex);
        mPropertyChangeQueue.push({property, value});
    }
    mPropertyChangeCV.notify_all();
    return ::android::binder::Status::ok();
}

bool CompositeInputEventListener::getNextPropertyChange(
        ::com::rdk::hal::compositeinput::PortProperty& property,
        ::com::rdk::hal::PropertyValue& value,
        chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mPropertyChangeMutex);
    if (timeout.count() > 0) {
        if (!mPropertyChangeCV.wait_for(lock, timeout, [this]{ return !mPropertyChangeQueue.empty(); })) {
            return false;
        }
    } else if (mPropertyChangeQueue.empty()) {
        return false;
    }
    property = mPropertyChangeQueue.front().first;
    value    = mPropertyChangeQueue.front().second;
    mPropertyChangeQueue.pop();
    return true;
}

bool CompositeInputEventListener::waitForPropertyCount(int32_t count, chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mPropertyChangeMutex);
    return mPropertyChangeCV.wait_for(lock, timeout,
        [this, count]{ return static_cast<int32_t>(mPropertyChangeQueue.size()) >= count; });
}

void CompositeInputEventListener::clearPropertyQueue()
{
    std::lock_guard<std::mutex> lock(mPropertyChangeMutex);
    std::queue<std::pair<::com::rdk::hal::compositeinput::PortProperty,
                         ::com::rdk::hal::PropertyValue>> emptyQueue;
    std::swap(mPropertyChangeQueue, emptyQueue);
}

bool CompositeInputEventListener::hasPropertyChanged() const
{
    std::lock_guard<std::mutex> lock(mPropertyChangeMutex);
    return !mPropertyChangeQueue.empty();
}

bool CompositeInputEventListener::getNextStateChanges(
        std::pair<::com::rdk::hal::compositeinput::State,
                  ::com::rdk::hal::compositeinput::State>& stateChanges)
{
    std::lock_guard<std::mutex> lock(mStateChangeMutex);
    if (mStateChangeQueue.empty()) {
        return false;
    }
    stateChanges = mStateChangeQueue.front();
    mStateChangeQueue.pop();
    return true;
}

void CompositeInputEventListener::reset()
{
    {
        std::lock_guard<std::mutex> lock(mStateChangeMutex);
        std::queue<std::pair<::com::rdk::hal::compositeinput::State,
                             ::com::rdk::hal::compositeinput::State>> emptyQueue;
        std::swap(mStateChangeQueue, emptyQueue);
    }
    clearPropertyQueue();
}

bool CompositeInputEventListener::hasStateChanged() const
{
    std::lock_guard<std::mutex> lock(mStateChangeMutex);
    return !mStateChangeQueue.empty();
}

// ─── CompositeInputControllerListener implementations ───────────────────────

::android::binder::Status CompositeInputControllerListener::onConnectionChanged(bool connected)
{
    {
        std::lock_guard<std::mutex> lock(mConnectionMutex);
        mConnectionQueue.push(connected);
        m_connected = connected;
        m_connectionChanged = true;
    }
    mConnectionCV.notify_all();
    UT_LOG_INFO("CompositeInputControllerListener::onConnectionChanged called with connected[%s]\n",
                connected ? "true" : "false");
    return ::android::binder::Status::ok();
}

bool CompositeInputControllerListener::getNextConnectionChange(bool& connected, chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mConnectionMutex);
    if (timeout.count() > 0) {
        if (!mConnectionCV.wait_for(lock, timeout, [this]{ return !mConnectionQueue.empty(); })) {
            return false;
        }
    } else if (mConnectionQueue.empty()) {
        return false;
    }
    connected = mConnectionQueue.front();
    mConnectionQueue.pop();
    return true;
}

bool CompositeInputControllerListener::waitForConnectionCount(int32_t count, chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mConnectionMutex);
    return mConnectionCV.wait_for(lock, timeout,
        [this, count]{ return static_cast<int32_t>(mConnectionQueue.size()) >= count; });
}

void CompositeInputControllerListener::clearConnectionQueue()
{
    std::lock_guard<std::mutex> lock(mConnectionMutex);
    std::queue<bool> emptyQueue;
    std::swap(mConnectionQueue, emptyQueue);
}

::android::binder::Status CompositeInputControllerListener::onSignalStatusChanged(
        ::com::rdk::hal::compositeinput::SignalStatus signalStatus)
{
    {
        std::lock_guard<std::mutex> lock(mSignalStatusMutex);
        mSignalStatusQueue.push(signalStatus);
        m_signalStatus = signalStatus;
        m_signalStatusChanged = true;
    }
    mSignalStatusCV.notify_all();
    UT_LOG_INFO("CompositeInputControllerListener::onSignalStatusChanged called with status[%d]\n",
                static_cast<int>(signalStatus));
    return ::android::binder::Status::ok();
}

bool CompositeInputControllerListener::getNextSignalStatusChange(
        ::com::rdk::hal::compositeinput::SignalStatus& status,
        chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mSignalStatusMutex);
    if (timeout.count() > 0) {
        if (!mSignalStatusCV.wait_for(lock, timeout, [this]{ return !mSignalStatusQueue.empty(); })) {
            return false;
        }
    } else if (mSignalStatusQueue.empty()) {
        return false;
    }
    status = mSignalStatusQueue.front();
    mSignalStatusQueue.pop();
    return true;
}

bool CompositeInputControllerListener::waitForSignalStatusCount(int32_t count, chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mSignalStatusMutex);
    return mSignalStatusCV.wait_for(lock, timeout,
        [this, count]{ return static_cast<int32_t>(mSignalStatusQueue.size()) >= count; });
}

void CompositeInputControllerListener::clearSignalStatusQueue()
{
    std::lock_guard<std::mutex> lock(mSignalStatusMutex);
    std::queue<::com::rdk::hal::compositeinput::SignalStatus> emptyQueue;
    std::swap(mSignalStatusQueue, emptyQueue);
}

bool CompositeInputControllerListener::getNextVideoModeChange(
        ::com::rdk::hal::compositeinput::VideoResolution& resolution,
        chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mVideoModeMutex);
    if (timeout.count() > 0) {
        if (!mVideoModeCV.wait_for(lock, timeout, [this]{ return !mVideoModeQueue.empty(); })) {
            return false;
        }
    } else if (mVideoModeQueue.empty()) {
        return false;
    }
    resolution = mVideoModeQueue.front();
    mVideoModeQueue.pop();
    return true;
}

bool CompositeInputControllerListener::waitForVideoModeCount(int32_t count, chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mVideoModeMutex);
    return mVideoModeCV.wait_for(lock, timeout,
        [this, count]{ return static_cast<int32_t>(mVideoModeQueue.size()) >= count; });
}

void CompositeInputControllerListener::clearVideoModeQueue()
{
    std::lock_guard<std::mutex> lock(mVideoModeMutex);
    std::queue<::com::rdk::hal::compositeinput::VideoResolution> emptyQueue;
    std::swap(mVideoModeQueue, emptyQueue);
}

::android::binder::Status CompositeInputControllerListener::onVideoModeChanged(
        const ::com::rdk::hal::compositeinput::VideoResolution& resolution)
{
    {
        std::lock_guard<std::mutex> lock(mVideoModeMutex);
        mVideoModeQueue.push(resolution);
    }
    mVideoModeCV.notify_all();
    UT_LOG_INFO("CompositeInputControllerListener::onVideoModeChanged called\n");
    return ::android::binder::Status::ok();
}

void CompositeInputControllerListener::reset()
{
    clearConnectionQueue();
    clearSignalStatusQueue();
    clearVideoModeQueue();
    {
        std::lock_guard<std::mutex> lock(mConnectionMutex);
        m_connectionChanged = false;
        m_connected = false;
    }
    {
        std::lock_guard<std::mutex> lock(mSignalStatusMutex);
        m_signalStatusChanged = false;
    }
}

bool CompositeInputControllerListener::hasConnectionChanged() const
{
    std::lock_guard<std::mutex> lock(mConnectionMutex);
    return m_connectionChanged;
}

bool CompositeInputControllerListener::hasSignalStatusChanged() const
{
    std::lock_guard<std::mutex> lock(mSignalStatusMutex);
    return m_signalStatusChanged;
}

android::sp<com::rdk::hal::compositeinput::ICompositeInputPort> getCompositeInputPort(int32_t portId)
{
    android::sp<com::rdk::hal::compositeinput::ICompositeInputPort> port;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
    return port;
}

/* End of vts_compositeinput.cpp */
