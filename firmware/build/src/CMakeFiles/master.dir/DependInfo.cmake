# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "ASM"
  "C"
  "CXX"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_ASM
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_divider/divider.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_divider/divider.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_irq/irq_handler_chain.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_irq/irq_handler_chain.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_bit_ops/bit_ops_aeabi.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_bit_ops/bit_ops_aeabi.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_crt0/crt0.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_crt0/crt0.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_divider/divider_hardware.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_divider/divider_hardware.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_aeabi_rp2040.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_aeabi_rp2040.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_v1_rom_shim_rp2040.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_v1_rom_shim_rp2040.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_aeabi_rp2040.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_aeabi_rp2040.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_v1_rom_shim_rp2040.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_v1_rom_shim_rp2040.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_int64_ops/pico_int64_ops_aeabi.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_int64_ops/pico_int64_ops_aeabi.S.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_mem_ops/mem_ops_aeabi.S" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_mem_ops/mem_ops_aeabi.S.obj"
  )
set(CMAKE_ASM_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_ASM
  "CFG_TUSB_DEBUG=1"
  "CFG_TUSB_MCU=OPT_MCU_RP2040"
  "CFG_TUSB_OS=OPT_OS_PICO"
  "LIB_PICO_ATOMIC=1"
  "LIB_PICO_BIT_OPS=1"
  "LIB_PICO_BIT_OPS_PICO=1"
  "LIB_PICO_CLIB_INTERFACE=1"
  "LIB_PICO_CRT0=1"
  "LIB_PICO_CXX_OPTIONS=1"
  "LIB_PICO_DIVIDER=1"
  "LIB_PICO_DIVIDER_HARDWARE=1"
  "LIB_PICO_DOUBLE=1"
  "LIB_PICO_DOUBLE_PICO=1"
  "LIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1"
  "LIB_PICO_FLOAT=1"
  "LIB_PICO_FLOAT_PICO=1"
  "LIB_PICO_INT64_OPS=1"
  "LIB_PICO_INT64_OPS_PICO=1"
  "LIB_PICO_MALLOC=1"
  "LIB_PICO_MEM_OPS=1"
  "LIB_PICO_MEM_OPS_PICO=1"
  "LIB_PICO_NEWLIB_INTERFACE=1"
  "LIB_PICO_PLATFORM=1"
  "LIB_PICO_PLATFORM_COMPILER=1"
  "LIB_PICO_PLATFORM_PANIC=1"
  "LIB_PICO_PLATFORM_SECTIONS=1"
  "LIB_PICO_PRINTF=1"
  "LIB_PICO_PRINTF_PICO=1"
  "LIB_PICO_RUNTIME=1"
  "LIB_PICO_RUNTIME_INIT=1"
  "LIB_PICO_STANDARD_BINARY_INFO=1"
  "LIB_PICO_STANDARD_LINK=1"
  "LIB_PICO_STDIO=1"
  "LIB_PICO_STDIO_USB=1"
  "LIB_PICO_STDLIB=1"
  "LIB_PICO_SYNC=1"
  "LIB_PICO_SYNC_CRITICAL_SECTION=1"
  "LIB_PICO_SYNC_MUTEX=1"
  "LIB_PICO_SYNC_SEM=1"
  "LIB_PICO_TIME=1"
  "LIB_PICO_TIME_ADAPTER=1"
  "LIB_PICO_UNIQUE_ID=1"
  "LIB_PICO_UTIL=1"
  "PICO_32BIT=1"
  "PICO_BOARD=\"pico\""
  "PICO_BUILD=1"
  "PICO_CMAKE_BUILD_TYPE=\"Debug\""
  "PICO_COPY_TO_RAM=0"
  "PICO_CXX_ENABLE_EXCEPTIONS=0"
  "PICO_NO_FLASH=0"
  "PICO_NO_HARDWARE=0"
  "PICO_ON_DEVICE=1"
  "PICO_RP2040=1"
  "PICO_RP2040_USB_DEVICE_UFRAME_FIX=1"
  "PICO_TARGET_NAME=\"master\""
  "PICO_USE_BLOCKED_RAM=0"
  "SCPI_USER_CONFIG=1"
  )

