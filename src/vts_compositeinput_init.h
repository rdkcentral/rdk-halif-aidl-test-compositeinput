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
 * @defgroup COMPOSITEINPUT_HALTEST_INIT_HEADER COMPOSITEINPUT HAL Tests helper functions Header File
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
 * @file vts_compositeinput_init.h
 *
 */

#ifndef __VTS_COMPOSITEINPUT_INIT_H__
#define __VTS_COMPOSITEINPUT_INIT_H__

UT_status_t VTS_COMPOSITEINPUT_init();
void VTS_COMPOSITEINPUT_destroy();

#endif //__VTS_COMPOSITEINPUT_INIT_H__

/** @} */ // End of COMPOSITEINPUT_HALTEST_INIT_HEADER
/** @} */ // End of COMPOSITEINPUT_HALTEST
/** @} */ // End of COMPOSITEINPUT
/** @} */ // End of HPK
