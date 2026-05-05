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
 * @defgroup COMPOSITEINPUT_HALTEST_INIT COMPOSITEINPUT HAL Tests helper functions
 * @{
 * @parblock
 *
 * ### Initialization functions for VTS tests :
 *
 * Initialization functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_compositeinput_init.cpp
 *
 */

#include <ut.h>
#include <ut_log.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>

#include "com/rdk/hal/compositeinput/ICompositeInputManager.h"

#include "vts_compositeinput_init.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::compositeinput;

sp<ICompositeInputManager> gCOMPOSITEINPUTHandle = nullptr;

static UT_status_t vts_compositeinput_getService()
{
    gCOMPOSITEINPUTHandle = android::interface_cast<ICompositeInputManager>(
            defaultServiceManager()->getService(String16(ICompositeInputManager::serviceName().c_str())));
    if (gCOMPOSITEINPUTHandle == nullptr)
    {
        UT_LOG_ERROR("CompositeInput Service not available\n");
        return UT_STATUS_FAILURE;
    }
    return UT_STATUS_OK;
}

UT_status_t VTS_COMPOSITEINPUT_init()
{
    if(vts_compositeinput_getService() != UT_STATUS_OK)
    {
        UT_LOG_ERROR("Failed to get CompositeInput Service\n");
        return UT_STATUS_FAILURE;
    }

    return UT_STATUS_OK;
}

void VTS_COMPOSITEINPUT_destroy()
{
    UT_LOG_INFO("Destroying VTS_COMPOSITEINPUT...\n");
}

/** @} */ // End of COMPOSITEINPUT_HALTEST_INIT
/** @} */ // End of COMPOSITEINPUT_HALTEST
/** @} */ // End of COMPOSITEINPUT
/** @} */ // End of HPK