# The include file search paths:
set(CMAKE_ASM_TARGET_INCLUDE_PATH
  "src/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_atomic/include"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src"
  "../src/./include"
  "../src/pico_lib2/src/sys/include"
  "../src/pico_lib2/src/dev/dev_ina219"
  "../src/pico_lib2/src/dev/dev_mcp4725"
  "../src/pico_lib2/src/dev/dev_24lc32"
  "../src/pico_lib2/src/dev/dev_ds2431"
  "/home/pi/pico/pico-sdk/src/common/pico_stdlib_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_gpio/include"
  "/home/pi/pico/pico-sdk/src/common/pico_base_headers/include"
  "generated/pico_base"
  "/home/pi/pico/pico-sdk/src/boards/include"
  "/home/pi/pico/pico-sdk/src/rp2040/pico_platform/include"
  "/home/pi/pico/pico-sdk/src/rp2040/hardware_regs/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_base/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_compiler/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_panic/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_sections/include"
  "/home/pi/pico/pico-sdk/src/rp2040/hardware_structs/include"
  "/home/pi/pico/pico-sdk/src/common/hardware_claim/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync_spin_lock/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_irq/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_uart/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_resets/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_clocks/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_pll/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_vreg/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_watchdog/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_ticks/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_xosc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_divider/include"
  "/home/pi/pico/pico-sdk/src/common/pico_time/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_timer/include"
  "/home/pi/pico/pico-sdk/src/common/pico_sync/include"
  "/home/pi/pico/pico-sdk/src/common/pico_util/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_time_adapter/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/include"
  "/home/pi/pico/pico-sdk/src/common/pico_bit_ops_headers/include"
  "/home/pi/pico/pico-sdk/src/common/pico_divider_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_double/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_float/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_malloc/include"
  "/home/pi/pico/pico-sdk/src/common/pico_binary_info/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_printf/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_unique_id/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_flash/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_bootrom/include"
  "/home/pi/pico/pico-sdk/src/common/boot_picoboot_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_boot_lock/include"
  "/home/pi/pico/pico-sdk/src/common/pico_usb_reset_interface_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_int64_ops/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_mem_ops/include"
  "/home/pi/pico/pico-sdk/src/rp2040/boot_stage2/include"
  "/home/pi/pico/pico-sdk/src/common/boot_picobin_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_fix/rp2040_usb_device_enumeration/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_adc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_i2c/include"
  "../src/lib/scpi-parser/libscpi/inc"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_rtc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_spi/include"
  )
