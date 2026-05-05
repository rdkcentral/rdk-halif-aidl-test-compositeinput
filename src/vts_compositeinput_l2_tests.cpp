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
 * @defgroup COMPOSITE_INPUT_HALTEST_L2 COMPOSITE INPUT HAL Tests L2 File
 * @{
 * @parblock
 *
 * ### L2 Test Cases for COMPOSITE INPUT HAL :
 *
 * This module includes Level 2 functional tests that validate state transitions,
 * event callbacks, property read operations, metrics reset behaviour and platform
 * concurrency constraints.  All L2 tests are designed to work WITHOUT an external
 * composite input device connected to the DUT.
 *
 * **Pre-Conditions:** Composite Input service running @n
 * **Dependencies:** None@n
 *
 * @endparblock
 */

/**
 * @file vts_compositeinput_l2_tests.cpp
 */

#include "vts_compositeinput.h"
#include <map>
#include <thread>
#include <chrono>

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L2_COMPOSITEINPUT, UT_TESTS_L2)

class VTS_L2_COMPOSITEINPUT : public UTCore
{
public:
    VTS_L2_COMPOSITEINPUT() : UTCore() {}

    ~VTS_L2_COMPOSITEINPUT() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Verify full state machine lifecycle with onStateChanged callbacks.
 *
 * Registers an ICompositeInputEventListener on each port and drives the port through
 * open → start → stop → close. After each operation the test verifies getState()
 * returns the expected stable state and that onStateChanged() fired the correct
 * pairs of intermediate transitions.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | For each port register event listener and verify initial state is CLOSED | portId from profile | State::CLOSED | Should Pass |@n
 * | 02 | Call open() and verify state is READY; check CLOSED→OPENING and OPENING→READY callbacks | valid listener | State::READY + 2 state callbacks | Should Pass |@n
 * | 03 | Call start() and verify state is STARTED; check READY→STARTING and STARTING→STARTED callbacks | controller | State::STARTED + 2 state callbacks | Should Pass |@n
 * | 04 | Call stop() and verify state is READY; check STARTED→STOPPING and STOPPING→READY callbacks | controller | State::READY + 2 state callbacks | Should Pass |@n
 * | 05 | Call close() and verify state is CLOSED; check READY→CLOSING and CLOSING→CLOSED callbacks | controller | State::CLOSED + 2 state callbacks | Should Pass |@n
 * | 06 | Unregister event listener | listener | EX_NONE | Should Pass |@n
 *
 * **Refer:** [compositeinput_L2_Low-Level_TestSpecification](../../docs/pages/compositeinput_L2_Low-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_COMPOSITEINPUT, StateTransitions)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        // Register event listener before open so CLOSED→OPENING is captured
        auto eventListener = android::sp<CompositeInputEventListener>::make();
        COMPOSITEINPUT_REGISTER_EVENT_LISTENER(port, eventListener);
        eventListener->reset();

