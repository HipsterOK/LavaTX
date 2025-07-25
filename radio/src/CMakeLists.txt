include(CMakeForceCompiler)
include(Bitmaps)

set(PCB_TYPES X9LITE X9LITES X7 XLITE XLITES X9D X9D+ X9E X10 X12S SKY9X 9XRPRO AR9X TANGO MAMBO)
set(GUI_LANGUAGES CZ DE EN ES FR IT PT SK SE PL HU NL)
set(TTS_LANGUAGES CZ DE EN ES FR IT PT SK SE PL HU NL RU)

set(PCB "X9D+" CACHE STRING "Radio type, one of: ${PCB_TYPES}")
set_property(CACHE PCB PROPERTY STRINGS ${PCB_TYPES})
set(TRANSLATIONS "EN" CACHE STRING "Radio language, one of: ${GUI_LANGUAGES}")
set_property(CACHE TRANSLATIONS PROPERTY STRINGS ${GUI_LANGUAGES})
set(SPLASH "DEFAULT" CACHE STRING "Splash (DEFAULT/OFF/FRSKY)")
set_property(CACHE SPLASH PROPERTY STRINGS DEFAULT OFF FRSKY)
set(PPM_UNIT "PERCENT_PREC1" CACHE STRING "PPM display unit (US/PERCENT_PREC1/PERCENT_PREC0)")
set_property(CACHE PPM_UNIT PROPERTY STRINGS US PERCENT_PREC1 PERCENT_PREC0)
set(DEFAULT_MODE "" CACHE STRING "Default sticks mode")
set(FONT "STD" CACHE STRING "Choose font : STD or SQT5")
set_property(CACHE FONT PROPERTY STRINGS SQT5)
set(POPUP_LEVEL 2 CACHE STRING "Popup level")

option(HELI "Heli menu" ON)
option(FLIGHT_MODES "Flight Modes" ON)
option(CURVES "Curves" ON)
option(GVARS "Global variables" ON)
option(GUI "GUI enabled" ON)
option(PPM_CENTER_ADJUSTABLE "PPM center adjustable" ON)
option(PPM_LIMITS_SYMETRICAL "PPM limits symetrical" OFF)
option(OVERRIDE_CHANNEL_FUNCTION "OverrideChannel function available" ON)
option(DANGEROUS_MODULE_FUNCTIONS "Dangerous module functions (RangeCheck / Bind / Module OFF, etc.) available" OFF)
option(FAI "Competition mode (no telemetry)" OFF)
option(AUTOSOURCE "Automatic source detection in menus" ON)
option(AUTOSWITCH "Automatic switch detection in menus" ON)
option(SEMIHOSTING "Enable debugger semihosting" OFF)
option(JITTER_MEASURE "Enable ADC jitter measurement" OFF)
option(WATCHDOG "Enable hardware Watchdog" ON)
option(ASTERISK "Enable asterisk icon (test only firmware)" OFF)
if(SDL_FOUND)
  option(SIMU_AUDIO "Enable simulator audio." ON)
endif()
option(LUA "Enable LUA support" ON)
option(SIMU_DISKIO "Enable disk IO simulation in simulator. Simulator will use FatFs module and simulated IO layer that  uses \"./sdcard.image\" file as image of SD card. This file must contain whole SD card from first to last sector" OFF)
option(SIMU_LUA_COMPILER "Pre-compile and save Lua scripts in simulator." ON)
option(FAS_PROTOTYPE "Support of old FAS prototypes (different resistors)" OFF)
option(RAS "RAS (SWR) enabled" ON)
option(TEMPLATES "Model templates menu" OFF)
option(TRACE_SIMPGMSPACE "Turn on traces in simpgmspace.cpp" ON)
option(TRACE_LUA_INTERNALS "Turn on traces for Lua internals" OFF)
option(FRSKY_STICKS "Reverse sticks for FrSky sticks" OFF)
option(NANO "Use nano newlib and binalloc")
option(TEST_BUILD_WARNING "Warn this is a test build" OFF)
option(MODULE_PROTOCOL_FCC "Add support for FCC modules" ON)
option(MODULE_PROTOCOL_LBT "Add support for EU/LBT modules" ON)
option(MODULE_PROTOCOL_FLEX "Add support for non certified FLEX modules" OFF)
option(MODULE_PROTOCOL_D8 "Add support for D8 modules" ON)
option(FRSKY_RELEASE "Used to build FrSky released firmware" OFF)
option(TBS_RELEASE "Used to build TBS released firmware" ON)
option(IMRC_RELEASE "Used to build IMRC released firmware" OFF)
option(HARDWARE_TRAINER_MULTI_MOD "Allow multi trainer" OFF)