set(CMAKE_DEPENDS_CHECK_C
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/fts_scpi.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/fts_scpi.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/functadv.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/functadv.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/audio/audio_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/audio/audio_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/cdc/cdc_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/cdc/cdc_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/dfu/dfu_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/dfu/dfu_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/dfu/dfu_rt_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/dfu/dfu_rt_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/hid/hid_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/hid/hid_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/midi/midi_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/midi/midi_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/msc/msc_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/msc/msc_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/net/ecm_rndis_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/net/ecm_rndis_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/net/ncm_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/net/ncm_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/usbtmc/usbtmc_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/usbtmc/usbtmc_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/vendor/vendor_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/vendor/vendor_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/class/video/video_device.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/class/video/video_device.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/common/tusb_fifo.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/common/tusb_fifo.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/device/usbd.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/device/usbd.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/device/usbd_control.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/device/usbd_control.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/rp2040_usb.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/portable/raspberrypi/rp2040/rp2040_usb.c.obj"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src/tusb.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/lib/tinyusb/src/tusb.c.obj"
  "/home/pi/pico/pico-sdk/src/common/hardware_claim/claim.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/hardware_claim/claim.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_sync/critical_section.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_sync/critical_section.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_sync/lock_core.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_sync/lock_core.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_sync/mutex.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_sync/mutex.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_sync/sem.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_sync/sem.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_time/time.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_time/time.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_time/timeout_helper.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_time/timeout_helper.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_util/datetime.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_util/datetime.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_util/pheap.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_util/pheap.c.obj"
  "/home/pi/pico/pico-sdk/src/common/pico_util/queue.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/common/pico_util/queue.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2040/pico_platform/platform.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2040/pico_platform/platform.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_adc/adc.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_adc/adc.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_boot_lock/boot_lock.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_boot_lock/boot_lock.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_clocks/clocks.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_clocks/clocks.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_flash/flash.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_flash/flash.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_gpio/gpio.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_gpio/gpio.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_i2c/i2c.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_i2c/i2c.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_irq/irq.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_irq/irq.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_pll/pll.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_pll/pll.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_rtc/rtc.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_rtc/rtc.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_spi/spi.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_spi/spi.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync/sync.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync/sync.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync_spin_lock/sync_spin_lock.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync_spin_lock/sync_spin_lock.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_ticks/ticks.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_ticks/ticks.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_timer/timer.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_timer/timer.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_uart/uart.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_uart/uart.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_vreg/vreg.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_vreg/vreg.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_watchdog/watchdog.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_watchdog/watchdog.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_xosc/xosc.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/hardware_xosc/xosc.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_atomic/atomic.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_atomic/atomic.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_bootrom/bootrom.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_bootrom/bootrom.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_bootrom/bootrom_lock.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_bootrom/bootrom_lock.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_clib_interface/newlib_interface.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_clib_interface/newlib_interface.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_init_rom_rp2040.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_init_rom_rp2040.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_math.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_double/double_math.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_fix/rp2040_usb_device_enumeration/rp2040_usb_device_enumeration.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_fix/rp2040_usb_device_enumeration/rp2040_usb_device_enumeration.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_init_rom_rp2040.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_init_rom_rp2040.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_math.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_float/float_math.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_malloc/malloc.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_malloc/malloc.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_panic/panic.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_panic/panic.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_printf/printf.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_printf/printf.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime/runtime.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime/runtime.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/runtime_init.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/runtime_init.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/runtime_init_clocks.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/runtime_init_clocks.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/runtime_init_stack_guard.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/runtime_init_stack_guard.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_standard_binary_info/standard_binary_info.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_standard_binary_info/standard_binary_info.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio/stdio.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio/stdio.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/reset_interface.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/reset_interface.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/stdio_usb.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/stdio_usb.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/stdio_usb_descriptors.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/stdio_usb_descriptors.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdlib/stdlib.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_stdlib/stdlib.c.obj"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_unique_id/unique_id.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_unique_id/unique_id.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/i2c_com.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/i2c_com.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/master.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/master.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/pico_lib2/src/dev/dev_24lc32/dev_24lc32.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/pico_lib2/src/dev/dev_24lc32/dev_24lc32.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/pico_lib2/src/dev/dev_ds2431/dev_ds2431.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/pico_lib2/src/dev/dev_ds2431/dev_ds2431.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/pico_lib2/src/dev/dev_ina219/dev_ina219.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/pico_lib2/src/dev/dev_ina219/dev_ina219.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/pico_lib2/src/sys/sys_adc.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/pico_lib2/src/sys/sys_adc.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/pico_lib2/src/sys/sys_i2c.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/pico_lib2/src/sys/sys_i2c.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/scpi_i2c.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/scpi_i2c.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/scpi_spi.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/scpi_spi.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/scpi_uart.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/scpi_uart.c.obj"
  "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/src/test.c" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/test.c.obj"
  )