        // Verify initial state
        State state;
        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] initial state:[%d]\n", profilePortId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::CLOSED);

        // ── open ──────────────────────────────────────────────────────────────
        auto controllerListener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, controllerListener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after open:[%d]\n", profilePortId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::READY);

        // Expect CLOSED→OPENING and OPENING→READY
        std::pair<State, State> change;
        UT_ASSERT_TRUE(eventListener->hasStateChanged());
        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::CLOSED);
        UT_ASSERT_EQUAL(change.second, State::OPENING);

        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::OPENING);
        UT_ASSERT_EQUAL(change.second, State::READY);
        eventListener->reset();

        // ── start ─────────────────────────────────────────────────────────────
        COMPOSITEINPUT_START(controller);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after start:[%d]\n", profilePortId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::STARTED);

        // Expect READY→STARTING and STARTING→STARTED
        UT_ASSERT_TRUE(eventListener->hasStateChanged());
        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::READY);
        UT_ASSERT_EQUAL(change.second, State::STARTING);

        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::STARTING);
        UT_ASSERT_EQUAL(change.second, State::STARTED);
        eventListener->reset();

        // ── stop ──────────────────────────────────────────────────────────────
        COMPOSITEINPUT_STOP(controller);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after stop:[%d]\n", profilePortId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::READY);

        // Expect STARTED→STOPPING and STOPPING→READY
        UT_ASSERT_TRUE(eventListener->hasStateChanged());
        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::STARTED);
        UT_ASSERT_EQUAL(change.second, State::STOPPING);

        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::STOPPING);
        UT_ASSERT_EQUAL(change.second, State::READY);
        eventListener->reset();

        // ── close ─────────────────────────────────────────────────────────────
        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after close:[%d]\n", profilePortId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::CLOSED);

        // Expect READY→CLOSING and CLOSING→CLOSED
        UT_ASSERT_TRUE(eventListener->hasStateChanged());
        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::READY);
        UT_ASSERT_EQUAL(change.second, State::CLOSING);

        UT_ASSERT_TRUE(eventListener->getNextStateChanges(change));
        UT_LOG_INFO("Port[%d] onStateChanged: %d to %d\n", profilePortId,
                    static_cast<int>(change.first), static_cast<int>(change.second));
        UT_ASSERT_EQUAL(change.first,  State::CLOSING);
        UT_ASSERT_EQUAL(change.second, State::CLOSED);

        COMPOSITEINPUT_UNREGISTER_EVENT_LISTENER(port, eventListener);
        UT_LOG_INFO("Port[%d] StateTransitions passed\n", profilePortId);
    }
}

