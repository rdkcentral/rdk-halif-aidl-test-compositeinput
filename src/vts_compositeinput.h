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
 * @addtogroup HPK Hardware Porting Kit
 * @{
 */

/**
 * @addtogroup COMPOSITEINPUT COMPOSITEINPUT Module
 * @{
 */

/**
 * @addtogroup COMPOSITEINPUT_HALTEST COMPOSITEINPUT HAL Tests
 * @{
 */

/**
 * @defgroup COMPOSITEINPUT_HALTEST_CLASS COMPOSITEINPUT HAL Tests Common Classes
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the COMPOSITEINPUT module :
 *
 * Common functions/macros required for the COMPOSITEINPUT module.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_compositeinput.h
 *
 */

#pragma once

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <utility>
#include <ut.h>
#include <ut_kvp_profile.h>
#include <ut_log.h>

#include "com/rdk/hal/compositeinput/ICompositeInputEventListener.h"
#include "com/rdk/hal/compositeinput/ICompositeInputManager.h"
#include "com/rdk/hal/compositeinput/ICompositeInputPort.h"
#include "com/rdk/hal/compositeinput/ICompositeInputController.h"
#include "com/rdk/hal/compositeinput/ICompositeInputControllerListener.h"
#include "com/rdk/hal/compositeinput/BnCompositeInputEventListener.h"
#include "com/rdk/hal/compositeinput/BnCompositeInputControllerListener.h"
#include "vts_compositeinput_helper_functions.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;
using namespace std;
using namespace std::chrono;

using namespace com::rdk::hal;
using namespace com::rdk::hal::compositeinput;

extern sp<ICompositeInputManager> gCOMPOSITEINPUTHandle;

// ─── Convenience macros ────────────────────────────────────────────────────

#define COMPOSITEINPUT_GET_PLATFORM_CAPABILITIES(handle, caps) \
    do { \
        UT_LOG_INFO("Calling getPlatformCapabilities(IN: none, OUT: caps)\n"); \
        ::android::binder::Status status = handle->getPlatformCapabilities(&caps); \
        UT_LOG_INFO("Result getPlatformCapabilities(OUT: caps), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)


#define COMPOSITEINPUT_GET_PORT_IDS(handle, portIds) \
    do { \
        UT_LOG_INFO("Calling getPortIds(OUT: portIds)\n"); \
        ::android::binder::Status status = handle->getPortIds(&portIds); \
        UT_LOG_INFO("Result getPortIds(OUT: portIds count:[%zu]), status:[isOK:%s, exception:%s]\n", \
                    portIds.size(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_PORT(handle, portId, port) \
    do { \
        UT_LOG_INFO("Calling getPort(IN: portId:[%d], OUT: ICompositeInputPort)\n", portId); \
        ::android::binder::Status status = handle->getPort(portId, &port); \
        UT_LOG_INFO("Result getPort(IN: portId:[%d], OUT: ICompositeInputPort[%p]), status:[isOK:%s]\n", \
                    portId, port.get(), status.isOk() ? "true" : "false"); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_PORT_NEG(handle, portId, port, status) \
    do { \
        UT_LOG_INFO("Calling getPort(IN: portId:[%d], OUT: ICompositeInputPort)\n", portId); \
        status = handle->getPort(portId, &port); \
        UT_LOG_INFO("Result getPort(IN: portId:[%d], OUT: ICompositeInputPort[%p]), status:[isOK:%s, exception:%s]\n", \
                    portId, port.get(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_GET_PORT_INFO(port, portInfo) \
    do { \
        UT_LOG_INFO("Calling getPortInfo(IN: none, OUT: portInfo)\n"); \
        ::android::binder::Status status = port->getPortInfo(&portInfo); \
        UT_LOG_INFO("Result getPortInfo(OUT: name:[%s]), status:[isOK:%s, exception:%s]\n", \
                    portInfo.name.c_str(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_ID(port, portId) \
    do { \
        UT_LOG_INFO("Calling getId(IN: none, OUT: portId)\n"); \
        ::android::binder::Status status = port->getId(&portId); \
        UT_LOG_INFO("Result getId(OUT: portId:[%d]), status:[isOK:%s, exception:%s]\n", \
                    portId, \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_CAPABILITIES(port, caps) \
    do { \
        UT_LOG_INFO("Calling getCapabilities(IN: none, OUT: caps)\n"); \
        ::android::binder::Status status = port->getCapabilities(&caps); \
        UT_LOG_INFO("Result getCapabilities(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)


#define COMPOSITEINPUT_GET_STATE(port, state) \
    do { \
        UT_LOG_INFO("Calling getState(IN: none, OUT:_aidl_return)"); \
        ::android::binder::Status status = port->getState(&state); \
        UT_LOG_INFO("Result getState(OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(state), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_STATUS(port, portStatus) \
    do { \
        UT_LOG_INFO("Calling getStatus(IN: none, OUT: portStatus)\n"); \
        ::android::binder::Status status = port->getStatus(&portStatus); \
        UT_LOG_INFO("Result getStatus(OUT: portId:[%d], connected:[%s]), status:[isOK:%s, exception:%s]\n", \
                    portStatus.portId, \
                    portStatus.connected ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_PROPERTY(port, property, propValue) \
    do { \
        UT_LOG_INFO("Calling getProperty(IN: property:[%d], OUT: propValue)\n", static_cast<int32_t>(property)); \
        ::std::optional<::com::rdk::hal::PropertyValue> optionalValue; \
        ::android::binder::Status status = port->getProperty(property, &optionalValue); \
        if (optionalValue.has_value()) { \
            propValue = optionalValue.value(); \
        } \
        UT_LOG_INFO("Result getProperty(IN: property:[%d], OUT: [has value:%s]), status:[isOK:%s, exception:%s]\n", \
                    static_cast<int32_t>(property), \
                    (optionalValue.has_value() ? "true" : "false"), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_PROPERTY_NEG(port, property, propValue, status) \
    do { \
        UT_LOG_INFO("Calling getProperty(IN: property:[%d], OUT: propValue)\n", static_cast<int32_t>(property)); \
        status = port->getProperty(property, &propValue); \
        UT_LOG_INFO("Result getProperty(IN: property:[%d], OUT: [has value:%s]), status:[isOK:%s, exception:%s]\n", \
                    static_cast<int32_t>(property), \
                    (propValue.has_value() ? "true" : "false"), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_GET_PROPERTY_MULTI(port, properties, kvPairs) \
    do { \
        UT_LOG_INFO("Calling getPropertyMulti(IN: properties count:[%zu], OUT: kvPairs)\n", properties.size()); \
        ::android::binder::Status status = port->getPropertyMulti(properties, &kvPairs); \
        UT_LOG_INFO("Result getPropertyMulti(OUT: kvPairs count:[%zu]), status:[isOK:%s, exception:%s]\n", \
                    kvPairs.size(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_GET_PROPERTY_MULTI_NEG(port, properties, kvPairs, out_status) \
    do { \
        UT_LOG_INFO("Calling getPropertyMulti(IN: properties count:[%zu], OUT: kvPairs) [negative]\n", properties.size()); \
        out_status = port->getPropertyMulti(properties, &kvPairs); \
        UT_LOG_INFO("Result getPropertyMulti(OUT: kvPairs count:[%zu]), status:[isOK:%s, exception:%s]\n", \
                    kvPairs.size(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, out_status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, out_status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_REGISTER_EVENT_LISTENER(port, listener) \
    do { \
        UT_LOG_INFO("Calling registerEventListener(IN: listener)\n"); \
        ::android::binder::Status status = port->registerEventListener(listener); \
        UT_LOG_INFO("Result registerEventListener(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_UNREGISTER_EVENT_LISTENER(port, listener) \
    do { \
        UT_LOG_INFO("Calling unregisterEventListener(IN: listener)\n"); \
        ::android::binder::Status status = port->unregisterEventListener(listener); \
        UT_LOG_INFO("Result unregisterEventListener(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_REGISTER_EVENT_LISTENER_NEG(port, listener, status) \
    do { \
        UT_LOG_INFO("Calling registerEventListener(IN: listener)\n"); \
        status = port->registerEventListener(listener); \
        UT_LOG_INFO("Result registerEventListener(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_UNREGISTER_EVENT_LISTENER_NEG(port, listener, status) \
    do { \
        UT_LOG_INFO("Calling unregisterEventListener(IN: listener)\n"); \
        status = port->unregisterEventListener(listener); \
        UT_LOG_INFO("Result unregisterEventListener(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_OPEN(port, listener, controller) \
    do { \
        UT_LOG_INFO("Calling open(IN: listener, OUT: ICompositeInputController)\n"); \
        ::android::binder::Status status = port->open(listener, &controller); \
        UT_LOG_INFO("Result open(IN: listener, OUT: ICompositeInputController[%p]), status:[isOK:%s]\n", \
                    controller.get(), status.isOk() ? "true" : "false"); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_OPEN_NEG(port, listener, controller, status) \
    do { \
        UT_LOG_INFO("Calling open(IN: listener, OUT: ICompositeInputController)\n"); \
        status = port->open(listener, &controller); \
        UT_LOG_INFO("Result open(IN: listener, OUT: ICompositeInputController[%p]), status:[isOK:%s, exception:%s]\n", \
                    controller.get(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_CLOSE(port, controller, aidl_return) \
    do { \
        UT_LOG_INFO("Calling close(IN: controller, OUT:_aidl_return)\n"); \
        ::android::binder::Status status = port->close(controller, &aidl_return); \
        UT_LOG_INFO("Result close(OUT:_aidl_return [%s]), status:[isOK:%s, exception:%s]\n", \
                    aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_CLOSE_NEG(port, controller, aidl_return, out_status) \
    do { \
        UT_LOG_INFO("Calling close(IN: controller, OUT:_aidl_return) [negative]\n"); \
        out_status = port->close(controller, &aidl_return); \
        UT_LOG_INFO("Result close(OUT:_aidl_return [%s]), status:[isOK:%s, exception:%s]\n", \
                    aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, out_status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, out_status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_START(controller) \
    do { \
        UT_LOG_INFO("Calling start()\n"); \
        ::android::binder::Status status = controller->start(); \
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_START_NEG(controller, out_status) \
    do { \
        UT_LOG_INFO("Calling start() [negative]\n"); \
        out_status = controller->start(); \
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, out_status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, out_status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_STOP(controller) \
    do { \
        UT_LOG_INFO("Calling stop()\n"); \
        ::android::binder::Status status = controller->stop(); \
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_STOP_NEG(controller, out_status) \
    do { \
        UT_LOG_INFO("Calling stop() [negative]\n"); \
        out_status = controller->stop(); \
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, out_status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, out_status.exceptionCode())); \
    } while (0)


#define COMPOSITEINPUT_RESET_METRICS(controller) \
    do { \
        UT_LOG_INFO("Calling resetMetrics()\n"); \
        ::android::binder::Status status = controller->resetMetrics(); \
        UT_LOG_INFO("Result resetMetrics(), status:[isOK:%s, exception:%s]\n", \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_SET_PROPERTY(controller, property, value) \
    do { \
        UT_LOG_INFO("Calling setProperty(IN: property:[%d])\n", static_cast<int32_t>(property)); \
        ::android::binder::Status status = controller->setProperty(property, value); \
        UT_LOG_INFO("Result setProperty(IN: property:[%d]), status:[isOK:%s, exception:%s]\n", \
                    static_cast<int32_t>(property), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_SET_PROPERTY_NEG(controller, property, value, status) \
    do { \
        UT_LOG_INFO("Calling setProperty(IN: property:[%d])\n", static_cast<int32_t>(property)); \
        status = controller->setProperty(property, value); \
        UT_LOG_INFO("Result setProperty(IN: property:[%d]), status:[isOK:%s, exception:%s]\n", \
                    static_cast<int32_t>(property), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
    } while (0)

#define COMPOSITEINPUT_SET_PROPERTY_MULTI(controller, properties) \
    do { \
        UT_LOG_INFO("Calling setPropertyMulti(IN: properties count:[%zu])\n", properties.size()); \
        ::android::binder::Status status = controller->setPropertyMulti(properties); \
        UT_LOG_INFO("Result setPropertyMulti(IN: properties count:[%zu]), status:[isOK:%s, exception:%s]\n", \
                    properties.size(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define COMPOSITEINPUT_SET_PROPERTY_MULTI_NEG(controller, properties, status) \
    do { \
        UT_LOG_INFO("Calling setPropertyMulti(IN: properties count:[%zu])\n", properties.size()); \
        status = controller->setPropertyMulti(properties); \
        UT_LOG_INFO("Result setPropertyMulti(IN: properties count:[%zu]), status:[isOK:%s, exception:%s]\n", \
                    properties.size(), \
                    UT_Control_GetMapString(bool_mapTable_compositeinput, status.isOk()), \
                    UT_Control_GetMapString(statusCompositeInputException_mapTable, status.exceptionCode())); \
    } while (0)

// ─── Event listener classes ─────────────────────────────────────────────────

class CompositeInputEventListener
        : public com::rdk::hal::compositeinput::BnCompositeInputEventListener {
public:
    CompositeInputEventListener() = default;
    virtual ~CompositeInputEventListener() = default;

    ::android::binder::Status onStateChanged(
            ::com::rdk::hal::compositeinput::State oldState,
            ::com::rdk::hal::compositeinput::State newState) override;

    ::android::binder::Status onPropertyChanged(
            ::com::rdk::hal::compositeinput::PortProperty property,
            const ::com::rdk::hal::PropertyValue& value) override;

    bool getNextStateChanges(std::pair<::com::rdk::hal::compositeinput::State,
                                       ::com::rdk::hal::compositeinput::State>& stateChanges);
    bool getNextPropertyChange(::com::rdk::hal::compositeinput::PortProperty& property,
                               ::com::rdk::hal::PropertyValue& value,
                               chrono::milliseconds timeout = chrono::milliseconds(0));
    bool waitForPropertyCount(int32_t count, chrono::milliseconds timeout);
    void clearPropertyQueue();
    void reset();
    bool hasStateChanged() const;
    bool hasPropertyChanged() const;

private:
    mutable std::mutex mStateChangeMutex;
    std::queue<std::pair<::com::rdk::hal::compositeinput::State,
                         ::com::rdk::hal::compositeinput::State>> mStateChangeQueue;

    mutable std::mutex mPropertyChangeMutex;
    std::condition_variable mPropertyChangeCV;
    std::queue<std::pair<::com::rdk::hal::compositeinput::PortProperty,
                         ::com::rdk::hal::PropertyValue>> mPropertyChangeQueue;
};

class CompositeInputControllerListener
        : public com::rdk::hal::compositeinput::BnCompositeInputControllerListener {
public:
    CompositeInputControllerListener() = default;
    virtual ~CompositeInputControllerListener() = default;

    ::android::binder::Status onConnectionChanged(bool connected) override;
    ::android::binder::Status onSignalStatusChanged(
            ::com::rdk::hal::compositeinput::SignalStatus signalStatus) override;
    ::android::binder::Status onVideoModeChanged(
            const ::com::rdk::hal::compositeinput::VideoResolution& resolution) override;

    bool getNextConnectionChange(bool& connected, chrono::milliseconds timeout = chrono::milliseconds(0));
    bool getNextSignalStatusChange(::com::rdk::hal::compositeinput::SignalStatus& status,
                                   chrono::milliseconds timeout = chrono::milliseconds(0));
    bool getNextVideoModeChange(::com::rdk::hal::compositeinput::VideoResolution& resolution,
                                chrono::milliseconds timeout = chrono::milliseconds(0));
    bool waitForConnectionCount(int32_t count, chrono::milliseconds timeout);
    bool waitForSignalStatusCount(int32_t count, chrono::milliseconds timeout);
    bool waitForVideoModeCount(int32_t count, chrono::milliseconds timeout);

    void reset();
    void clearConnectionQueue();
    void clearSignalStatusQueue();
    void clearVideoModeQueue();
    bool hasConnectionChanged() const;
    bool hasSignalStatusChanged() const;

private:
    mutable std::mutex mConnectionMutex;
    std::condition_variable mConnectionCV;
    std::queue<bool> mConnectionQueue;

    mutable std::mutex mSignalStatusMutex;
    std::condition_variable mSignalStatusCV;
    std::queue<::com::rdk::hal::compositeinput::SignalStatus> mSignalStatusQueue;

    mutable std::mutex mVideoModeMutex;
    std::condition_variable mVideoModeCV;
    std::queue<::com::rdk::hal::compositeinput::VideoResolution> mVideoModeQueue;

    bool m_connectionChanged{false};
    bool m_connected{false};
    bool m_signalStatusChanged{false};
    ::com::rdk::hal::compositeinput::SignalStatus m_signalStatus{::com::rdk::hal::compositeinput::SignalStatus::NO_SIGNAL};
};

android::sp<com::rdk::hal::compositeinput::ICompositeInputPort> getCompositeInputPort(int32_t portId);

/**
 * @} // End of COMPOSITEINPUT_HALTEST_CLASS
 * @} // End of COMPOSITEINPUT_HALTEST
 * @} // End of COMPOSITEINPUT
 * @} // End of HPK
 */
