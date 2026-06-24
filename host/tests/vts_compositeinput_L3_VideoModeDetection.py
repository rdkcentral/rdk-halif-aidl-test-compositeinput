#!/usr/bin/env python3
#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2026 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

import os
import sys
import time
import threading

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from composite_controller.compositeController import CompositeController
from raft.framework.core.logModule import logModule
from vts_compositeinput_helper import compositeInputHelperClass, COMPOSITEINPUT_L3_TEST_SUITE_NAME

class VTS_COMPOSITEINPUT_L3_VideoModeDetection(compositeInputHelperClass):
    """
    Runs L3 VideoModeDetection test for composite input HAL.

    Validates that ICompositeInputControllerListener::onVideoModeChanged() fires
    with the correct VideoResolution when the compositeinput device reports video mode change.
    The test verifies two sequential video mode changes:
      1. videomode1 from the test config (initial signal)
      2. videomode2 from the test config (composite input source switch)
    """

    def __init__(self, log: logModule = None, hfpfile:str=None, testMenuConfigFile=None, moduleName=None, artifactDirPath=None):
        """
        Initializes the L3 VideoModeDetection test.

        Args:
            log (logModule, optional): Log module instance. Defaults to None.
        """
        self.testName = "CompositeInput_L3_VideoModeDetection"
        qcId = "3"
        super().__init__(self.testName, qcId, log, hfpfile, testMenuConfigFile, moduleName, artifactDirPath)

        self.compositeController = CompositeController(
            self.log, self.dut.rawConfig.get("dut").get("compositeController"))

    def videoModeThread(self):
        """
        Background thread that drives sequential video mode changes.

        Reads the ``videomode`` list from the test config for each port and
        sends each entry via setVideoMode(). Each entry triggers one
        onVideoModeChanged callback in the C++ test.

        An initial delay allows the C++ test to initialize and start the port
        before video mode events are sent.
        """
        time.sleep(2)
        ports = self.test_config_data[self.moduleName][self.testLevel][self.testcase]['ports']
        for port in ports:
            port_id = port.get('id')
            videomodes = port.get('videomode', [])

            for vm in videomodes:
                self.compositeController.setVideoMode(
                    port_id,
                    vm.get('width'),
                    vm.get('height'),
                    vm.get('interlaced', False),
                    float(vm.get('framerate', 0))
                )
            time.sleep(1)

    def testFunction(self):
        """
        The main test function to run the L3 VideoModeDetection.

        Establishes an active composite source (connected + STABLE signal) for
        each port before the test starts. A background thread then drives the
        two video mode changes that the C++ test waits for.

        Returns:
            bool: True if the test passed, False otherwise.
        """
        self.testcase = "VideoModeDetection"
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        final_results = True
        # Update test configuration on target device from HFP profile
        self.updateConnectionTestConfiguration(self.testcase)

        # Load test config (no port expansion from profile needed for VideoModeDetection)
        self.test_config_data = self.compositeInputReadYaml(self.l3TestConfigFile)

        self.videoModeThreadObj = threading.Thread(target=self.videoModeThread)

        self.utMenu.start()

        # Set initial connected + STABLE state so the port can enter STARTED with active source
        ports = self.test_config_data[self.moduleName][self.testLevel][self.testcase]['ports']
        for port in ports:
            port_id = port.get('id')
            self.compositeController.connectDevice(port_id, True)
            self.compositeController.setSignalStatus(port_id, "STABLE")
            time.sleep(1)

        self.videoModeThreadObj.start()

        self.log.stepStart(f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')
        output = self.utMenu.select(COMPOSITEINPUT_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)

        results = self.utMenu.collect_results(output, True)
        if results is None:
            final_results = False

        self.videoModeThreadObj.join()

        self.log.stepResult(results, f'Test Suit: {COMPOSITEINPUT_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')

        self.utMenu.stop()

        return final_results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_COMPOSITEINPUT_L3_VideoModeDetection(summaryLog)
    test.run(False)
