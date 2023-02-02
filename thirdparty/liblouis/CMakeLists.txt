#=============================================================================
#  Mscore
#  Linux Music Score Editor
#
#  Copyright (C) 2009 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

set(MODULE liblouis)

set(WIDECHAR_TYPE "unsigned int") # liblouis.h.in
set(WIDECHARS_ARE_UCS4 TRUE)      # configMS.h.in

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/configMS.h.in
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/config.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/liblouis.h.in
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/liblouis.h
    @ONLY
)

set(MODULE_SRC    
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/commonTranslationFunctions.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/liblouis.h
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/pattern.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/compileTranslationTable.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/config.h
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/logging.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/utils.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/lou_translateString.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/lou_backTranslateString.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/maketable.c
    ${CMAKE_CURRENT_LIST_DIR}/liblouis/internal.h   
)

set(MODULE_USE_PCH_NONE ON)
set(MODULE_USE_UNITY_NONE ON)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -w)