/**
 * @brief Verify connection state and signal status callbacks with no device connected.
 *
 * Opens each port and waits for onConnectionChanged(false) from the controller listener,
 * then starts the port and waits for onSignalStatusChanged(NO_SIGNAL). Both getStatus()
 * fields are validated to match the received callbacks.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** No external composite input device connected@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | For each port: open with controller listener | valid listener | Non-null controller | Should Pass |@n
 * | 02 | Wait for onConnectionChanged callback (up to 1 s) | timeout 1000 ms | connected == false | Should Pass |@n
 * | 03 | Verify getStatus().connected == false | none | connected is false | Should Pass |@n
 * | 04 | Start the port | controller | EX_NONE | Should Pass |@n
 * | 05 | Wait for onSignalStatusChanged callback (up to 1 s) | timeout 1000 ms | signalStatus == NO_SIGNAL | Should Pass |@n
 * | 06 | Verify getStatus().signalStatus == NO_SIGNAL | none | signalStatus is NO_SIGNAL | Should Pass |@n
 * | 07 | Stop and close (cleanup) | controller | Port returns to CLOSED | Should Pass |@n
 *
 * **Refer:** [compositeinput_L2_Low-Level_TestSpecification](../../docs/pages/compositeinput_L2_Low-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_COMPOSITEINPUT, ConnectionState)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto controllerListener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, controllerListener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        // Wait for onConnectionChanged — AIDL guarantees at least one call during OPENING
        bool connected = true;
        bool gotConnectionCallback = controllerListener->getNextConnectionChange(
                connected, std::chrono::milliseconds(1000));
        UT_LOG_INFO("Port[%d] onConnectionChanged received:[%s] connected:[%s]\n",
                    profilePortId,
                    gotConnectionCallback ? "true" : "false",
                    connected ? "true" : "false");
        UT_ASSERT_TRUE(gotConnectionCallback);
        UT_ASSERT_EQUAL(connected, false);

        // Validate getStatus().connected
        PortStatus portStatus;
        COMPOSITEINPUT_GET_STATUS(port, portStatus);
        UT_LOG_INFO("Port[%d] getStatus().connected:[%s]\n",
                    profilePortId, portStatus.connected ? "true" : "false");
        UT_ASSERT_EQUAL(portStatus.connected, false);

        COMPOSITEINPUT_START(controller);

        // Wait for onSignalStatusChanged — AIDL guarantees at least one call during STARTING
        SignalStatus signalStatus = SignalStatus::STABLE;
        bool gotSignalCallback = controllerListener->getNextSignalStatusChange(
                signalStatus, std::chrono::milliseconds(1000));
        UT_LOG_INFO("Port[%d] onSignalStatusChanged received:[%s] signalStatus:[%d]\n",
                    profilePortId,
                    gotSignalCallback ? "true" : "false",
                    static_cast<int>(signalStatus));
        UT_ASSERT_TRUE(gotSignalCallback);
        UT_ASSERT_EQUAL(signalStatus, SignalStatus::NO_SIGNAL);

        // Validate getStatus().signalStatus
        COMPOSITEINPUT_GET_STATUS(port, portStatus);
        UT_LOG_INFO("Port[%d] getStatus().signalStatus:[%d]\n",
                    profilePortId, static_cast<int>(portStatus.signalStatus));
        UT_ASSERT_EQUAL(portStatus.signalStatus, SignalStatus::NO_SIGNAL);

        COMPOSITEINPUT_STOP(controller);
        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);

        UT_LOG_INFO("Port[%d] ConnectionState passed\n", profilePortId);
    }
}

/**
 * @brief Verify getProperty() succeeds for all supported properties in STARTED state.
 *
 * Opens and starts each port, then calls getProperty() for every property declared in
 * PortCapabilities.supportedProperties. All calls must return EX_NONE.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | For each port: open and start | valid listener | Port in STARTED state | Should Pass |@n
 * | 02 | Get PortCapabilities.supportedProperties | none | Non-empty list | Should Pass |@n
 * | 03 | Build a readOnly lookup map from compositeinput.ports.N.propertyMetadata | profile | Map of property → {readOnly, type} | Should Pass |@n
 * | 04 | For each supported property: if writable call setProperty() then getProperty() and assert returned value equals set value; if read-only call getProperty() only | each PortProperty | All calls return EX_NONE; set/get values match for writable properties | Should Pass |@n
 * | 05 | Stop and close (cleanup) | controller | Port returns to CLOSED | Should Pass |@n
 *
 * **Refer:** [compositeinput_L2_Low-Level_TestSpecification](../../docs/pages/compositeinput_L2_Low-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_COMPOSITEINPUT, PropertySetGetStarted)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto controllerListener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, controllerListener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        // Get capabilities to find supported properties
        PortCapabilities caps;
        COMPOSITEINPUT_GET_CAPABILITIES(port, caps);
        UT_ASSERT_TRUE(caps.supportedProperties.size() > 0);

        // Build a lookup map: PortProperty → {readOnly, PropertyType} from profile metadata
        struct MetaEntry { bool readOnly; PropertyMetadata::PropertyType type; };
        std::map<PortProperty, MetaEntry> metaMap;
        std::string metaListKey = "compositeinput.ports." + std::to_string(i) + ".propertyMetadata";
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

        // For each supported property: set then get if writable; get only if read-only
        for (const auto& property : caps.supportedProperties)
        {
            auto it = metaMap.find(property);
            bool readOnly = (it == metaMap.end()) ? true : it->second.readOnly;

            if (!readOnly)
            {
                // Build a typed default value matching PropertyMetadata.type
                PropertyMetadata::PropertyType propType = it->second.type;
                com::rdk::hal::PropertyValue pv;
                switch (propType)
                {
                    case PropertyMetadata::PropertyType::BOOLEAN:
                        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::booleanValue>(false);
                        break;
                    case PropertyMetadata::PropertyType::INTEGER:
                        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                        break;
                    case PropertyMetadata::PropertyType::LONG:
                        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::longValue>(int64_t(0));
                        break;
                    case PropertyMetadata::PropertyType::FLOAT:
                        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::floatValue>(0.0f);
                        break;
                    case PropertyMetadata::PropertyType::DOUBLE:
                        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::doubleValue>(0.0);
                        break;
                    case PropertyMetadata::PropertyType::STRING:
                        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::stringValue>(android::String16(""));
                        break;
                    default:
                        pv.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                        break;
                }

                COMPOSITEINPUT_SET_PROPERTY(controller, property, pv);
                UT_LOG_INFO("Port[%d] setProperty(%d) OK\n", profilePortId, static_cast<int>(property));

                com::rdk::hal::PropertyValue getVal;
                COMPOSITEINPUT_GET_PROPERTY(port, property, getVal);
                UT_LOG_INFO("Port[%d] getProperty(%d) after set: OK\n", profilePortId, static_cast<int>(property));

                UT_ASSERT_EQUAL(getVal, pv);
                UT_LOG_INFO("Port[%d] getProperty(%d) value matches setProperty value\n",
                            profilePortId, static_cast<int>(property));
            }
            else
            {
                com::rdk::hal::PropertyValue propValue;
                COMPOSITEINPUT_GET_PROPERTY(port, property, propValue);
                UT_LOG_INFO("Port[%d] getProperty(%d) [read-only]: OK\n", profilePortId, static_cast<int>(property));
            }
        }

        COMPOSITEINPUT_STOP(controller);
        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);

        UT_LOG_INFO("Port[%d] PropertyGetStarted passed\n", profilePortId);
    }
}

/**
 * @brief Verify getPropertyMulti() returns correct results for all profile properties in STARTED state.
 *
 * Opens and starts each port, then calls getPropertyMulti() with the full list of profile-declared
 * properties. Verifies the returned count and key ordering match the request.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | For each port: open and start | valid listener | Port in STARTED state | Should Pass |@n
 * | 02 | Load properties from compositeinput.ports.N.supportedProperties in profile | profile data | Non-empty list | Should Pass |@n
 * | 03 | Build readOnly lookup map from compositeinput.ports.N.propertyMetadata | profile | Map of property → {readOnly, type} | Should Pass |@n
 * | 04 | Call getPropertyMulti() for all supported properties; verify key ordering | full properties vector | EX_NONE; kvPairs.size() == properties.size(); keys in order | Should Pass |@n
 * | 05 | For writable properties: call setPropertyMulti(), then getPropertyMulti() and assert each returned value equals the set value | writable kvPairs | EX_NONE; values match | Should Pass |@n
 * | 06 | For read-only properties: call getPropertyMulti() only | read-only property keys | EX_NONE; key ordering matches | Should Pass |@n
 * | 07 | Stop and close (cleanup) | controller | Port returns to CLOSED | Should Pass |@n
 *
 * **Refer:** [compositeinput_L2_Low-Level_TestSpecification](../../docs/pages/compositeinput_L2_Low-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_COMPOSITEINPUT, PropertyMultiSetGetStarted)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto controllerListener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, controllerListener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        // Load properties from profile
        std::vector<PortProperty> properties;
        std::string propListKey = "compositeinput.ports." + std::to_string(i) + ".supportedProperties";
        uint32_t profilePropCount = UT_KVP_PROFILE_GET_LIST_COUNT(propListKey.c_str());
        UT_ASSERT_TRUE(profilePropCount > 0);

        for (uint32_t j = 0; j < profilePropCount; ++j)
        {
            char propStr[UT_KVP_MAX_ELEMENT_SIZE];
            std::string propKey = propListKey + "." + std::to_string(j);
            UT_KVP_PROFILE_GET_STRING(propKey.c_str(), propStr);
            properties.push_back(convertStringToPortProperty(std::string(propStr)));
        }

        // Build metaMap and split into writable / read-only groups
        struct MetaEntry { bool readOnly; PropertyMetadata::PropertyType type; };
        std::map<PortProperty, MetaEntry> metaMap;
        std::string metaListKey = "compositeinput.ports." + std::to_string(i) + ".propertyMetadata";
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

        std::vector<PropertyKVPair> writeKVPairs;
        std::vector<PortProperty> readOnlyProps;

        for (const auto& property : properties)
        {
            auto it = metaMap.find(property);
            bool readOnly = (it == metaMap.end()) ? true : it->second.readOnly;

            if (!readOnly)
            {
                PropertyMetadata::PropertyType propType = it->second.type;
                PropertyKVPair kv;
                kv.property = property;
                switch (propType)
                {
                    case PropertyMetadata::PropertyType::BOOLEAN:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::booleanValue>(false);
                        break;
                    case PropertyMetadata::PropertyType::INTEGER:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                        break;
                    case PropertyMetadata::PropertyType::LONG:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::longValue>(int64_t(0));
                        break;
                    case PropertyMetadata::PropertyType::FLOAT:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::floatValue>(0.0f);
                        break;
                    case PropertyMetadata::PropertyType::DOUBLE:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::doubleValue>(0.0);
                        break;
                    case PropertyMetadata::PropertyType::STRING:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::stringValue>(android::String16(""));
                        break;
                    default:
                        kv.value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(int32_t(0));
                        break;
                }
                writeKVPairs.push_back(kv);
            }
            else
            {
                readOnlyProps.push_back(property);
            }
        }

        // Writable: setPropertyMulti then getPropertyMulti; assert values match
        if (!writeKVPairs.empty())
        {
            COMPOSITEINPUT_SET_PROPERTY_MULTI(controller, writeKVPairs);
            UT_LOG_INFO("Port[%d] setPropertyMulti(%zu props) OK\n", profilePortId, writeKVPairs.size());

            std::vector<PortProperty> writtenKeys;
            for (const auto& kv : writeKVPairs)
                writtenKeys.push_back(kv.property);

            std::vector<PropertyKVPair> readBack;
            COMPOSITEINPUT_GET_PROPERTY_MULTI(port, writtenKeys, readBack);
            UT_ASSERT_EQUAL(readBack.size(), writeKVPairs.size());

            for (size_t k = 0; k < readBack.size(); ++k)
            {
                UT_ASSERT_EQUAL(readBack[k].property, writeKVPairs[k].property);
                UT_ASSERT_EQUAL(readBack[k].value, writeKVPairs[k].value);
                UT_LOG_INFO("Port[%d] writable property[%d] value matches after set\n",
                            profilePortId, static_cast<int>(readBack[k].property));
            }
        }
        else
        {
            UT_LOG_INFO("Port[%d]: no writable properties; setPropertyMulti step skipped\n", profilePortId);
        }

        // Read-only: getPropertyMulti only
        if (!readOnlyProps.empty())
        {
            std::vector<PropertyKVPair> readOnlyResult;
            COMPOSITEINPUT_GET_PROPERTY_MULTI(port, readOnlyProps, readOnlyResult);
            UT_ASSERT_EQUAL(readOnlyResult.size(), readOnlyProps.size());
            for (size_t k = 0; k < readOnlyResult.size(); ++k)
            {
                UT_ASSERT_EQUAL(readOnlyResult[k].property, readOnlyProps[k]);
                UT_LOG_INFO("Port[%d] read-only property[%d] getPropertyMulti OK\n",
                            profilePortId, static_cast<int>(readOnlyResult[k].property));
            }
        }

        COMPOSITEINPUT_STOP(controller);
        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);

        UT_LOG_INFO("Port[%d] PropertyMultiGetStarted passed\n", profilePortId);
    }
}

/**
 * @brief Verify resetMetrics() clears METRIC counters and updates METRIC_LAST_RESET_TIMESTAMP.
 *
 * For each port: opens, starts, reads METRIC_LAST_RESET_TIMESTAMP before reset, calls
 * resetMetrics(), then verifies the timestamp is updated and all other METRIC_* counters
 * read back as zero.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | For each port: open and start | valid listener | Port in STARTED state | Should Pass |@n
 * | 02 | Filter METRIC_* properties (value >= 1000) from getCapabilities() | none | Metric list | Should Pass |@n
 * | 03 | Read METRIC_LAST_RESET_TIMESTAMP before reset (if supported) | none | API returns EX_NONE | Should Pass |@n
 * | 04 | Call resetMetrics() | controller | API returns EX_NONE | Should Pass |@n
 * | 05 | Read METRIC_LAST_RESET_TIMESTAMP after reset (if supported) | none | Value is non-zero and >= value before reset | Should Pass |@n
 * | 06 | Read each other METRIC_* property after reset | each METRIC_* | API returns EX_NONE; value == 0 | Should Pass |@n
 * | 07 | Stop and close (cleanup) | controller | Port returns to CLOSED | Should Pass |@n
 *
 * **Refer:** [compositeinput_L2_Low-Level_TestSpecification](../../docs/pages/compositeinput_L2_Low-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_COMPOSITEINPUT, ResetMetrics)
{
    uint32_t portCount = UT_KVP_PROFILE_GET_LIST_COUNT("compositeinput.ports");
    UT_ASSERT_TRUE(portCount > 0);

    for (uint32_t i = 0; i < portCount; ++i)
    {
        std::string idKey = "compositeinput.ports." + std::to_string(i) + ".id";
        int32_t profilePortId = static_cast<int32_t>(UT_KVP_PROFILE_GET_UINT32(idKey.c_str()));

        android::sp<ICompositeInputPort> port;
        COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, profilePortId, port);
        UT_ASSERT_NOT_EQUAL(port, nullptr);

        auto controllerListener = android::sp<CompositeInputControllerListener>::make();
        android::sp<ICompositeInputController> controller;
        COMPOSITEINPUT_OPEN(port, controllerListener, controller);
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        COMPOSITEINPUT_START(controller);

        // Verify state
        State state;
        COMPOSITEINPUT_GET_STATE(port, state);
        UT_LOG_INFO("Port[%d] state after start:[%d]\n", profilePortId, static_cast<int>(state));
        UT_ASSERT_EQUAL(state, State::STARTED);

        // Collect metric properties (PortProperty enum value >= 1000)
        PortCapabilities caps;
        COMPOSITEINPUT_GET_CAPABILITIES(port, caps);

        std::vector<PortProperty> metricProps;
        for (const auto& prop : caps.supportedProperties)
        {
            if (static_cast<int32_t>(prop) >= 1000)
                metricProps.push_back(prop);
        }
        UT_LOG_INFO("Port[%d] found %zu metric property(ies)\n",
                    profilePortId, metricProps.size());

        // Read METRIC_LAST_RESET_TIMESTAMP before reset (if supported)
        int64_t tsBefore = 0;
        bool hasTimestamp = false;
        for (const auto& prop : metricProps)
        {
            if (prop == PortProperty::METRIC_LAST_RESET_TIMESTAMP)
            {
                hasTimestamp = true;
                com::rdk::hal::PropertyValue val;
                COMPOSITEINPUT_GET_PROPERTY(port, prop, val);
                if (val.value.has_value() &&
                    val.value->getTag() == com::rdk::hal::PropertyValue::Value::Tag::longValue)
                    tsBefore = val.value->get<com::rdk::hal::PropertyValue::Value::Tag::longValue>();
                UT_LOG_INFO("Port[%d] METRIC_LAST_RESET_TIMESTAMP before reset:[%lld]\n",
                            profilePortId, static_cast<long long>(tsBefore));
                break;
            }
        }

        // Reset metrics
        COMPOSITEINPUT_RESET_METRICS(controller);
        UT_LOG_INFO("Port[%d] resetMetrics() called\n", profilePortId);

        // Read METRIC_LAST_RESET_TIMESTAMP after reset
        if (hasTimestamp)
        {
            com::rdk::hal::PropertyValue val;
            COMPOSITEINPUT_GET_PROPERTY(port, PortProperty::METRIC_LAST_RESET_TIMESTAMP, val);
            int64_t tsAfter = 0;
            if (val.value.has_value() &&
                val.value->getTag() == com::rdk::hal::PropertyValue::Value::Tag::longValue)
                tsAfter = val.value->get<com::rdk::hal::PropertyValue::Value::Tag::longValue>();
            UT_LOG_INFO("Port[%d] METRIC_LAST_RESET_TIMESTAMP after reset:[%lld]\n",
                        profilePortId, static_cast<long long>(tsAfter));
            UT_ASSERT_TRUE(tsAfter > 0);
            UT_ASSERT_TRUE(tsAfter >= tsBefore);
        }

        // Verify all other METRIC_* counters read as 0 after reset
        for (const auto& prop : metricProps)
        {
            if (prop == PortProperty::METRIC_LAST_RESET_TIMESTAMP)
                continue;

            com::rdk::hal::PropertyValue val;
            COMPOSITEINPUT_GET_PROPERTY(port, prop, val);

            int64_t counterValue = 0;
            if (val.value.has_value())
            {
                if (val.value->getTag() == com::rdk::hal::PropertyValue::Value::Tag::longValue)
                    counterValue = val.value->get<com::rdk::hal::PropertyValue::Value::Tag::longValue>();
                else if (val.value->getTag() == com::rdk::hal::PropertyValue::Value::Tag::intValue)
                    counterValue = static_cast<int64_t>(val.value->get<com::rdk::hal::PropertyValue::Value::Tag::intValue>());
            }

            UT_LOG_INFO("Port[%d] METRIC[%d] after reset:[%lld]\n",
                        profilePortId, static_cast<int>(prop),
                        static_cast<long long>(counterValue));
            UT_ASSERT_EQUAL(counterValue, static_cast<int64_t>(0));
        }

        COMPOSITEINPUT_STOP(controller);
        bool closed = false;
        COMPOSITEINPUT_CLOSE(port, controller, closed);
        UT_ASSERT_TRUE(closed);

        UT_LOG_INFO("Port[%d] ResetMetrics passed\n", profilePortId);
    }
}

/**
 * @brief Verify that starting a second port when maximumConcurrentStartedPorts is 1 fails.
 *
 * Opens and starts the first available port. Then opens the second port and attempts to
 * start it — this must fail with EX_ILLEGAL_STATE. After stopping the first port, the
 * second must start successfully.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 006@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Profile declares maximumConcurrentStartedPorts == 1; at least 2 ports available@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Read maximumConcurrentStartedPorts from profile; skip if != 1 or < 2 ports | profile | value == 1 | Should Pass |@n
 * | 02 | Open and start port[0] | portIds[0] | State::STARTED | Should Pass |@n
 * | 03 | Open port[1] | portIds[1] | Non-null controller; State::READY | Should Pass |@n
 * | 04 | Attempt to start port[1] | controller1 | !isOk() with EX_ILLEGAL_STATE | Should Pass |@n
 * | 05 | Verify port[1] state is still READY | getState() | State::READY | Should Pass |@n
 * | 06 | Stop port[0] | controller0 | State::READY | Should Pass |@n
 * | 07 | Start port[1] now | controller1 | State::STARTED | Should Pass |@n
 * | 08 | Stop port[1] and close both | controllers | Both ports CLOSED | Should Pass |@n
 *
 * **Refer:** [compositeinput_L2_Low-Level_TestSpecification](../../docs/pages/compositeinput_L2_Low-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_COMPOSITEINPUT, MaximumConcurrentStartedPorts)
{
    // Check preconditions
    uint32_t maxConcurrent = UT_KVP_PROFILE_GET_UINT32(
            "compositeinput/platformCapabilities/maximumConcurrentStartedPorts");
    UT_LOG_INFO("maximumConcurrentStartedPorts:[%u]\n", maxConcurrent);

    if (maxConcurrent != 1)
    {
        UT_LOG_INFO("maximumConcurrentStartedPorts != 1; skipping test\n");
        return;
    }

    std::vector<int32_t> portIds;
    COMPOSITEINPUT_GET_PORT_IDS(gCOMPOSITEINPUTHandle, portIds);
    UT_ASSERT_TRUE(portIds.size() > 0);

    if (portIds.size() < 2)
    {
        UT_LOG_INFO("Only %zu port(s) available; need >= 2 to test concurrency limit; skipping\n",
                    portIds.size());
        return;
    }

    // Open and start port[0]
    android::sp<ICompositeInputPort> port0;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[0], port0);
    UT_ASSERT_NOT_EQUAL(port0, nullptr);

    auto listener0 = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller0;
    COMPOSITEINPUT_OPEN(port0, listener0, controller0);
    UT_ASSERT_NOT_EQUAL(controller0, nullptr);

    COMPOSITEINPUT_START(controller0);

    State state0;
    COMPOSITEINPUT_GET_STATE(port0, state0);
    UT_LOG_INFO("Port[%d] state after start:[%d]\n", portIds[0], static_cast<int>(state0));
    UT_ASSERT_EQUAL(state0, State::STARTED);

    // Open port[1] — should succeed (READY)
    android::sp<ICompositeInputPort> port1;
    COMPOSITEINPUT_GET_PORT(gCOMPOSITEINPUTHandle, portIds[1], port1);
    UT_ASSERT_NOT_EQUAL(port1, nullptr);

    auto listener1 = android::sp<CompositeInputControllerListener>::make();
    android::sp<ICompositeInputController> controller1;
    COMPOSITEINPUT_OPEN(port1, listener1, controller1);
    UT_ASSERT_NOT_EQUAL(controller1, nullptr);

    State state1;
    // Attempt to start port[1] — must fail (exceeds maximumConcurrentStartedPorts)
    ::android::binder::Status startStatus;
    COMPOSITEINPUT_START_NEG(controller1, startStatus);
    UT_LOG_INFO("Port[%d] second start: isOk:[%s] exception:[%d]\n",
                portIds[1],
                startStatus.isOk() ? "true" : "false",
                startStatus.exceptionCode());
    UT_ASSERT_TRUE(!startStatus.isOk());
    UT_ASSERT_EQUAL(startStatus.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

    // State of port[1] must still be READY
    COMPOSITEINPUT_GET_STATE(port1, state1);
    UT_LOG_INFO("Port[%d] state after failed start:[%d]\n", portIds[1], static_cast<int>(state1));
    UT_ASSERT_EQUAL(state1, State::READY);

    // Stop port[0] — now port[1] can start
    COMPOSITEINPUT_STOP(controller0);
    COMPOSITEINPUT_GET_STATE(port0, state0);
    UT_LOG_INFO("Port[%d] state after stop:[%d]\n", portIds[0], static_cast<int>(state0));
    UT_ASSERT_EQUAL(state0, State::READY);

    // Start port[1] — must now succeed
    COMPOSITEINPUT_START(controller1);
    COMPOSITEINPUT_GET_STATE(port1, state1);
    UT_LOG_INFO("Port[%d] state after start (after port[0] stopped):[%d]\n",
                portIds[1], static_cast<int>(state1));
    UT_ASSERT_EQUAL(state1, State::STARTED);

    // Cleanup
    COMPOSITEINPUT_STOP(controller1);

    bool closed0 = false;
    COMPOSITEINPUT_CLOSE(port0, controller0, closed0);
    UT_ASSERT_TRUE(closed0);

    bool closed1 = false;
    COMPOSITEINPUT_CLOSE(port1, controller1, closed1);
    UT_ASSERT_TRUE(closed1);

    UT_LOG_INFO("MaximumConcurrentStartedPorts passed\n");
}

/** @} */ // End of COMPOSITE_INPUT_HALTEST_L2
/** @} */ // End of COMPOSITE_INPUT_HALTEST
/** @} */ // End of COMPOSITE_INPUT
/** @} */ // End of HPK
