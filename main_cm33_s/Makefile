################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Top-level application make file.
#
################################################################################
# \copyright
# (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
# Technologies AG.  SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################


################################################################################
# Basic Configuration
################################################################################

# Type of ModusToolbox Makefile Options include:
#
# COMBINED    -- Top Level Makefile usually for single standalone application
# APPLICATION -- Top Level Makefile usually for multi project application
# PROJECT     -- Project Makefile under Application
#


# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager
# ('make library-manager' from command line), which will also update Eclipse IDE launch
# configurations.

include ../common.mk

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=main_cm33_s
APPTYPE=flash

VCORE_ATTRS=SECURE

# If set to "true" or "1", display full command-lines when building.
VERBOSE=


################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS+=SECURE_DEVICE

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

#for main linker
CORE=CM33
CORE_NAME=CM33_0


# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES=

# Add additional defines to the build process (without a leading -D).
DEFINES+=CY_PDL_FLASH_BOOT

# Comment to build the application for SORT LCS 
DEFINES+=NORMAL_PROVISIONED_LCS

# Enable ThreadX user configuration file (tx_user.h)
DEFINES+=TX_INCLUDE_USER_DEFINE_FILE

# Select softfp or hardfp floating point. Default is hardfp.
VFP_SELECT=hardfp

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS=

# Additional / custom linker flags.
LDFLAGS+=

# Additional / custom libraries to link in to the application.
LDLIBS+=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT+=

# Custom pre-build commands to run.
PREBUILD+=

# Custom post-build commands to run.
POSTBUILD+=

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../../

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# IAR toolchain: Enable full DLIB (required for DLib_threads.h) and multi-threading.
# --dlib_config=full is needed to expose DLib_threads.h (included by tx_iar.c for
# IAR v8+).  IAR.mk already adds this flag when MW_ABSTRACTION_RTOS is detected
# (i.e. when the abstraction-rtos library is fetched).  This project embeds ThreadX
# directly, so the flag must also be set here for the case when abstraction-rtos is
# NOT present.  Adding it twice causes IAR compiler error:
#   "Option --dlib_config does not match previous occurrence"
#
# This block is intentionally placed after CY_GETLIBS_SHARED_PATH/NAME so that the
# $(wildcard ...) check can locate the abstraction-rtos library directory.
ifeq ($(TOOLCHAIN),IAR)
  # Skip --dlib_config when building from IAR IDE (CY_IDE=IAR); the IDE project
  # settings handle the Library Configuration / dlib_config directly.
  ifneq ($(CY_IDE),IAR)
    # Only add the flag when abstraction-rtos is NOT fetched; if it IS present,
    # IAR.mk adds it via the MW_ABSTRACTION_RTOS component check.
    ifeq ($(wildcard $(CY_GETLIBS_SHARED_PATH)$(CY_GETLIBS_SHARED_NAME)/abstraction-rtos/*/),)
      CFLAGS   += --dlib_config=full
      CXXFLAGS += --dlib_config=full
    endif
  endif

  LDFLAGS  += --threaded_lib
  # Append the required -D defines for IAR assembler.
  MTB_TOOLCHAIN_IAR__ASFLAGS_LC = $(MTB_TOOLCHAIN_IAR__ASFLAGS) -DTX_SINGLE_MODE_SECURE -DTX_INCLUDE_USER_DEFINE_FILE
endif

include $(CY_TOOLS_DIR)/make/start.mk
