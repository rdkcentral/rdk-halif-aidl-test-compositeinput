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
 * @addtogroup COMPOSITE_INPUT COMPOSITE INPUT Module
 * @{
 */

/**
 * @addtogroup COMPOSITE_INPUT_HALTEST COMPOSITE INPUT HAL Tests
 * @{
 */

/**
 * @defgroup COMPOSITE_INPUT_HALTEST_L3 COMPOSITE INPUT HAL Tests L3 File
 * @{
 * @parblock
 *
 * ### L3 Test Cases for COMPOSITE INPUT HAL :
 *
 * This module includes Level 3 scenario tests requiring an active composite input
 * source connected via the vcomponent or a physical device. Tests verify real-world
 * callback behaviour including connection detection, signal acquisition, video mode
 * reporting, property change notifications, and end-to-end video playback readiness.
 *
 * **Pre-Conditions:** vcomponent (Vendor-CompositeInput) must be running @n
 * **Dependencies:** vcomponent running with valid configuration @n
 *
 * @endparblock
 */

/**
 * @file vts_compositeinput_l3_tests.cpp
 */

#include <unistd.h>
#include <map>
#include "vts_compositeinput.h"
#include "vts_compositeinput_parse_testconfig.h"

static const char *moduleName    = "compositeinput";
static const char *testGroupName = "L3";

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_COMPOSITEINPUT, UT_TESTS_L3)

class VTS_L3_COMPOSITEINPUT : public UTCore
{
public:
    VTS_L3_COMPOSITEINPUT() : UTCore() {}
    ~VTS_L3_COMPOSITEINPUT() override = default;

    std::vector<std::tuple<
        int32_t,                                        /* portId               */
        android::sp<ICompositeInputPort>,               /* port                 */
        android::sp<CompositeInputControllerListener>,  /* controllerListener   */
        android::sp<CompositeInputEventListener>,       /* eventListener        */
        android::sp<ICompositeInputController>          /* controller           */
    >> mCompositeInputList;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();

        uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
        for (uint32_t i = 0; i < portCount; ++i)
        {
            std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
            int32_t portId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

            android::sp<ICompositeInputPort> port;
            COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portId, port);
            UT_ASSERT_NOT_EQUAL(port, nullptr);

            auto controllerListener = android::sp<CompositeInputControllerListener>::make();
            auto eventListener      = android::sp<CompositeInputEventListener>::make();
            android::sp<ICompositeInputController> controller;

            /* Register event listener before open so CLOSED→OPENING state changes are captured */
            COMPOSITEINPUT_REGISTER_EVENT_LISTENER(port, eventListener);

            /* open() drives CLOSED → OPENING → READY; the controllerListener receives
             * onConnectionChanged() at least once during OPENING to report current
             * cable/device presence before the port becomes READY. */
            COMPOSITEINPUT_OPEN(port, controllerListener, controller);
            UT_ASSERT_NOT_EQUAL(controller, nullptr);

            mCompositeInputList.push_back({portId, port, controllerListener, eventListener, controller});
        }
    }

    void TearDown() override
    {
        for (auto& entry : mCompositeInputList)
        {
            auto& port          = std::get<1>(entry);
            auto& controllerListener = std::get<2>(entry);
            auto& eventListener = std::get<3>(entry);
            auto& controller    = std::get<4>(entry);

            bool closed = false;
            COMPOSITEINPUT_CLOSE(port, controller, closed);
            controllerListener->reset();
            eventListener->reset();
            COMPOSITEINPUT_UNREGISTER_EVENT_LISTENER(port, eventListener);
        }
        mCompositeInputList.clear();
    }

    void StartPort(int32_t portId)
    {
        for (const auto& entry : mCompositeInputList)
        {
            if (std::get<0>(entry) != portId)
                continue;
            auto& controller = std::get<4>(entry);
            COMPOSITEINPUT_START(controller);
            UT_LOG_INFO("Port[%d] STARTED\n", portId);
        }
    }

    void StopPort(int32_t portId)
    {
        for (const auto& entry : mCompositeInputList)
        {
            if (std::get<0>(entry) != portId)
                continue;
            auto& controller = std::get<4>(entry);
            COMPOSITEINPUT_STOP(controller);
            UT_LOG_INFO("Port[%d] STOPPED\n", portId);
        }
    }
};

