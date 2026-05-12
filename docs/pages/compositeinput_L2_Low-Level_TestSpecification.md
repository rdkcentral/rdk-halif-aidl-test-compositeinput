# Composite Input L2 Low Level Test Specification

## Table of Contents
- [Overview](#overview)
- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [References](#references)
- [Level 2 Test Specification](#level-2-test-specification)

## Overview

This document describes the level 2 low level test specification for the `Composite Input` module.

### Acronyms, Terms and Abbreviations

- `HAL`   - Hardware Abstraction Layer
- `VTS`   - Vendor Test Suite
- `L1`    - Level 1 Testing
- `L2`    - Level 2 Testing
- `L3`    - Level 3 Testing
- `DUT`   - Device Under Test
- `HFP`   - Hardware Feature Profile
- `RAFT`  - Rapid Automation Framework for Testing
- `YAML`  - YAML Ain't Markup Language
- `AIDL`  - Android Interface Definition Language

### References

- High Level Test Specification - [compositeinput_High-Level_TestSpecification.md](compositeinput_High-Level_TestSpecification.md)

## Level 2 Test Specification

The following functions are intended to test the `Composite Input` `HAL` module according to the `L2` Test specification.

### Test 1
| Title | Details |
| -- | -- |
| Function Name | `StateTransitions` |
| Description | This test validates the state machine behaviour of the Composite Input interface. It ensures that the interface transitions correctly between CLOSED, READY, STARTED, and other states, and that state change notifications are received by the event listener. |
| Test Group | 02 |
| Test Case ID | 001 |
| Priority | High |

**Pre-Conditions :**
- No external composite input device connected to the DUT
- Composite Input interface must be available in State::CLOSED initially

**Dependencies :**
- None

**User Interaction :**
None

**Test Procedure :**
| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Register Event Listeners | None | Register the event listener |
| 2 | Get the State before Open | None | State should be CLOSED |
| 3 | Open the Composite Input with event listener | event listener handle | Function should return valid controller handle |
| 4 | Get the State | None | State should be OPENING |
| 5 | Check the event listener callback | | Previous State should be CLOSED and current state should be OPENING |
| 6 | Get the State | None | State should be READY |
| 7 | Check the event listener callback | | Previous State should be OPENING and current state should be READY |
| 8 | Start the Composite Input | | Function should return `True` |
| 9 | Get the State | None | State should be STARTING |
| 10 | Check the event listener callback | | Previous State should be READY and current state should be STARTING |
| 11 | Get the State | None | State should be STARTED |
| 12 | Check the event listener callback | | Previous State should be STARTING and current state should be STARTED |
| 13 | Stop the Composite Input | | Function should return `True` |
| 14 | Get the State | None | State should be STOPPING |
| 15 | Check the event listener callback | | Previous State should be STARTED and current state should be STOPPING |
| 16 | Get the State | None | State should be READY |
| 17 | Check the event listener callback | | Previous State should be STOPPING and current state should be READY |
| 18 | Close the Composite Input | | Function should return `True` |
| 19 | Get the State | None | State should be CLOSING |
| 20 | Check the event listener callback | | Previous State should be READY and current state should be CLOSING |
| 21 | Get the State | None | State should be CLOSED |
| 22 | Check the event listener callback | | Previous State should be CLOSING and current state should be CLOSED |
| 23 | Unregister Event Listeners | | Function should return `True` |

### Test 2
| Title | Details |
| -- | -- |
| Function Name | `ConnectionState` |
| Description | This test verifies that the default callbacks `onConnectionChanged()` and `onSignalStatusChanged()` are fired with the correct values during state transitions. It ensures `onConnectionChanged(false)` is fired after opening and `onSignalStatusChanged(NO_SIGNAL)` is fired after starting when no device is connected. |
| Test Group | 02 |
| Test Case ID | 002 |
| Priority | High |

**Pre-Conditions :**
- No external composite input device connected to the DUT
- Composite Input interface must be available in State::CLOSED initially

**Dependencies :**
- None

**User Interaction :**
None

**Test Procedure :**
| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Open the Composite Input with event listener | event listener handle | Function should return valid controller handle |
| 2 | Check `onConnectionChanged` callback | None | Callback should be fired with `false` |
| 3 | Get connection state via `getStatus()` | None | Should return `connected = false` |
| 4 | Start the Composite Input | | Function should return `True` |
| 5 | Check `onSignalStatusChanged` callback | | Callback should be fired with `NO_SIGNAL` |
| 6 | Get signal status via `getStatus()` | None | Should return `signalStatus = NO_SIGNAL` |
| 7 | Stop and Close the Composite Input | | Function should return `True` |

### Test 3
| Title | Details |
| -- | -- |
| Function Name | `PropertySetGetStarted` |
| Description | This test validates setting and getting individual Composite Input properties. It loops through all supported properties and tests set and get operations after the port is in STARTED state. |
| Test Group | 02 |
| Test Case ID | 003 |
| Priority | High |

**Pre-Conditions :**
- No external composite input device required. Properties can be set/get without device connected.
- Composite Input interface must be available in State::CLOSED initially

**Dependencies :**
- None

**User Interaction :**
None

**Test Procedure :**
| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Open the Composite Input | None | Function should return valid controller handle |
| 2 | Start the Composite Input | None | Function should return `True` |
| 3 | Get supported properties from `getCapabilities()` | None | Non-empty supported properties list returned |
| 4 | For each property | Supported values from profile | Set property, then get property | Retrieved value matches set value |
| 5 | Stop the Composite Input | None | Function should return `True` |
| 6 | Close the Composite Input | None | Function should return `True` |

### Test 4
| Title | Details |
| -- | -- |
| Function Name | `PropertyMultiSetGetStarted` |
| Description | This test validates setting multiple properties simultaneously using `setPropertyMulti()` and retrieving them with `getPropertyMulti()` when the port is in STARTED state. |
| Test Group | 02 |
| Test Case ID | 004 |
| Priority | High |

**Pre-Conditions :**
- No external composite input device required. Properties can be set/get without device connected.
- Composite Input interface must be available in State::CLOSED initially

**Dependencies :**
- None

**User Interaction :**
None

**Test Procedure :**
| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Open the Composite Input | None | Function should return valid controller handle |
| 2 | Start the Composite Input | None | Function should return `True` |
| 3 | Load all supported properties from profile | profile data | Non-empty properties list |
| 4 | For each set of supported values | Supported values from profile | Set multiple properties at once using `setPropertyMulti()` | All properties set successfully |
| 5 | Get all properties using `getPropertyMulti()` | properties from step 3 | All retrieved values match set values |
| 6 | Stop the Composite Input | None | Function should return `True` |
| 7 | Close the Composite Input | None | Function should return `True` |

### Test 5
| Title | Details |
| -- | -- |
| Function Name | `ResetMetrics` |
| Description | This test verifies the `resetMetrics()` API. Opens and starts each port, reads `METRIC_*` property values before and after calling `resetMetrics()`, and verifies that `METRIC_LAST_RESET_TIMESTAMP` is updated and metric counters are zeroed. |
| Test Group | 02 |
| Test Case ID | 005 |
| Priority | High |

**Pre-Conditions :**
- No external composite input device required.
- Composite Input interface must be available in State::CLOSED initially

**Dependencies :**
- None

**User Interaction :**
None

**Test Procedure :**
| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Open the Composite Input | None | Function should return valid controller handle |
| 2 | Start the Composite Input | None | Function should return `True` |
| 3 | Read `METRIC_LAST_RESET_TIMESTAMP` before reset (if supported) | `METRIC_LAST_RESET_TIMESTAMP` | API returns successfully; record value |
| 4 | Call `resetMetrics()` on the controller | None | Function should return `True` |
| 5 | Read `METRIC_LAST_RESET_TIMESTAMP` after reset | `METRIC_LAST_RESET_TIMESTAMP` | Value should be updated and non-zero |
| 6 | Read each `METRIC_*` counter after reset | each `METRIC_*` property | All metric counter values should be 0 |
| 7 | Stop the Composite Input | None | Function should return `True` |
| 8 | Close the Composite Input | None | Function should return `True` |

### Test 6
| Title | Details |
| -- | -- |
| Function Name | `MaximumConcurrentStartedPorts` |
| Description | This test verifies that the platform enforces the maximum number of concurrently started Composite Input ports as specified by `maximumConcurrentStartedPorts`. It opens and starts the first port, then attempts to start a second port, expecting failure. After stopping the first port, the second port must start successfully. |
| Test Group | 02 |
| Test Case ID | 006 |
| Priority | High |

**Pre-Conditions :**
- Profile declares `maximumConcurrentStartedPorts = 1`
- At least 2 ports must be available
- Composite Input interface must be available in State::CLOSED initially

**Dependencies :**
- None

**User Interaction :**
None

**Test Procedure :**
| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Read `maximumConcurrentStartedPorts` from profile | None | Value should be 1 |
| 2 | Open and Start `port[0]` | `portIds[0]` | Function should return valid controller handle and port in STARTED state |
| 3 | Open `port[1]` | `portIds[1]` | Function should return valid controller handle; port in READY state |
| 4 | Attempt to Start `port[1]` | controller 1 | Start should fail with `EX_ILLEGAL_STATE`; maximum concurrent limit exceeded |
| 5 | Verify `port[1]` state is still READY | None | State should remain READY |
| 6 | Stop `port[0]` | controller 0 | Function should return `True`; port[0] returns to READY |
| 7 | Start `port[1]` | controller 1 | Function should return `True`; port[1] transitions to STARTED |
| 8 | Stop `port[1]` and Close both ports | controllers 0, 1 | Function should return `True`; both ports return to CLOSED |