# since we reset all default CMAKE compiler flags for firmware builds, provide an alternate way for user to specify additional flags.
set(FIRMWARE_C_FLAGS "" CACHE STRING "Additional flags for firmware target c compiler (note: all CMAKE_C_FLAGS[_*] are ignored for firmware/bootloader).")
set(FIRMWARE_C_FLAGS_DEBUG "-g" CACHE STRING "Additional flags for firmware target (Debug config) c compiler (note: CMAKE_C_FLAGS_DEBUG is ignored for firmware/bootloader).")
set(FIRMWARE_CXX_FLAGS "" CACHE STRING "Additional flags for firmware target c++ compiler (note: all CMAKE_CXX_FLAGS[_*] are ignored for firmware/bootloader).")
set(FIRMWARE_CXX_FLAGS_DEBUG "-g" CACHE STRING "Additional flags for firmware target (Debug config) c++ compiler (note: CMAKE_CXX_FLAGS_DEBUG is ignored for firmware/bootloader).")

set(THIRDPARTY_DIR thirdparty)
set(LUA_DIR ${THIRDPARTY_DIR}/Lua/src)
set(COOS_DIR ${THIRDPARTY_DIR}/CoOS)
set(FATFS_DIR ${THIRDPARTY_DIR}/FatFs)
set(RADIO_BIN_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

configure_file(stamp.h.in stamp.h @ONLY)

add_subdirectory(translations)
add_subdirectory(bitmaps)
add_subdirectory(fonts)

set(EEPROM_VARIANT  0)
set(GVARS_VARIANT   1)
set(FRSKY_VARIANT   2)
set(3POS_VARIANT    4)

set(RADIO_DEPENDENCIES firmware_translations)

set(FATFS_SRC
  ${FATFS_DIR}/ff.c
  ${FATFS_DIR}/fattime.c
  ${FATFS_DIR}/option/ccsbcs.c
)

if(PCB STREQUAL X12S OR PCB STREQUAL X10)
  include(targets/horus/CMakeLists.txt)
elseif(PCB STREQUAL X9E OR PCB STREQUAL X9D+ OR PCB STREQUAL X9D OR PCB STREQUAL X7 OR PCB STREQUAL X9LITE OR PCB STREQUAL X9LITES OR PCB STREQUAL XLITE OR PCB STREQUAL XLITES)
  include(targets/taranis/CMakeLists.txt)
elseif(PCB STREQUAL SKY9X OR PCB STREQUAL 9XRPRO OR PCB STREQUAL AR9X)
  include(targets/sky9x/CMakeLists.txt)
elseif(PCB STREQUAL TANGO OR PCB STREQUAL MAMBO)
  include(targets/tbs/CMakeLists.txt)
else()
  message(FATAL_ERROR "Unknown PCB '${PCB}'")
endif()

if(NOT PCB STREQUAL 9XRPRO)
  option(DBLKEYS "Double Keys" ON)
  if(DBLKEYS)
    add_definitions(-DDBLKEYS)
  endif()
endif()

if(CPU_TYPE STREQUAL STM32F4)
  include(targets/common/arm/stm32/f4/CMakeLists.txt)
endif()

if(CPU_TYPE STREQUAL STM32F2)
  include(targets/common/arm/stm32/f2/CMakeLists.txt)
endif()

if(CPU_FAMILY STREQUAL STM32)
  include(targets/common/arm/stm32/CMakeLists.txt)
endif()

if(ARCH STREQUAL ARM)
  include(targets/common/arm/CMakeLists.txt)
endif()

include_directories(targets/${TARGET_DIR} ${THIRDPARTY_DIR})

foreach(LANGUAGE ${GUI_LANGUAGES})
  if(TRANSLATIONS STREQUAL ${LANGUAGE})
    add_definitions(-DTRANSLATIONS_${LANGUAGE} -DTRANSLATIONS="${LANGUAGE}")
  endif()
endforeach()

set(SRC ${SRC} debug.cpp)

if(${EEPROM} STREQUAL SDCARD)
  set(SRC ${SRC} storage/storage_common.cpp storage/sdcard_raw.cpp storage/modelslist.cpp)
  add_definitions(-DEEPROM_SDCARD)
elseif(${EEPROM} STREQUAL EEPROM_RLC)
  set(SRC ${SRC} storage/storage_common.cpp storage/eeprom_common.cpp storage/eeprom_rlc.cpp)
  add_definitions(-DEEPROM -DEEPROM_RLC)
else()
  set(SRC ${SRC} storage/storage_common.cpp storage/eeprom_common.cpp storage/eeprom_raw.cpp)
  add_definitions(-DEEPROM -DEEPROM_RAW)
endif()

include(storage/conversions/CMakeLists.txt)

add_definitions(-DFLAVOUR="${FLAVOUR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/bitmaps/${GUI_DIR})

include(gui/CMakeLists.txt)

if(RTC_BACKUP_RAM)
  add_definitions(-DRTC_BACKUP_RAM)
  set(SRC ${SRC} storage/rlc.cpp)
  set(FIRMWARE_SRC ${FIRMWARE_SRC} storage/rtc_backup.cpp)
  set(GTEST_SRC ${GTEST_SRC} ${RADIO_SRC_DIRECTORY}/storage/rtc_backup.cpp)
endif()

if(NOT LUA STREQUAL NO)
  add_definitions(-DLUA)
  if(LUA_COMPILER)
    add_definitions(-DLUA_COMPILER)
  endif()
  if(LUA_ALLOCATOR_TRACER AND DEBUG)
    add_definitions(-DLUA_ALLOCATOR_TRACER)
  endif()
  if(NOT "${LUA_SCRIPT_LOAD_MODE}" STREQUAL "")
    add_definitions(-DLUA_SCRIPT_LOAD_MODE="${LUA_SCRIPT_LOAD_MODE}")
  endif()
  include_directories(${LUA_DIR})
  set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${LUA_EXPORT})
  if(LUA STREQUAL YES)
    add_definitions(-DLUA_MODEL_SCRIPTS)
    set(GUI_SRC ${GUI_SRC} model_custom_scripts.cpp)
  endif()
  set(SRC ${SRC} lua/interface.cpp lua/api_general.cpp lua/api_lcd.cpp lua/api_model.cpp)
  if(PCB STREQUAL X12S OR PCB STREQUAL X10)
    set(SRC ${SRC} lua/widgets.cpp)
  endif()
  set(LUA_SRC lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c
    lstate.c lstring.c ltable.c lrotable.c ltm.c lundump.c lvm.c lzio.c linit.c
    lbaselib.c lmathlib.c lbitlib.c loadlib.c lauxlib.c ltablib.c lcorolib.c liolib.c lstrlib.c
    )
  foreach(FILE ${LUA_SRC})
    set(SRC ${SRC} ${LUA_DIR}/${FILE})
  endforeach()
  if(MSVC)
    # silence lots of warnings regarding "insecure" use of functions (sprintf, strcpy, etc) from Lua code
    set(WARNING_FLAGS "${WARNING_FLAGS} /D_CRT_SECURE_NO_WARNINGS")
  endif()
endif()

if(HELI)
  add_definitions(-DHELI)
endif()

if(FLIGHT_MODES)
  add_definitions(-DFLIGHT_MODES)
  set(GUI_SRC ${GUI_SRC} model_flightmodes.cpp)
endif()

if(CURVES)
  add_definitions(-DCURVES)
  set(SRC ${SRC} curves.cpp)
endif()

if(GVARS)
  add_definitions(-DGVARS)
  set(SRC ${SRC} gvars.cpp)
  math(EXPR EEPROM_VARIANT ${EEPROM_VARIANT}+${GVARS_VARIANT})
endif()

if(FAI STREQUAL YES)
  add_definitions(-DFAI)
elseif(FAI STREQUAL CHOICE)
  add_definitions(-DFAI_CHOICE)
endif()

if(NOT RAS)
  add_definitions(-DNO_RAS)
endif()

if(FAS_PROTOTYPE)
  add_definitions(-DFAS_PROTOTYPE)
endif()

if(AUTOSOURCE)
  add_definitions(-DAUTOSOURCE)
endif()

if(AUTOSWITCH)
  add_definitions(-DAUTOSWITCH)
endif()

if(JITTER_MEASURE)
  add_definitions(-DJITTER_MEASURE)
endif()

if(ASTERISK)
  add_definitions(-DASTERISK)
endif()

if(WATCHDOG)
  add_definitions(-DWATCHDOG)
endif()

if(SIMU_AUDIO)
  add_definitions(-DSIMU_AUDIO)
endif()

if(SIMU_DISKIO)
  add_definitions(-DSIMU_DISKIO)
endif()

if(SDCARD)
  add_definitions(-DSDCARD)
  include_directories(${FATFS_DIR} ${FATFS_DIR}/option)
  set(SRC ${SRC} sdcard.cpp rtc.cpp logs.cpp)
  set(FIRMWARE_SRC ${FIRMWARE_SRC} ${FATFS_SRC})
endif()

if(SHUTDOWN_CONFIRMATION)
  add_definitions(-DSHUTDOWN_CONFIRMATION)
endif()

if(PPM_UNIT STREQUAL US)
  add_definitions(-DPPM_UNIT_US)
elseif(PPM_UNIT STREQUAL PERCENT_PREC1)
  add_definitions(-DPPM_UNIT_PERCENT_PREC1)
else()
  add_definitions(-DPPM_UNIT_PERCENT_PREC0)
endif()

if(PPM_CENTER_ADJUSTABLE)
  add_definitions(-DPPM_CENTER_ADJUSTABLE)
endif()

if(PPM_LIMITS_SYMETRICAL)
  add_definitions(-DPPM_LIMITS_SYMETRICAL)
endif()

if(OVERRIDE_CHANNEL_FUNCTION)
  add_definitions(-DOVERRIDE_CHANNEL_FUNCTION)
endif()

if(DANGEROUS_MODULE_FUNCTIONS)
  add_definitions(-DDANGEROUS_MODULE_FUNCTIONS)
endif()

if(NOT SPLASH STREQUAL OFF)
  add_definitions(-DSPLASH)
endif()

if(NOT DEFAULT_MODE STREQUAL "")
  add_definitions(-DDEFAULT_MODE=${DEFAULT_MODE})
endif()

if(TRACE_SIMPGMSPACE)
  add_definitions(-DTRACE_SIMPGMSPACE)
endif()

if(TRACE_LUA_INTERNALS)
  add_definitions(-DTRACE_LUA_INTERNALS_ENABLED)
endif()

if(FRSKY_STICKS)
  add_definitions(-DFRSKY_STICKS)
endif()

if(NOT FONT STREQUAL "")
  add_definitions(-DFONT=${DEFAULT_MODE})
endif()

if(EEPROM_VARIANT_NEEDED)
  add_definitions(-DEEPROM_VARIANT=${EEPROM_VARIANT})
endif()

if(TEST_BUILD_WARNING)
  add_definitions(-DTEST_BUILD_WARNING)
endif()

if(MODULE_PROTOCOL_FCC)
  add_definitions(-DMODULE_PROTOCOL_FCC)
endif()

if(MODULE_PROTOCOL_LBT)
  add_definitions(-DMODULE_PROTOCOL_LBT)
endif()

if(MODULE_PROTOCOL_FLEX)
  add_definitions(-DMODULE_PROTOCOL_FLEX)
endif()

if(MODULE_PROTOCOL_D8)
  add_definitions(-DMODULE_PROTOCOL_D8)
endif()

if(JUMPER_RELEASE)
  add_definitions(-DJUMPER_RELEASE)
endif()

if(RADIOMASTER_RELEASE)
  add_definitions(-DRADIOMASTER_RELEASE)
endif()

if(RADIOMASTER_RTF_RELEASE)
  add_definitions(-DRADIOMASTER_RTF_RELEASE)
endif()

if(TBS_RELEASE)
  add_definitions(-DTBS_RELEASE)
endif()

if(IMRC_RELEASE)
  add_definitions(-DIMRC_RELEASE)
endif()

if(FRSKY_RELEASE)
  add_definitions(-DFRSKY_RELEASE)
  set(POPUP_LEVEL 3)
endif()

if(HARDWARE_TRAINER_MULTI_MOD)
  add_definitions(-DHARDWARE_TRAINER_MULTI_MOD)
endif()

add_definitions(-DPOPUP_LEVEL=${POPUP_LEVEL})

if(INTERNAL_MODULE_MULTI)
  set(DEFAULT_TEMPLATE_SETUP 21 CACHE STRING "")
else()
  set(DEFAULT_TEMPLATE_SETUP 0 CACHE STRING "")
endif()
add_definitions(-DDEFAULT_TEMPLATE_SETUP=${DEFAULT_TEMPLATE_SETUP})

set(SRC
  ${SRC}
  opentx.cpp
  functions.cpp
  strhelpers.cpp
  switches.cpp
  mixer.cpp
  mixer_scheduler.cpp
  stamp.cpp
  timers.cpp
  trainer.cpp
  )

if(GUI)
  add_definitions(-DGUI)
  set(SRC
    ${SRC}
    keys.cpp
    maths.cpp
    translations.cpp
    gui/gui_common.cpp
    )
  foreach(FILE ${GUI_SRC})
    set(SRC ${SRC} gui/${GUI_DIR}/${FILE})
  endforeach()
endif()

foreach(FILE ${TARGET_SRC})
  set(SRC targets/${TARGET_DIR}/${FILE} ${SRC})
endforeach()

foreach(FILE ${FIRMWARE_TARGET_SRC})
  set(FIRMWARE_SRC ${FIRMWARE_SRC} targets/${TARGET_DIR}/${FILE})
endforeach()

foreach(FILE ${PULSES_SRC})
  set(SRC ${SRC} pulses/${FILE})
endforeach()

add_definitions(-DCORRECT_NEGATIVE_SHIFTS)

if(NOT MSVC)
  set(WARNING_FLAGS "${WARNING_FLAGS} -Wall -Wno-address-of-packed-member -Wno-strict-aliasing -Wformat -Wreturn-type -Wunused -Wuninitialized -Wunknown-pragmas -Wno-switch -Wtype-limits")
  if(WARNINGS_AS_ERRORS)
    set(WARNING_FLAGS "${WARNING_FLAGS} -Werror")
  endif(WARNINGS_AS_ERRORS)
else()
  add_definitions(-DHAVE_STRUCT_TIMESPEC)  # this is for pthread.h
  if(WARNINGS_AS_ERRORS)
    set(WARNING_FLAGS "${WARNING_FLAGS} /WX")
  endif(WARNINGS_AS_ERRORS)
endif()

add_subdirectory(targets/simu)
if(NOT MSVC)
  add_subdirectory(tests)
endif()

set(SRC ${SRC} ${FIRMWARE_SRC})

##### firmware target #####

option(FIRMWARE_TARGET "Configure Firmware target (can be turned of for compiling Companion only)" ON)
if(NOT FIRMWARE_TARGET)
  message(STATUS "firmware target disabled")
  return()
endif()

set(CMAKE_EXECUTABLE_SUFFIX ".elf")
# trick to remove the -rdynamic and --out-implib issues
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
# similar for -mmacosx-version-min="
set(CMAKE_C_OSX_DEPLOYMENT_TARGET_FLAG "")
set(CMAKE_CXX_OSX_DEPLOYMENT_TARGET_FLAG "")
# reset some CMake default flags which assume optimization levels/etc
set(CMAKE_C_FLAGS "${FIRMWARE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${FIRMWARE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_MINSIZEREL "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "")
set(CMAKE_CXX_FLAGS "${FIRMWARE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${FIRMWARE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_CXX_FLAGS_MINSIZEREL "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
# customize linker command
set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET>")

use_cxx11()  # ensure gnu++11 in CXX_FLAGS with CMake < 3.1

find_program(BASH bash HINTS "c:/cygwin/bin/" "c:/msys/bin/" DOC "bash shell for firmware size report (eg. c:/cygwin/bin/bash.exe on Windows).")

if(NOT MSVC)
  set(OPT s)

  if(ARCH STREQUAL ARM)
    enable_language(ASM)
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
    set(CMAKE_ASM_COMPILER arm-none-eabi-as)

    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_VERSION 1)
    set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> -o <OBJECT> <SOURCE>")

    set(COMMON_FLAGS "-mcpu=${MCU} -mthumb -fomit-frame-pointer -fverbose-asm -Wa,-ahlms=firmware.lst -O${OPT} -gdwarf-2 -DHSE_VALUE=${HSE_VALUE} -fno-exceptions -fdata-sections -ffunction-sections ${WARNING_FLAGS}")
    set(COMMON_FLAGS "${COMMON_FLAGS} ${FPU_FLAGS}")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -Wimplicit")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")

    # these are in addition to CMAKE_CXX_FLAGS
    set(CMAKE_EXE_LINKER_FLAGS "-lm -T${RADIO_SRC_DIRECTORY}/${LINKER_SCRIPT} -Wl,-Map=firmware.map,--cref,--no-warn-mismatch,--gc-sections")

    if(ACCESS_LIB)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--whole-archive ${RADIO_SRC_DIRECTORY}/${THIRDPARTY_DIR}/libACCESS/libAccess.a -Wl,--no-whole-archive")
    endif()
    
    if(SEMIHOSTING)
      add_definitions(-DSEMIHOSTING)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=rdimon.specs")
    endif()

    # Use newlib nano, which saves a few kilobytes.
    if(NOT NANO STREQUAL NO)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
      if(NOT LUA STREQUAL NO)
        # Lua needs %g and %f
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _printf_float -u _scanf_float")
      endif()
      # Nano's malloc does work well with lua, use our own
      add_definitions(-DUSE_BIN_ALLOCATOR)
      set(SRC ${SRC} bin_allocator.cpp)
    endif()

    if(PCB STREQUAL XLITE OR PCB STREQUAL XLITES OR PCB STREQUAL X9D OR PCB STREQUAL X9D+ OR PCB STREQUAL X9E OR PCB STREQUAL X7 OR PCB STREQUAL X9LITE OR PCB STREQUAL X9LITES OR PCB STREQUAL X10 OR PCB STREQUAL X12S)
      add_subdirectory(targets/common/arm/stm32/bootloader)
      include_directories(${CMAKE_CURRENT_BINARY_DIR}/targets/common/arm/stm32/bootloader)
      set(FIRMWARE_DEPENDENCIES ${FIRMWARE_DEPENDENCIES} bootloader)
    elseif(OPENRC_BOOTLOADER)
      # We fetch Mike's bootloader as it is not included inside OpenTX
      message(STATUS "Downloading Mike's bootloader: https://github.com/MikeBland/OpenRcBootloader/releases/download/V1.9/${OPENRC_BOOTLOADER}")
      file(DOWNLOAD https://github.com/MikeBland/OpenRcBootloader/releases/download/V1.9/${OPENRC_BOOTLOADER} ${CMAKE_CURRENT_BINARY_DIR}/bootloader.lbm STATUS status)
      list(GET status 0 error_code)
      if(error_code)
        list(GET status 1 error_message)
        message(FATAL_ERROR "Download failed: ${error_message}")
      endif()
    endif()

    add_definitions(-DRTOS_COOS)

    add_executable(firmware ${SRC} ${FIRMWARE_HEADERS})
    link_libraries(firmware -lstdc++)
    add_dependencies(firmware ${RADIO_DEPENDENCIES} ${FIRMWARE_DEPENDENCIES})
    set_target_properties(firmware PROPERTIES EXCLUDE_FROM_ALL TRUE)

    add_custom_command(
      TARGET firmware POST_BUILD
      COMMAND arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    find_program(GAWK gawk HINTS "c:/cygwin/bin/" "c:/msys/bin/" DOC "gawk for firmware size report.")  # OS X might only have "awk"
    if (BASH AND GAWK)
      add_custom_target(firmware-size
        COMMAND ${BASH} -kc '${RADIO_DIRECTORY}/util/elf-size-report.sh --mcu=${CPU_TYPE_FULL} ${SIZE_TARGET_MEM_DEFINE} firmware.elf'
        DEPENDS firmware
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else()
      add_custom_target(firmware-size
        COMMAND arm-none-eabi-size -A firmware.elf
        DEPENDS firmware
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
  endif()  # ARM

  if(CPU_FAMILY STREQUAL STM32)
    add_custom_target(flash
      COMMAND dfu-util --alt 0 --dfuse-address 0x08000000:leave -d 0483:df11 -D firmware.bin
      DEPENDS firmware
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_target(flash-jtag
      COMMAND st-flash --reset write firmware.bin 0x8000000
      DEPENDS firmware
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      )

    add_custom_target(flash-stlink
      COMMAND ST-LINK_CLI -c SWD -P firmware.bin 0x8000000 -Rst -Q
      DEPENDS firmware-size
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      )
  endif()

  PrintTargetReport("firmware")

endif(NOT MSVC)
# Include the lua subdirectory after CMAKE_C_COMPILER has been
# set. Otherwise, the custom commands it includes will use the
# default CMAKE_C_COMPILER instead of the one we set manually
add_subdirectory(lua)