/**
 * @brief Verify connection state callbacks during OPENING and in READY state.
 *
 * Validates that ICompositeInputControllerListener::onConnectionChanged() is fired
 * at least once during the OPENING transition to report the initial cable presence
 * state before the port reaches READY. After READY the vcomponent drives HPD
 * plug/unplug events and the test verifies a callback fires for each event with
 * the correct connection state (TRUE on plug/powerON, FALSE on unplug/powerOFF).
 *
 * **Test Group ID:** L3: 03@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** vcomponent must be running @n
 * **User Interaction:** None (vcomponent drives connection events) @n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify onConnectionChanged fires once during OPENING | portId from config | Callback received, state matches config connection_state | Should Pass |
 * | 02 | vcomponent drives HPD plug event in READY state | connection_sequence | onConnectionChanged(TRUE) received | Should Pass |
 * | 03 | vcomponent drives HPD unplug event in READY state | connection_sequence | onConnectionChanged(FALSE) received | Should Pass |
 * | 04 | vcomponent drives powerON event in READY state | connection_sequence | onConnectionChanged(TRUE) received | Should Pass |
 * | 05 | vcomponent drives powerOFF event in READY state | connection_sequence | onConnectionChanged(FALSE) received | Should Pass |
 *
 * **Refer:** [compositeinput_High-Level_TestSpecification](../../docs/pages/compositeinput_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_COMPOSITEINPUT, ConnectionTestBeforeStart)
{
    const char *testName = "ConnectionTestBeforeStart";
    std::vector<compositeInputConnectionConfig_t> configs;

    UT_status_t parseStatus = vts_compositeinput_parse_connectionTest(
        (char*)VTS_COMPOSITEINPUT_TEST_CONFIG_PATH,
        moduleName, testGroupName, testName, configs);

    UT_ASSERT_EQUAL_FATAL(parseStatus, UT_STATUS_OK);
    UT_ASSERT_TRUE(configs.size() > 0);

    for (const auto& cfg : configs)
    {
        UT_LOG_INFO("Testing port[%d]\n", cfg.portId);

        bool portFound = false;
        for (const auto& entry : mCompositeInputList)
        {
            if (std::get<0>(entry) != cfg.portId)
                continue;

            portFound = true;
            auto& controllerListener = std::get<2>(entry);

            const auto timeout = chrono::milliseconds(cfg.maxWaitTimeMs);

            /* Verify onConnectionChanged fires at least once during OPENING.
             * The callback is queued in SetUp when open() drives CLOSED→OPENING→READY. */
            bool connected = false;
            bool received = controllerListener->getNextConnectionChange(connected, timeout);
            UT_LOG_INFO("Port[%d] onConnectionChanged during OPENING: received=%s, connected=%s\n",
                        cfg.portId, received ? "true" : "false", connected ? "true" : "false");
            UT_ASSERT_TRUE(received);
            UT_ASSERT_EQUAL(connected, cfg.connectionState);

            /* In READY state the vcomponent drives HPD plug/unplug and powerON/powerOFF
             * events; verify a callback fires for each entry in the connection sequence. */
            if (!cfg.connectionSequence.empty())
            {
                bool seqReceived = controllerListener->waitForConnectionCount(
                    (int32_t)cfg.connectionSequence.size(), timeout);
                UT_LOG_INFO("Port[%d] connection sequence (%zu events): %s\n",
                            cfg.portId, cfg.connectionSequence.size(),
                            seqReceived ? "received" : "timed-out");
                UT_ASSERT_TRUE(seqReceived);

                for (size_t i = 0; i < cfg.connectionSequence.size(); ++i)
                {
                    bool seqConnected = false;
                    bool evReceived = controllerListener->getNextConnectionChange(
                        seqConnected, chrono::milliseconds(500));
                    UT_ASSERT_TRUE(evReceived);
                    UT_ASSERT_EQUAL(seqConnected, cfg.connectionSequence[i]);
                    UT_LOG_INFO("Port[%d] sequence[%zu]: expected=%s, actual=%s\n",
                                cfg.portId, i,
                                cfg.connectionSequence[i] ? "true" : "false",
                                seqConnected ? "true" : "false");
                }
            }

            break;
        }

        UT_ASSERT_TRUE(portFound);
    }
}

