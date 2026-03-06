# USB Communication Test - STM32N6

## Overview

This implementation adds USB host and device communication tests to the STM32N6 project, similar to what's done on the ESP32. Two FreeRTOS tasks run in parallel to test bidirectional USB communication.

## Architecture

### Components Added:

1. **USB Device Task** (`usb_device_test_task`)
   - Waits for host connection
   - Receives data and sends echo back
   - Periodically sends test messages
   - Logs connection state transitions

2. **USB Host Task** (`usb_host_test_task`)
   - Waits for device connection
   - Sends test messages at intervals
   - Receives and displays echo responses
   - Logs connection state transitions

3. **Helper Functions** (in `app_usbx.c`)
   - `usb_device_send()` - Send data via USB device
   - `usb_device_receive()` - Receive data on USB device
   - `usb_device_is_connected()` - Check device connection status
   - `usb_host_send()` - Send data via USB host
   - `usb_host_receive()` - Receive data on USB host
   - `usb_host_is_connected()` - Check host connection status

4. **CDC ACM Integration**
   - Device CDC ACM implementation in `ux_device_cdc_acm.c/h`
   - Host CDC ACM implementation in `ux_host_cdc_acm.c/h`
   - Callbacks for device connection/disconnection events
   - Global instance management for task access

## Files Modified

```
Appli/
├── Core/Src/app_freertos.c          - Added USB test tasks
├── USBX/App/
│   ├── app_usbx.c                   - Helper functions
│   ├── app_usbx.h                   - Helper function prototypes
│   ├── app_usbx_device.c            - CDC ACM registration
│   ├── ux_device_cdc_acm.c          - Device CDC ACM callbacks
│   ├── ux_device_cdc_acm.h          - Device CDC ACM interface
│   ├── app_usbx_host.c              - HCD registration
│   ├── ux_host_cdc_acm.c            - Host CDC ACM callbacks
│   └── ux_host_cdc_acm.h            - Host CDC ACM interface
```

## Test Output

When compiled and running, you should see console output like:

```
[USB_DEV] ✓ Host connected
[USB_DEV] RX [20 bytes]: [HOST] Test message 1
[USB_DEV] ✓ Echo sent [20 bytes]
[USB_DEV] ✓ TX: [DEV] Test message 1

[USB_HOST] ✓ Device connected
[USB_HOST] ✓ TX: [HOST] Test message 1
[USB_HOST] RX [20 bytes]: [DEV] Test message 1
```

## Communication Flow

### Device Side:
1. Task starts monitoring USB connection
2. When host connects, device is ready to receive
3. Device receives any data and echoes it back
4. Every 100ms, task checks for new data
5. Every ~1 second, sends a periodic test message

### Host Side:
1. Task starts monitoring USB device connection
2. When device connects, host is ready to communicate
3. Every 2 seconds, host sends a numbered test message
4. After each send, waits to receive echo response
5. Displays all received data

## Expected Behavior

### Device Test:
- Listens for incoming data from host
- Echoes received data back immediately
- Sends periodic `[DEV] Test message X` messages
- Maintains activity log with timestamps

### Host Test:
- Initiates communication by sending `[HOST] Test message X`
- Waits for and displays echo from device
- Connection state is monitored (connect/disconnect events)
- Retry delay if device not connected

## Building and Testing

1. **Compile the project:**
   ```bash
   make -C Makefile/Appli
   ```

2. **Flash to device:**
   ```bash
   ./flash_windows.ps1
   ```

3. **Monitor Output:**
   - Use serial monitor at 115200 baud (or configured UART)
   - Or use USB-to-UART bridge if available

4. **Test Communication:**
   - Connect two STM32N6 boards (or equivalent USB device)
   - One board acts as device (connected to host)
   - Other board acts as host (detects device)
   - Monitor console output for test messages

## Data Flow Example

```
Host: [HOST] Test message 1
              ↓
Device: Receives → Echoes
Device: [DEV] Test message 1
              ↓
Host: [HOST] RX: [DEV] Test message 1
```

## Customization

### Modify Test Intervals
Edit `app_freertos.c`:
- Device send interval: Line with `tick_count++`
- Host send interval: `osDelay(2000)` in `usb_host_test_task`

### Change Message Format
Modify format strings in test tasks:
```c
snprintf((char*)tx_buf, sizeof(tx_buf), "[DEV] Test message %ld\r\n", counter);
```

### Enable/Disable Tasks
Comment out task creation in `MX_FREERTOS_Init()`:
```c
// osThreadNew(usb_device_test_task, NULL, &defaultTask_attributes);
// osThreadNew(usb_host_test_task, NULL, &defaultTask_attributes);
```

## Debugging Tips

1. **Check connection status** using the helper functions:
   ```c
   if (usb_device_is_connected()) {
       // Device is ready
   }
   ```

2. **Monitor buffer operations:**
   - Verify buffer sizes in task definitions (256 bytes)
   - Check timeout values in receive calls

3. **CDC ACM Instance Debug:**
   - Use breakpoints in `ux_device_cdc_acm_get_instance()`
   - Verify instance is not NULL before operations

4. **Enable/Disable Logging:**
   - Comment out `printf()` statements to reduce output
   - Or redirect to separate debug UART if available

## Notes

- Tasks run with normal priority (osPriorityNormal)
- Default task is still active (LED blink pattern)
- USB operations are non-blocking with timeouts
- Both tasks use 128×4 bytes of stack (configurable)
- LED task continues independently for status indication
