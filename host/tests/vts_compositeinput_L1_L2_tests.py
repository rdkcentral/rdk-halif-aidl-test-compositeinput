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

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from vts_compositeinput_helper import compositeInputHelperClass

COMPOSITEINPUT_L1_TEST_SUITE_NAME = "VTS_L1_COMPOSITEINPUT"
COMPOSITEINPUT_L2_TEST_SUITE_NAME = "VTS_L2_COMPOSITEINPUT"

class VTS_COMPOSITEINPUT_L1_L2_Tests(compositeInputHelperClass):
    """
    Runs automated L1 and L2 composite input tests on the target device.
    Handles setup, asset management, and test execution.
    Collects and aggregates test results for pass/fail reporting.
    """

    def __init__(self, log: logModule = None):
        """
        Initializes the L1/L2 composite input test class.

        Args:
            log (logModule, optional): Log module instance. Defaults to None.
        """
        qcId = "1"
        self.testName = "COMPOSITEINPUT_L1_L2"
        super().__init__(self.testName, qcId, log)

    def runTest(self, testSuite, test_case: str = None):
        """
        Runs a single test case (or all test cases) from the given test suite.

        Args:
            testSuite (dict): Test suite entry from the test case configuration.
            test_case (str, optional): Name of the specific test case to run.
                If None, runs all test cases in the suite.

        Returns:
            bool: True if the test passed, False otherwise.
        """
        testsuite_name = testSuite.get("name")
        output = self.utMenu.select(testsuite_name, test_case, None, self.waitTime, False)
        results = self.utMenu.collect_results(output, True)
        if results is None:
            results = False
        return results

    def testFunction(self):
        """
        The main test function to run all L1 and L2 test cases.

        Iterates over the test suites defined in the test case configuration YAML,
        selecting only the L1 and L2 suites. For each suite, runs all configured
        test cases individually (or all at once if 'all' is specified).

        Returns:
            bool: True if all test cases passed, False if any failed.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        self.utMenu.start()

        testsuites = self.testCaseConfiguration.fields.get('test_suites')
        finalresult = True

        for testsuite in testsuites:
            testsuite_name = testsuite.get("name")
            if (testsuite_name != COMPOSITEINPUT_L1_TEST_SUITE_NAME and
                    testsuite_name != COMPOSITEINPUT_L2_TEST_SUITE_NAME):
                continue

            test_cases = testsuite.get("test_cases")

            if len(test_cases) == 1 and test_cases[0] == "all":
                self.log.stepStart(f'Test Suit: {testsuite_name} Run all Tests cases')
                result = self.runTest(testsuite)
                self.log.stepResult(result, f'Test Suit: {testsuite_name} Run all Tests cases')
            else:
                for test_case in test_cases:
                    self.log.stepStart(f'Test Suit: {testsuite_name} Test Case: {test_case}')
                    result = self.runTest(testsuite, test_case)
                    if not result:
                        finalresult = False
                    self.log.stepResult(result, f'Test Suit: {testsuite_name} Test Case: {test_case}')

        self.utMenu.stop()

        return finalresult

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_COMPOSITEINPUT_L1_L2_Tests(summaryLog)
    test.run(False)