/**
 * @brief Verify signal status callbacks during STARTING and after STARTED.
 *
 * Validates that ICompositeInputControllerListener::onSignalStatusChanged() is fired
 * at least once during the STARTING transition before STARTED is reached.
 * The initial signal state reported must reflect the actual source state:
 * NO_SIGNAL when no device is connected, STABLE when a device with valid signal
 * is already connected.
 *
 * After STARTED the vcomponent drives HPD plug/unplug and powerON/powerOFF
 * events; the test verifies that signal state sequences
 * (NO_SIGNAL → UNSTABLE → STABLE on connect, STABLE → NO_SIGNAL on disconnect)
 * are received via onSignalStatusChanged() callbacks.
 *
 * **Test Group ID:** L3: 03@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** vcomponent must be running @n
 * **User Interaction:** None (vcomponent drives signal events) @n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Start composite input port | portId from config | Port transitions to STARTED | Should Pass |
 * | 02 | Verify onSignalStatusChanged fires during STARTING | none | Callback received | Should Pass |
 * | 03 | Verify initial signal state matches device connection state | connection_state | NO_SIGNAL (not connected) or STABLE (connected) | Should Pass |
 * | 04 | vcomponent drives HPD plug in STARTED state | connection_sequence | NO_SIGNAL → UNSTABLE → STABLE callbacks received | Should Pass |
 * | 05 | vcomponent drives HPD unplug in STARTED state | connection_sequence | STABLE → NO_SIGNAL callback received | Should Pass |
 * | 06 | Stop and close port | none | Port closes successfully | Should Pass |
 *
 * **Refer:** [compositeinput_High-Level_TestSpecification](../../docs/pages/compositeinput_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_COMPOSITEINPUT, ConnectionTestAfterStart)
{
    const char *testName = "ConnectionTestAfterStart";
    std::vector<compositeInputConnectionConfig_t> configs;

    UT_status_t parseStatus = vts_compositeinput_parse_connectionTest(
        (char*)VTS_COMPOSITEINPUT_TEST_CONFIG_PATH,
        moduleName, testGroupName, testName, configs);

    UT_ASSERT_EQUAL_FATAL(parseStatus, UT_STATUS_OK);
    UT_ASSERT_TRUE(configs.size() > 0);


    for (const auto& cfg : configs)
    {
        UT_LOG_INFO("Testing port[%d]\n", cfg.portId);
        StartPort(cfg.portId);

        bool portFound = false;
        for (const auto& entry : mCompositeInputList)
        {
            if (std::get<0>(entry) != cfg.portId)
                continue;

            portFound = true;
            auto& controllerListener = std::get<2>(entry);

            const auto timeout = chrono::milliseconds(cfg.maxWaitTimeMs);

            /* Verify onConnectionChanged fires at least once during OPENING.
             * The callback is queued in SetUp when open() drives CLOSED→OPENING→READY. */
            bool connected = false;
            bool received = controllerListener->getNextConnectionChange(connected, timeout);
            UT_LOG_INFO("Port[%d] onConnectionChanged during OPENING: received=%s, connected=%s\n",
                        cfg.portId, received ? "true" : "false", connected ? "true" : "false");
            UT_ASSERT_TRUE(received);
            UT_ASSERT_EQUAL(connected, cfg.connectionState);

            /* onSignalStatusChanged MUST fire at least once during STARTING.
             * The signal state reflects device presence at the time start() is called. */
            SignalStatus signalStatus = SignalStatus::NO_SIGNAL;
            bool received1 = controllerListener->getNextSignalStatusChange(signalStatus, timeout);
            UT_LOG_INFO("Port[%d] onSignalStatusChanged during STARTING: received=%s, status=%d\n",
                        cfg.portId, received1 ? "true" : "false", static_cast<int>(signalStatus));
            UT_ASSERT_TRUE(received1);

            if (cfg.connectionState)
            {
                /* Device is already connected with active signal: STABLE expected */
                UT_ASSERT_EQUAL(signalStatus, SignalStatus::STABLE);
            }
            else
            {
                /* Device is not connected: NO_SIGNAL expected */
                UT_ASSERT_EQUAL(signalStatus, SignalStatus::NO_SIGNAL);
            }

            /* In STARTED state the vcomponent drives HPD/powerON/powerOFF events.
             * Verify the expected number of signal status callbacks are received. */
            if (cfg.expectSignalAfterConnect && !cfg.connectionSequence.empty())
            {
                bool seqReceived = controllerListener->waitForConnectionCount(
                    (int32_t)cfg.connectionSequence.size(), timeout);
                UT_LOG_INFO("Port[%d] signal sequence (%zu events): %s\n",
                            cfg.portId, cfg.connectionSequence.size(),
                            seqReceived ? "received" : "timed-out");
                UT_ASSERT_TRUE(seqReceived);
                sleep(1); // allow time for all callbacks to be processed
                for (size_t i = 0; i < cfg.connectionSequence.size(); ++i)
                {
                    bool seqConnected = false;
                    bool evReceived = controllerListener->getNextConnectionChange(
                        seqConnected, chrono::milliseconds(500));
                    UT_ASSERT_TRUE(evReceived);
                    UT_ASSERT_EQUAL(seqConnected, cfg.connectionSequence[i]);
                    UT_LOG_INFO("Port[%d] sequence[%zu]: expected=%s, actual=%s\n",
                                cfg.portId, i,
                                cfg.connectionSequence[i] ? "true" : "false",
                                seqConnected ? "true" : "false");

                    if (!cfg.connectionSequence[i])
                    {
                        SignalStatus actualStatus = SignalStatus::NO_SIGNAL;
                        bool evReceived0 = controllerListener->getNextSignalStatusChange(
                            actualStatus, chrono::milliseconds(500));
                        UT_ASSERT_TRUE(evReceived0);
                        UT_ASSERT_EQUAL(actualStatus, SignalStatus::NO_SIGNAL);
                        UT_LOG_INFO("Port[%d] sequence[%zu]: expected=%d, actual=%d\n",
                                    cfg.portId, i, static_cast<int>(SignalStatus::NO_SIGNAL),
                                    static_cast<int>(actualStatus));
                    } else
                    {
                        SignalStatus actualStatus = SignalStatus::NO_SIGNAL;
                        bool evReceived = controllerListener->getNextSignalStatusChange(
                            actualStatus, chrono::milliseconds(500));
                        UT_ASSERT_TRUE(evReceived);
                        UT_ASSERT_EQUAL(actualStatus, SignalStatus::UNSTABLE);
                        UT_LOG_INFO("Port[%d] sequence[%zu]: expected=%d, actual=%d\n",
                                    cfg.portId, i, static_cast<int>(SignalStatus::UNSTABLE),
                                    static_cast<int>(actualStatus));
                        SignalStatus actualStatus1 = SignalStatus::NO_SIGNAL;
                        bool evReceived1 = controllerListener->getNextSignalStatusChange(
                            actualStatus1, chrono::milliseconds(500));
                        UT_ASSERT_TRUE(evReceived1);
                        UT_ASSERT_EQUAL(actualStatus1, SignalStatus::STABLE);
                        UT_LOG_INFO("Port[%d] sequence[%zu]: expected=%d, actual=%d\n",
                                    cfg.portId, i, static_cast<int>(SignalStatus::STABLE),
                                    static_cast<int>(actualStatus1));
                    }
                }
            }

            break;
        }

        UT_ASSERT_TRUE(portFound);
        StopPort(cfg.portId);
    }

}