set(CMAKE_C_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_C
  "CFG_TUSB_DEBUG=1"
  "CFG_TUSB_MCU=OPT_MCU_RP2040"
  "CFG_TUSB_OS=OPT_OS_PICO"
  "LIB_PICO_ATOMIC=1"
  "LIB_PICO_BIT_OPS=1"
  "LIB_PICO_BIT_OPS_PICO=1"
  "LIB_PICO_CLIB_INTERFACE=1"
  "LIB_PICO_CRT0=1"
  "LIB_PICO_CXX_OPTIONS=1"
  "LIB_PICO_DIVIDER=1"
  "LIB_PICO_DIVIDER_HARDWARE=1"
  "LIB_PICO_DOUBLE=1"
  "LIB_PICO_DOUBLE_PICO=1"
  "LIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1"
  "LIB_PICO_FLOAT=1"
  "LIB_PICO_FLOAT_PICO=1"
  "LIB_PICO_INT64_OPS=1"
  "LIB_PICO_INT64_OPS_PICO=1"
  "LIB_PICO_MALLOC=1"
  "LIB_PICO_MEM_OPS=1"
  "LIB_PICO_MEM_OPS_PICO=1"
  "LIB_PICO_NEWLIB_INTERFACE=1"
  "LIB_PICO_PLATFORM=1"
  "LIB_PICO_PLATFORM_COMPILER=1"
  "LIB_PICO_PLATFORM_PANIC=1"
  "LIB_PICO_PLATFORM_SECTIONS=1"
  "LIB_PICO_PRINTF=1"
  "LIB_PICO_PRINTF_PICO=1"
  "LIB_PICO_RUNTIME=1"
  "LIB_PICO_RUNTIME_INIT=1"
  "LIB_PICO_STANDARD_BINARY_INFO=1"
  "LIB_PICO_STANDARD_LINK=1"
  "LIB_PICO_STDIO=1"
  "LIB_PICO_STDIO_USB=1"
  "LIB_PICO_STDLIB=1"
  "LIB_PICO_SYNC=1"
  "LIB_PICO_SYNC_CRITICAL_SECTION=1"
  "LIB_PICO_SYNC_MUTEX=1"
  "LIB_PICO_SYNC_SEM=1"
  "LIB_PICO_TIME=1"
  "LIB_PICO_TIME_ADAPTER=1"
  "LIB_PICO_UNIQUE_ID=1"
  "LIB_PICO_UTIL=1"
  "PICO_32BIT=1"
  "PICO_BOARD=\"pico\""
  "PICO_BUILD=1"
  "PICO_CMAKE_BUILD_TYPE=\"Debug\""
  "PICO_COPY_TO_RAM=0"
  "PICO_CXX_ENABLE_EXCEPTIONS=0"
  "PICO_NO_FLASH=0"
  "PICO_NO_HARDWARE=0"
  "PICO_ON_DEVICE=1"
  "PICO_RP2040=1"
  "PICO_RP2040_USB_DEVICE_UFRAME_FIX=1"
  "PICO_TARGET_NAME=\"master\""
  "PICO_USE_BLOCKED_RAM=0"
  "SCPI_USER_CONFIG=1"
  )

