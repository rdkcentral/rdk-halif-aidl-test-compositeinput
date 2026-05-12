# Composite Input High-Level Test Specification Document


## Table of Contents

 - [Acronyms, Terms, and Abbreviations](#acronyms-terms-and-abbreviations)
 - [Introduction](#introduction)
 - [L2 Test Specification](#l2-test-specification)
 - [L3 Test Specification](#l3-test-specification)


## Acronyms, Terms, and Abbreviations
 
- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `AIDL`   - Android Interface Definition Language
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `HFP`    - Hardware Feature Profile
- `DUT`    - Device Under Test
- `NA`     - Not Applicable


## Introduction

This document provides a comprehensive high-level test specification for the Composite Input `HAL`. It details the `L2` and `L3` test cases designed to validate the functionality, reliability, and compliance of the Composite Input `HAL` according to the defined interface and `HFP`.

- **Composite Input Interface Definitions:** [Composite Input HAL](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/compositeinput/current/com/rdk/hal/compositeinput/)
- **Composite Input HFP:** [HFP](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/compositeinput/current/hfp-compositeinput.yaml)


## L2 Test Specification

| Test | Description | HAL APIs |
|------|-------------|----------|
|Verify the state of Composite Input port|<ul><li>Loop through supported input ports.</li><li>Register event listeners using registerEventListener</li><li>Ensure state transitions (open -> start -> stop -> close) work as expected using getState(). <ul><li>During open(): CLOSED->OPENING->READY</li><li>During start(): READY->STARTING->STARTED</li><li>During stop(): STARTED->STOPPING->READY</li><li>During close(): READY->CLOSING->CLOSED</li></ul></li> <li> Validate event listener notifications using onStateChanged() </li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getState()`, `registerEventListener()`, `unregisterEventListener()`, `close()`</li><li>`ICompositeInputController`: `start()`, `stop()`</li><li>`ICompositeInputEventListener`: `onStateChanged()`</li></ul>|
|Verify connection state of each CompositeInput port |<ul><li> Loop through supported input port </li><li>Verify the current connection state for a device attached to the composite Input port </li><li> connection state should be FALSE and signalStatus as NO_SIGNAL in getStatus() and callback</li></ul> |<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getStatus()`, `close()`</li><li>`ICompositeInputController`: `start()`, `stop()`</li><li>`ICompositeInputControllerListener`: `onConnectionChanged()`, `onSignalStatusChanged()`</li></ul>|
|Verify individual property set/get |<ul><li>Loop through supported input ports.</li><li>Verifies individual composite input property set and get operation after READY/STARTED of the input port instance</li><li>Property:SIGNAL_STRENGTH, SIGNAL_QUALITY, METRIC_SIGNAL_LOCK_TIME, METRIC_UPTIME, METRIC_LAST_RESET_TIMESTAMP etc</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getProperty()`, `close()`</li><li>`ICompositeInputController`: `start()`, `setProperty()` `stop()`</li></ul>|
|Verify multi-property set/get |<ul><li>Loop through supported input ports.</li><li>Verifies multiple property-value pairs set and get operation after READY/STARTED of the input port instance</li><li>Property:SIGNAL_STRENGTH, SIGNAL_QUALITY, METRIC_SIGNAL_LOCK_TIME, METRIC_UPTIME, METRIC_LAST_RESET_TIMESTAMP etc</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getPropertyMulti()`, `close()`</li><li>`ICompositeInputController`: `start()`, `setPropertyMulti()` `stop()`</li></ul>|
|Verify `resetMetrics()`|<ul><li>Loop through supported input ports.</li><li>Open and start the port.</li><li>Read telemetry metric properties (`METRIC_SIGNAL_DROPS`, `METRIC_SIGNAL_LOCK_COUNT`, `METRIC_LAST_RESET_TIMESTAMP` etc.) before and after calling `resetMetrics()`.</li><li>Verify all `METRIC_*` values are reset and `METRIC_LAST_RESET_TIMESTAMP` is updated.</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getProperty()`, `close()`</li><li>`ICompositeInputController`: `start()`, `stop()`, `resetMetrics()`</li></ul>|
|Verify `maximumConcurrentStartedPorts`|<ul><li>Open and `start()` one port.</li><li>Try to `open()` and `start()` a second port; `start()` should fail with `EX_ILLEGAL_STATE` since `maximumConcurrentStartedPorts` is 1 per the `HFP`.</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `close()`</li><li>`ICompositeInputController`: `start()`, `stop()`</li></ul>|


## L3 Test Specification

| Test | Description | HAL APIs |
|------|-------------|----------|
|Verify connection state|<ul><li>Loop through supported input ports.</li><li>Verify callback is triggered once during state OPENING</li><li>Verify the connection state in callback is FALSE when state is OPENING and composite input device is not connected</li><li>Verify the connection state in callback is TRUE when state is OPENING and composite input device is already connected</li><li>Verify Callback is fired on HPD events after state is READY</li><li>Verify the connection state is TRUE in callback on HPD plug event when the input port is READY/STARTING/STARTED/STOPPING state</li><li>Verify the connection state is FALSE in callback on HPD unplug event when the input port is READY/STARTING/STARTED/STOPPING state</li><li>Verify the connection state is TRUE in callback on powerON of composite input device when the input port is READY/STARTING/STARTED/STOPPING state</li><li>Verify the connection state is FALSE in callback on powerOFF of composite input device when the input port is READY/STARTING/STARTED/STOPPING state</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getStatus()`, `close()`</li><li>`ICompositeInputController`: `start()`, `stop()`</li><li>`ICompositeInputControllerListener`: `onConnectionChanged()`</li></ul>|
|Verify signal status|<ul><li>Loop through supported input ports.</li><li>Verify callback is triggered once during state STARTING</li><li>Verify the signal state in callback is NO_SIGNAL when state is STARTING and composite input device is not connected</li><li>Verify the signal state in callback is STABLE when state is STARTING and composite input device is already connected</li><li>Verify Callback is fired on signal change events after state is STARTED</li><li>Verify the signal state NO_SIGNAL->UNSTABLE->STABLE in callback on HPD plug event when the input port is STARTED </li><li>Verify the signal state STABLE->NO_SIGNAL in callback on HPD unplug event when the input port is STARTED </li><li>Verify the signal state NO_SIGNAL->UNSTABLE->STABLE in callback on powerON event of composite input device when the input port is STARTED </li><li>Verify the signal state STABLE->NO_SIGNAL in callback on powerOFF/standby event of composite input device when the input port is STARTED </li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getStatus()`, `close()`</li><li>`ICompositeInputController`: `start()`, `stop()`</li><li>`ICompositeInputControllerListener`: `onSignalStatusChanged()`</li></ul>|
|Verify video mode detection|<ul><li>Loop through supported input ports.</li><li>Connect a composite source that outputs a known video mode to the input port.</li><li>Call `open()` and `start()` on the input port and ensure `onVideoModeChanged()` is fired with the expected `VideoResolution` (resolution, format, frame rate) once signal is STABLE.</li><li>Switch the source video mode and verify `onVideoModeChanged()` fires with the updated resolution.</li><li>Validate the reported mode matches `getStatus().videoResolution`.</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getStatus()`, `close()`</li><li>`ICompositeInputController`: `start()`, `stop()`</li><li>`ICompositeInputControllerListener`: `onVideoModeChanged()`</li></ul>|
|Verify property set/get applied on device|<ul><li>Loop through supported input ports.</li><li>Connect an active composite source.</li><li>set the property value after READY/STARTED state of composite input port instance</li><li>Property:SIGNAL_STRENGTH, SIGNAL_QUALITY, METRIC_SIGNAL_LOCK_TIME, METRIC_UPTIME, METRIC_LAST_RESET_TIMESTAMP etc</li><li>Verify `onPropertyChanged()` callback is fired when values change.</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getProperty()`, `getPropertyMulti()`, `registerEventListener()`, `unregisterEventListener()`, `close()`</li><li>`ICompositeInputController`: `start()`, `setProperty()`, `setPropertyMulti()`, `stop()`</li><li>`ICompositeInputEventListener`: `onPropertyChanged()`</li></ul>|
|Verify video playback|<ul><li>Loop through supported input ports.</li><li>Connect an active composite source.</li><li>Configure audiomixer and planecontrol</li><li>Play video/audio and verify the playback</li></ul>|<ul><li>`ICompositeInputManager`: `getPortIds()`, `getPort()`</li><li>`ICompositeInputPort`: `open()`, `getProperty()`, `getPropertyMulti()`, `registerEventListener()`, `unregisterEventListener()`, `close()`</li><li>`ICompositeInputController`: `start()`, `setProperty()`, `setPropertyMulti()`, `stop()`</li><li>`ICompositeInputEventListener`: `onPropertyChanged()`</li></ul>|