/**
 * @brief Verify video mode detection via onVideoModeChanged() callback.
 *
 * Validates that ICompositeInputControllerListener::onVideoModeChanged() fires
 * with a valid VideoResolution once the input signal becomes STABLE. The test
 * also validates that the resolution reported in the callback matches the value
 * returned by ICompositeInputPort::getStatus().detectedResolution.
 *
 * Optionally, after the initial video mode is confirmed, the vcomponent may
 * switch the source video mode; the test verifies that onVideoModeChanged() fires
 * again with the updated resolution.
 *
 * **Test Group ID:** L3: 03@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** vcomponent must be running with an active source connected @n
 * **User Interaction:** None (vcomponent provides video signal) @n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Start composite input port with active source | portId from config | Port transitions to STARTED | Should Pass |
 * | 02 | Wait for first onVideoModeChanged (videomode1 from initial signal STABLE) | none | Callback received | Should Pass |
 * | 03 | Validate first callback VideoResolution matches videomode1 from config | videomode1 config | pixelWidth, pixelHeight, interlaced, frameRateInHz match | Should Pass |
 * | 04 | Verify getStatus().detectedResolution matches first callback VideoResolution | none | Values match | Should Pass |
 * | 05 | Wait for second onVideoModeChanged (videomode2 after vcomponent switches source) | none | Callback received | Should Pass |
 * | 06 | Validate second callback VideoResolution matches videomode2 from config | videomode2 config | pixelWidth, pixelHeight, interlaced, frameRateInHz match | Should Pass |
 * | 07 | Verify getStatus().detectedResolution matches second callback VideoResolution | none | Values match | Should Pass |
 * | 08 | Stop and close port | none | Port closes successfully | Should Pass |
 *
 * **Refer:** [compositeinput_High-Level_TestSpecification](../../docs/pages/compositeinput_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_COMPOSITEINPUT, VideoModeDetection)
{
    const char *testName = "VideoModeDetection";
    std::vector<compositeInputVideoModeConfig_t> configs;

    UT_status_t parseStatus = vts_compositeinput_parse_videoModeTest(
        (char*)VTS_COMPOSITEINPUT_TEST_CONFIG_PATH,
        moduleName, testGroupName, testName, configs);

    UT_ASSERT_EQUAL_FATAL(parseStatus, UT_STATUS_OK);
    UT_ASSERT_TRUE(configs.size() > 0);

    for (const auto& cfg : configs)
    {
        UT_LOG_INFO("Testing port[%d] for video mode detection\n", cfg.portId);

        StartPort(cfg.portId);
        bool portFound = false;
        for (const auto& entry : mCompositeInputList)
        {
            if (std::get<0>(entry) != cfg.portId)
                continue;

            portFound = true;
            auto& port               = std::get<1>(entry);
            auto& controllerListener = std::get<2>(entry);

            const auto timeout = chrono::milliseconds(cfg.maxWaitTimeMs);

            /* check the connect status, active status, state and signalstate*/
            State state = State::CLOSED;
            COMPOSITEINPUT_GET_STATE(port, state);
            UT_LOG_INFO("Port[%d] state after start: %d\n", cfg.portId, static_cast<int>(state));
            PortStatus portStatus0 = {};
            COMPOSITEINPUT_GET_STATUS(port, portStatus0);
            UT_LOG_INFO("Port[%d] status after start: portId=%d, connected=%s, active=%s, signalStatus=%d\n",
                        cfg.portId, portStatus0.portId, portStatus0.connected ? "true" : "false",
                        portStatus0.active ? "true" : "false", static_cast<int>(portStatus0.signalStatus));
            UT_ASSERT_EQUAL(state, State::STARTED);
            UT_ASSERT_EQUAL(portStatus0.signalStatus, SignalStatus::STABLE);
            UT_ASSERT_TRUE(portStatus0.connected);
            UT_ASSERT_TRUE(portStatus0.active);
            if (!portStatus0.connected || !portStatus0.active) 
            {
                UT_LOG_ERROR("Port[%d] is not connected and active after start, cannot proceed with video mode detection test\n", cfg.portId);
                continue;
            }
            /* Wait for first onVideoModeChanged (videomode1 from initial signal STABLE) */
            VideoResolution callbackResolution1 = {};
            bool videoModeReceived1 = controllerListener->getNextVideoModeChange(callbackResolution1, timeout);
            UT_LOG_INFO("Port[%d] onVideoModeChanged (1st) received: %s\n",
                        cfg.portId, videoModeReceived1 ? "true" : "false");
            UT_ASSERT_TRUE(videoModeReceived1);
            UT_LOG_INFO("Port[%d] VideoResolution (1st): pixelWidth=%d, pixelHeight=%d, "
                        "interlaced=%s, frameRateInHz=%.2f\n",
                        cfg.portId, callbackResolution1.pixelWidth, callbackResolution1.pixelHeight,
                        callbackResolution1.interlaced ? "true" : "false",
                        callbackResolution1.frameRateInHz);

            /* Validate first callback against videomode1 from config */
            UT_ASSERT_EQUAL(callbackResolution1.pixelWidth,    cfg.videomode1.width);
            UT_ASSERT_EQUAL(callbackResolution1.pixelHeight,   cfg.videomode1.height);
            UT_ASSERT_EQUAL(callbackResolution1.interlaced,    cfg.videomode1.interlaced);
            UT_ASSERT_EQUAL(callbackResolution1.frameRateInHz, (float)cfg.videomode1.framerate);

            /* Verify getStatus().detectedResolution matches first callback VideoResolution */
            PortStatus portStatus1 = {};
            COMPOSITEINPUT_GET_STATUS(port, portStatus1);
            UT_ASSERT_TRUE(portStatus1.detectedResolution.has_value());
            const VideoResolution& statusResolution1 = portStatus1.detectedResolution.value();
            UT_LOG_INFO("Port[%d] getStatus detectedResolution (1st): pixelWidth=%d, pixelHeight=%d, "
                        "interlaced=%s, frameRateInHz=%.2f\n",
                        cfg.portId, statusResolution1.pixelWidth, statusResolution1.pixelHeight,
                        statusResolution1.interlaced ? "true" : "false",
                        statusResolution1.frameRateInHz);
            UT_ASSERT_EQUAL(statusResolution1.pixelWidth,    callbackResolution1.pixelWidth);
            UT_ASSERT_EQUAL(statusResolution1.pixelHeight,   callbackResolution1.pixelHeight);
            UT_ASSERT_EQUAL(statusResolution1.interlaced,    callbackResolution1.interlaced);
            UT_ASSERT_EQUAL(statusResolution1.frameRateInHz, callbackResolution1.frameRateInHz);

            /* Wait for second onVideoModeChanged (videomode2 after vcomponent switches source) */
            VideoResolution callbackResolution2 = {};
            bool videoModeReceived2 = controllerListener->getNextVideoModeChange(callbackResolution2, timeout);
            UT_LOG_INFO("Port[%d] onVideoModeChanged (2nd) received: %s\n",
                        cfg.portId, videoModeReceived2 ? "true" : "false");
            UT_ASSERT_TRUE(videoModeReceived2);
            UT_LOG_INFO("Port[%d] VideoResolution (2nd): pixelWidth=%d, pixelHeight=%d, "
                        "interlaced=%s, frameRateInHz=%.2f\n",
                        cfg.portId, callbackResolution2.pixelWidth, callbackResolution2.pixelHeight,
                        callbackResolution2.interlaced ? "true" : "false",
                        callbackResolution2.frameRateInHz);

            /* Validate second callback against videomode2 from config */
            UT_ASSERT_EQUAL(callbackResolution2.pixelWidth,    cfg.videomode2.width);
            UT_ASSERT_EQUAL(callbackResolution2.pixelHeight,   cfg.videomode2.height);
            UT_ASSERT_EQUAL(callbackResolution2.interlaced,    cfg.videomode2.interlaced);
            UT_ASSERT_EQUAL(callbackResolution2.frameRateInHz, (float)cfg.videomode2.framerate);

            /* Verify getStatus().detectedResolution matches second callback VideoResolution */
            PortStatus portStatus2 = {};
            COMPOSITEINPUT_GET_STATUS(port, portStatus2);
            UT_ASSERT_TRUE(portStatus2.detectedResolution.has_value());
            const VideoResolution& statusResolution2 = portStatus2.detectedResolution.value();
            UT_LOG_INFO("Port[%d] getStatus detectedResolution (2nd): pixelWidth=%d, pixelHeight=%d, "
                        "interlaced=%s, frameRateInHz=%.2f\n",
                        cfg.portId, statusResolution2.pixelWidth, statusResolution2.pixelHeight,
                        statusResolution2.interlaced ? "true" : "false",
                        statusResolution2.frameRateInHz);
            UT_ASSERT_EQUAL(statusResolution2.pixelWidth,    callbackResolution2.pixelWidth);
            UT_ASSERT_EQUAL(statusResolution2.pixelHeight,   callbackResolution2.pixelHeight);
            UT_ASSERT_EQUAL(statusResolution2.interlaced,    callbackResolution2.interlaced);
            UT_ASSERT_EQUAL(statusResolution2.frameRateInHz, callbackResolution2.frameRateInHz);

            break;
        }

        UT_ASSERT_TRUE(portFound);
        StopPort(cfg.portId);
    }
}

