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
 * @file vts_compositeinput_helper_functions.h
 */

#ifndef __VTS_COMPOSITEINPUT_HELPER_H__
#define __VTS_COMPOSITEINPUT_HELPER_H__

#include <gtest/gtest.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <ut.h>
#include <ut_log.h>
#include <ut_kvp_profile.h>
#include <ut_control_plane.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "com/rdk/hal/compositeinput/ICompositeInputEventListener.h"
#include "com/rdk/hal/compositeinput/ICompositeInputManager.h"
#include "com/rdk/hal/compositeinput/ICompositeInputPort.h"
#include "com/rdk/hal/compositeinput/ICompositeInputController.h"
#include "com/rdk/hal/compositeinput/ICompositeInputControllerListener.h"
#include "com/rdk/hal/compositeinput/BnCompositeInputEventListener.h"
#include "com/rdk/hal/compositeinput/BnCompositeInputControllerListener.h"
#include "com/rdk/hal/compositeinput/SignalStatus.h"
#include "com/rdk/hal/compositeinput/PortProperty.h"
#include "com/rdk/hal/compositeinput/PropertyMetadata.h"

#define MAX_LENGTH 256

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::compositeinput;

extern const ut_control_keyStringMapping_t bool_mapTable_compositeinput[];
extern const ut_control_keyStringMapping_t statusCompositeInputException_mapTable[];
extern const ut_control_keyStringMapping_t signalStatus_mapTable_compositeinput[];
extern const ut_control_keyStringMapping_t portProperty_mapTable_compositeinput[];

std::string vecIntToString(std::vector<int32_t>& vals);
compositeinput::SignalStatus convertStringToSignalStatus(const std::string& val);
compositeinput::PortProperty convertStringToPortProperty(const std::string& propertyStr);
compositeinput::PropertyMetadata::PropertyType convertStringToPropertyType(const std::string& typeStr);

#endif //__VTS_COMPOSITEINPUT_HELPER_H__