# The include file search paths:
set(CMAKE_C_TARGET_INCLUDE_PATH
  "src/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_atomic/include"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src"
  "../src/./include"
  "../src/pico_lib2/src/sys/include"
  "../src/pico_lib2/src/dev/dev_ina219"
  "../src/pico_lib2/src/dev/dev_mcp4725"
  "../src/pico_lib2/src/dev/dev_24lc32"
  "../src/pico_lib2/src/dev/dev_ds2431"
  "/home/pi/pico/pico-sdk/src/common/pico_stdlib_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_gpio/include"
  "/home/pi/pico/pico-sdk/src/common/pico_base_headers/include"
  "generated/pico_base"
  "/home/pi/pico/pico-sdk/src/boards/include"
  "/home/pi/pico/pico-sdk/src/rp2040/pico_platform/include"
  "/home/pi/pico/pico-sdk/src/rp2040/hardware_regs/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_base/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_compiler/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_panic/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_sections/include"
  "/home/pi/pico/pico-sdk/src/rp2040/hardware_structs/include"
  "/home/pi/pico/pico-sdk/src/common/hardware_claim/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync_spin_lock/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_irq/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_uart/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_resets/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_clocks/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_pll/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_vreg/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_watchdog/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_ticks/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_xosc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_divider/include"
  "/home/pi/pico/pico-sdk/src/common/pico_time/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_timer/include"
  "/home/pi/pico/pico-sdk/src/common/pico_sync/include"
  "/home/pi/pico/pico-sdk/src/common/pico_util/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_time_adapter/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/include"
  "/home/pi/pico/pico-sdk/src/common/pico_bit_ops_headers/include"
  "/home/pi/pico/pico-sdk/src/common/pico_divider_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_double/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_float/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_malloc/include"
  "/home/pi/pico/pico-sdk/src/common/pico_binary_info/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_printf/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_unique_id/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_flash/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_bootrom/include"
  "/home/pi/pico/pico-sdk/src/common/boot_picoboot_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_boot_lock/include"
  "/home/pi/pico/pico-sdk/src/common/pico_usb_reset_interface_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_int64_ops/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_mem_ops/include"
  "/home/pi/pico/pico-sdk/src/rp2040/boot_stage2/include"
  "/home/pi/pico/pico-sdk/src/common/boot_picobin_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_fix/rp2040_usb_device_enumeration/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_adc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_i2c/include"
  "../src/lib/scpi-parser/libscpi/inc"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_rtc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_spi/include"
  )
set(CMAKE_DEPENDS_CHECK_CXX
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_cxx_options/new_delete.cpp" "/home/pi/pico/InterconnectIO/InterconnectIO_Master/firmware/build/src/CMakeFiles/master.dir/home/pi/pico/pico-sdk/src/rp2_common/pico_cxx_options/new_delete.cpp.obj"
  )
set(CMAKE_CXX_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_CXX
  "CFG_TUSB_DEBUG=1"
  "CFG_TUSB_MCU=OPT_MCU_RP2040"
  "CFG_TUSB_OS=OPT_OS_PICO"
  "LIB_PICO_ATOMIC=1"
  "LIB_PICO_BIT_OPS=1"
  "LIB_PICO_BIT_OPS_PICO=1"
  "LIB_PICO_CLIB_INTERFACE=1"
  "LIB_PICO_CRT0=1"
  "LIB_PICO_CXX_OPTIONS=1"
  "LIB_PICO_DIVIDER=1"
  "LIB_PICO_DIVIDER_HARDWARE=1"
  "LIB_PICO_DOUBLE=1"
  "LIB_PICO_DOUBLE_PICO=1"
  "LIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1"
  "LIB_PICO_FLOAT=1"
  "LIB_PICO_FLOAT_PICO=1"
  "LIB_PICO_INT64_OPS=1"
  "LIB_PICO_INT64_OPS_PICO=1"
  "LIB_PICO_MALLOC=1"
  "LIB_PICO_MEM_OPS=1"
  "LIB_PICO_MEM_OPS_PICO=1"
  "LIB_PICO_NEWLIB_INTERFACE=1"
  "LIB_PICO_PLATFORM=1"
  "LIB_PICO_PLATFORM_COMPILER=1"
  "LIB_PICO_PLATFORM_PANIC=1"
  "LIB_PICO_PLATFORM_SECTIONS=1"
  "LIB_PICO_PRINTF=1"
  "LIB_PICO_PRINTF_PICO=1"
  "LIB_PICO_RUNTIME=1"
  "LIB_PICO_RUNTIME_INIT=1"
  "LIB_PICO_STANDARD_BINARY_INFO=1"
  "LIB_PICO_STANDARD_LINK=1"
  "LIB_PICO_STDIO=1"
  "LIB_PICO_STDIO_USB=1"
  "LIB_PICO_STDLIB=1"
  "LIB_PICO_SYNC=1"
  "LIB_PICO_SYNC_CRITICAL_SECTION=1"
  "LIB_PICO_SYNC_MUTEX=1"
  "LIB_PICO_SYNC_SEM=1"
  "LIB_PICO_TIME=1"
  "LIB_PICO_TIME_ADAPTER=1"
  "LIB_PICO_UNIQUE_ID=1"
  "LIB_PICO_UTIL=1"
  "PICO_32BIT=1"
  "PICO_BOARD=\"pico\""
  "PICO_BUILD=1"
  "PICO_CMAKE_BUILD_TYPE=\"Debug\""
  "PICO_COPY_TO_RAM=0"
  "PICO_CXX_ENABLE_EXCEPTIONS=0"
  "PICO_NO_FLASH=0"
  "PICO_NO_HARDWARE=0"
  "PICO_ON_DEVICE=1"
  "PICO_RP2040=1"
  "PICO_RP2040_USB_DEVICE_UFRAME_FIX=1"
  "PICO_TARGET_NAME=\"master\""
  "PICO_USE_BLOCKED_RAM=0"
  "SCPI_USER_CONFIG=1"
  )