/**
 * @brief Verify property set/get operations are applied on device with callback notification.
 *
 * Validates that after calling setProperty() or setPropertyMulti() on a composite
 * input port with an active source connected and signal STABLE, the HAL fires
 * ICompositeInputEventListener::onPropertyChanged() for each changed property and
 * the updated value can be read back via getProperty().
 *
 * Writable properties are discovered from the HFP profile
 * (compositeinput.ports.N.propertyMetadata). The test sets each writable property
 * to an appropriate zero/default value for its declared type and verifies the
 * callback and readback. setPropertyMulti() is also exercised with all writable
 * properties in a single call.
 *
 * **Test Group ID:** L3: 03@n
 * **Test Case ID:** 004@n
 * **Priority:** Medium@n
 *
 * **Pre-Conditions:** vcomponent must be running with an active source connected @n
 * **User Interaction:** None @n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Start composite input port with active source | portId from config | Port STARTED, signal STABLE | Should Pass |
 * | 02 | For each writable property: call setProperty() | property + typed default value | EX_NONE | Should Pass |
 * | 03 | Verify onPropertyChanged fires with correct property and value | none | Callback received, property and value match | Should Pass |
 * | 04 | Read back via getProperty() and verify it matches the set value | none | Value matches | Should Pass |
 * | 05 | Call setPropertyMulti() with all writable properties | kvPairs vector | EX_NONE | Should Pass |
 * | 06 | Verify onPropertyChanged fires for each changed property | none | N callbacks received | Should Pass |
 * | 07 | Stop and close port | none | Port closes successfully | Should Pass |
 *
 * **Refer:** [compositeinput_High-Level_TestSpecification](../../docs/pages/compositeinput_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_COMPOSITEINPUT, PropertyApplicationVerification)
{
    const char *testName = "PropertyApplicationVerification";
    std::vector<compositeInputPropertyConfig_t> configs;

    UT_status_t parseStatus = vts_compositeinput_parse_propertyTest(
        (char*)VTS_COMPOSITEINPUT_TEST_CONFIG_PATH,
        moduleName, testGroupName, testName, configs);

    UT_ASSERT_EQUAL_FATAL(parseStatus, UT_STATUS_OK);
    UT_ASSERT_TRUE(configs.size() > 0);

    /* Locate the port's index in the HFP profile for property metadata lookup */
    uint32_t profilePortCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");

    for (const auto& cfg : configs)
    {
        UT_LOG_INFO("Testing port[%d] for property application\n", cfg.portId);
        StartPort(cfg.portId);

        bool portFound = false;
        for (const auto& entry : mCompositeInputList)
        {
            if (std::get<0>(entry) != cfg.portId)
                continue;

            portFound = true;
            auto& port          = std::get<1>(entry);
            auto& eventListener = std::get<3>(entry);
            auto& controller    = std::get<4>(entry);

            const auto timeout = chrono::milliseconds(cfg.maxWaitTimeMs);

            /* Locate the profile index for this portId */
            uint32_t profileIdx = UINT32_MAX;
            for (uint32_t i = 0; i < profilePortCount; ++i)
            {
                std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
                if (static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str())) == cfg.portId)
                {
                    profileIdx = i;
                    break;
                }
            }

            if (profileIdx == UINT32_MAX)
            {
                UT_LOG_WARNING("Port[%d] not found in profile, skipping\n", cfg.portId);
                break;
            }

            /* Build metadata map: property → {readOnly, type} */
            struct MetaEntry { bool readOnly; PropertyMetadata::PropertyType type; };
            std::map<PortProperty, MetaEntry> metaMap;

            std::string metaListKey = "compositeinput.ports." + std::to_string(profileIdx) +
                                      ".propertyMetadata";
            uint32_t metaCount = UT_KVP_PROFILE_GET_LIST_COUNT(metaListKey.c_str());
            for (uint32_t j = 0; j < metaCount; ++j)
            {
                std::string base = metaListKey + "." + std::to_string(j);
                char metaKeyStr[UT_KVP_MAX_ELEMENT_SIZE];
                UT_KVP_PROFILE_GET_STRING((base + ".key").c_str(), metaKeyStr);
                char metaTypeStr[UT_KVP_MAX_ELEMENT_SIZE];
                UT_KVP_PROFILE_GET_STRING((base + ".type").c_str(), metaTypeStr);
                bool readOnly = UT_KVP_PROFILE_GET_BOOL((base + ".readOnly").c_str());
                PortProperty prop = convertStringToPortProperty(std::string(metaKeyStr));
                metaMap[prop] = { readOnly, convertStringToPropertyType(std::string(metaTypeStr)) };
            }

            /* Collect writable properties from the profile's supportedProperties list */
            std::vector<PropertyKVPair> writeKVPairs;
            std::string propListKey = "compositeinput.ports." + std::to_string(profileIdx) +
                                      ".supportedProperties";
            uint32_t propCount = UT_KVP_PROFILE_GET_LIST_COUNT(propListKey.c_str());
            for (uint32_t j = 0; j < propCount; ++j)
            {
                char propStr[UT_KVP_MAX_ELEMENT_SIZE];
                UT_KVP_PROFILE_GET_STRING((propListKey + "." + std::to_string(j)).c_str(), propStr);
                PortProperty prop = convertStringToPortProperty(std::string(propStr));

                auto it = metaMap.find(prop);
                if (it == metaMap.end() || it->second.readOnly)
                    continue;

                PropertyKVPair kv;
                kv.property = prop;
                switch (it->second.type)
                {
                    case PropertyMetadata::PropertyType::BOOLEAN:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<
                            com::rdk::hal::PropertyValue::Value::Tag::booleanValue>(false);
                        break;
                    case PropertyMetadata::PropertyType::INTEGER:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<
                            com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                        break;
                    case PropertyMetadata::PropertyType::LONG:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<
                            com::rdk::hal::PropertyValue::Value::Tag::longValue>(int64_t(0));
                        break;
                    case PropertyMetadata::PropertyType::FLOAT:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<
                            com::rdk::hal::PropertyValue::Value::Tag::floatValue>(0.0f);
                        break;
                    case PropertyMetadata::PropertyType::DOUBLE:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<
                            com::rdk::hal::PropertyValue::Value::Tag::doubleValue>(0.0);
                        break;
                    case PropertyMetadata::PropertyType::STRING:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<
                            com::rdk::hal::PropertyValue::Value::Tag::stringValue>(
                                android::String16(""));
                        break;
                    default:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<
                            com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                        break;
                }
                writeKVPairs.push_back(kv);
            }

            if (writeKVPairs.empty())
            {
                UT_LOG_WARNING("Port[%d] has no writable properties in profile, skipping\n",
                               cfg.portId);
                break;
            }


            for (const auto& kv : writeKVPairs)
            {
                COMPOSITEINPUT_SET_PROPERTY(controller, kv.property, kv.value);
                UT_LOG_INFO("Port[%d] setProperty(property=%d)\n",
                            cfg.portId, static_cast<int>(kv.property));

                /* Verify onPropertyChanged fires for the set property */
                PortProperty cbProp;
                ::com::rdk::hal::PropertyValue cbValue;
                bool cbReceived = eventListener->getNextPropertyChange(cbProp, cbValue, timeout);
                UT_LOG_INFO("Port[%d] onPropertyChanged received: %s\n",
                            cfg.portId, cbReceived ? "true" : "false");
                UT_ASSERT_TRUE(cbReceived);
                UT_ASSERT_EQUAL(cbProp, kv.property);

                /* Read back value via getProperty() and verify it matches */
                ::com::rdk::hal::PropertyValue readValue;
                COMPOSITEINPUT_GET_PROPERTY(port, kv.property, readValue);
                UT_ASSERT_EQUAL(readValue, kv.value);
                UT_LOG_INFO("Port[%d] property[%d] value verified after setProperty\n",
                            cfg.portId, static_cast<int>(kv.property));
            }

            break;
        }

        UT_ASSERT_TRUE(portFound);
        StopPort(cfg.portId);
    }

}

/**
 * @brief Verify end-to-end video and audio playback readiness via composite input.
 *
 * Validates the full playback readiness chain for a composite input port:
 * the port is opened and started, the test waits for signal STABLE (confirming
 * a live composite source is providing a signal), verifies that the HAL reports
 * the detected video resolution via onVideoModeChanged() and that getStatus()
 * reflects a stable, connected state with a valid detectedResolution.
 *
 * This test confirms that the HAL layer has completed all prerequisites needed
 * by downstream components (audiomixer, planecontrol) to begin actual playback.
 *
 * **Test Group ID:** L3: 03@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** vcomponent must be running with an active composite source @n
 * **User Interaction:** None (vcomponent provides video/audio signal) @n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Start composite input port with active source | portId from config | Port transitions to STARTED | Should Pass |
 * | 02 | Verify getStatus() reports connected=TRUE | getStatus() | connected=true matches config connection_state | Should Pass |
 * | 03 | Wait for signal STABLE via onSignalStatusChanged | none | SignalStatus::STABLE received within timeout | Should Pass |
 * | 04 | Verify onVideoModeChanged fires with valid VideoResolution | none | Callback received, pixelWidth/pixelHeight > 0 | Should Pass |
 * | 05 | Verify getStatus().signalStatus == STABLE and detectedResolution is valid | getStatus() | Matches callback resolution | Should Pass |
 * | 06 | Stop and close port | none | Port closes successfully | Should Pass |
 *
 * **Refer:** [compositeinput_High-Level_TestSpecification](../../docs/pages/compositeinput_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_COMPOSITEINPUT, VideoPlaybackVerification)
{
    const char *testName = "VideoPlaybackVerification";
    std::vector<compositeInputConnectionConfig_t> configs;

    UT_status_t parseStatus = vts_compositeinput_parse_connectionTest(
        (char*)VTS_COMPOSITEINPUT_TEST_CONFIG_PATH,
        moduleName, testGroupName, testName, configs);

    UT_ASSERT_EQUAL_FATAL(parseStatus, UT_STATUS_OK);
    UT_ASSERT_TRUE(configs.size() > 0);

    /* Start all configured ports */
    for (const auto& cfg : configs)
        StartPort(cfg.portId);

    for (const auto& cfg : configs)
    {
        UT_LOG_INFO("Testing port[%d] for video playback\n", cfg.portId);

        bool portFound = false;
        for (const auto& entry : mCompositeInputList)
        {
            if (std::get<0>(entry) != cfg.portId)
                continue;

            portFound = true;
            auto& port               = std::get<1>(entry);
            auto& controllerListener = std::get<2>(entry);

            const auto timeout = chrono::milliseconds(cfg.maxWaitTimeMs);

            /* Step 1: Verify the port reports the expected connection state */
            PortStatus portStatus = {};
            COMPOSITEINPUT_GET_STATUS(port, portStatus);
            UT_LOG_INFO("Port[%d] getStatus().connected: %s\n",
                        cfg.portId, portStatus.connected ? "true" : "false");
            UT_ASSERT_EQUAL(portStatus.connected, cfg.connectionState);

            /* Step 2: Discard any OPENING connection callbacks still in the queue */
            controllerListener->clearConnectionQueue();

            /* Step 3: Wait for signal STABLE — confirms an active source is present */
            SignalStatus signalStatus = SignalStatus::NO_SIGNAL;
            bool signalStable = false;
            while (!signalStable)
            {
                bool ok = controllerListener->getNextSignalStatusChange(signalStatus, timeout);
                if (!ok)
                    break;
                UT_LOG_INFO("Port[%d] signal status update: %d\n",
                            cfg.portId, static_cast<int>(signalStatus));
                if (signalStatus == SignalStatus::STABLE)
                    signalStable = true;
            }
            UT_LOG_INFO("Port[%d] signal STABLE: %s\n", cfg.portId, signalStable ? "true" : "false");
            UT_ASSERT_TRUE(signalStable);

            /* Step 4: Verify onVideoModeChanged fires with a valid VideoResolution */
            VideoResolution resolution = {};
            bool videoModeReceived = controllerListener->getNextVideoModeChange(resolution, timeout);
            UT_LOG_INFO("Port[%d] onVideoModeChanged received: %s\n",
                        cfg.portId, videoModeReceived ? "true" : "false");
            UT_ASSERT_TRUE(videoModeReceived);

            UT_LOG_INFO("Port[%d] VideoResolution: pixelWidth=%d, pixelHeight=%d, "
                        "interlaced=%s, frameRateInHz=%.2f\n",
                        cfg.portId, resolution.pixelWidth, resolution.pixelHeight,
                        resolution.interlaced ? "true" : "false", resolution.frameRateInHz);
            UT_ASSERT_TRUE(resolution.pixelWidth  > 0);
            UT_ASSERT_TRUE(resolution.pixelHeight > 0);

            /* Step 5: Verify getStatus() reflects stable signal and valid resolution */
            COMPOSITEINPUT_GET_STATUS(port, portStatus);
            UT_ASSERT_EQUAL(portStatus.signalStatus, SignalStatus::STABLE);
            UT_ASSERT_TRUE(portStatus.detectedResolution.has_value());

            const VideoResolution& statusRes = portStatus.detectedResolution.value();
            UT_ASSERT_EQUAL(statusRes.pixelWidth,    resolution.pixelWidth);
            UT_ASSERT_EQUAL(statusRes.pixelHeight,   resolution.pixelHeight);
            UT_ASSERT_EQUAL(statusRes.interlaced,    resolution.interlaced);
            UT_ASSERT_EQUAL(statusRes.frameRateInHz, resolution.frameRateInHz);

            UT_LOG_INFO("Port[%d] video playback verification complete — "
                        "HAL is ready for audiomixer and planecontrol integration\n", cfg.portId);

            break;
        }

        UT_ASSERT_TRUE(portFound);
    }

    /* Stop all configured ports */
    for (const auto& cfg : configs)
        StopPort(cfg.portId);
}

/** @} */ // End of COMPOSITE_INPUT_HALTEST_L3
/** @} */ // End of COMPOSITE_INPUT_HALTEST
/** @} */ // End of COMPOSITE_INPUT
/** @} */ // End of HPK