# The include file search paths:
set(CMAKE_CXX_TARGET_INCLUDE_PATH
  "src/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_atomic/include"
  "/home/pi/pico/pico-sdk/lib/tinyusb/src"
  "../src/./include"
  "../src/pico_lib2/src/sys/include"
  "../src/pico_lib2/src/dev/dev_ina219"
  "../src/pico_lib2/src/dev/dev_mcp4725"
  "../src/pico_lib2/src/dev/dev_24lc32"
  "../src/pico_lib2/src/dev/dev_ds2431"
  "/home/pi/pico/pico-sdk/src/common/pico_stdlib_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_gpio/include"
  "/home/pi/pico/pico-sdk/src/common/pico_base_headers/include"
  "generated/pico_base"
  "/home/pi/pico/pico-sdk/src/boards/include"
  "/home/pi/pico/pico-sdk/src/rp2040/pico_platform/include"
  "/home/pi/pico/pico-sdk/src/rp2040/hardware_regs/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_base/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_compiler/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_panic/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_platform_sections/include"
  "/home/pi/pico/pico-sdk/src/rp2040/hardware_structs/include"
  "/home/pi/pico/pico-sdk/src/common/hardware_claim/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_sync_spin_lock/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_irq/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_uart/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_resets/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_clocks/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_pll/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_vreg/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_watchdog/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_ticks/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_xosc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_divider/include"
  "/home/pi/pico/pico-sdk/src/common/pico_time/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_timer/include"
  "/home/pi/pico/pico-sdk/src/common/pico_sync/include"
  "/home/pi/pico/pico-sdk/src/common/pico_util/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_time_adapter/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_runtime_init/include"
  "/home/pi/pico/pico-sdk/src/common/pico_bit_ops_headers/include"
  "/home/pi/pico/pico-sdk/src/common/pico_divider_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_double/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_float/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_malloc/include"
  "/home/pi/pico/pico-sdk/src/common/pico_binary_info/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_printf/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_stdio_usb/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_unique_id/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_flash/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_bootrom/include"
  "/home/pi/pico/pico-sdk/src/common/boot_picoboot_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_boot_lock/include"
  "/home/pi/pico/pico-sdk/src/common/pico_usb_reset_interface_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_int64_ops/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_mem_ops/include"
  "/home/pi/pico/pico-sdk/src/rp2040/boot_stage2/include"
  "/home/pi/pico/pico-sdk/src/common/boot_picobin_headers/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/pico_fix/rp2040_usb_device_enumeration/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_adc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_i2c/include"
  "../src/lib/scpi-parser/libscpi/inc"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_rtc/include"
  "/home/pi/pico/pico-sdk/src/rp2_common/hardware_spi/include"
  )

# Targets to which this target links.
set(CMAKE_TARGET_LINKED_INFO_FILES
  )

# Fortran module output directory.
set(CMAKE_Fortran_TARGET_MODULE_DIR "")
