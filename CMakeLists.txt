cmake_minimum_required (VERSION 3.7.2 FATAL_ERROR)

if (APPLE)
  #
  # The following variables define the portability and compatability attributes of the Mac macOS build
  # they are choosen with care and should not be changed without good cause.
  #
  # Among other things these options are chosen to match the portability and compatability options of the
  # Qt framework dylibs which can be checked as follows:
  #
  # otool -l <binary> | grep -A3 LC_VERSION_MIN_MACOSX
  #
  set (CMAKE_OSX_DEPLOYMENT_TARGET 10.12
    CACHE STRING "Earliest version of macOS supported

Earliest version we can support with Qt 5.12, C++11 & libc++ is 10.12.
Do not override this if you intend to build an official deployable installer.")
endif (APPLE)

#
# CMake policies
#
if (POLICY CMP0020)
  cmake_policy (SET CMP0020 NEW) # link to Qt winmain on Windows
endif ()

if (POLICY CMP0043)
  cmake_policy (SET CMP0043 NEW) # ignore COMPILE_DEFINITIONS_<CONFIG>
endif ()

if (POLICY CMP0048)
  cmake_policy (SET CMP0048 NEW) # clear PROJECT_Version_* variables if not set in project() command
endif ()

if (POLICY CMP0063)
  cmake_policy (SET CMP0063 NEW) # honour visibility properties for all library types
endif ()

if (POLICY CMP0071)
  cmake_policy (SET CMP0071 NEW) # run automoc and autouic on generated sources
endif ()

if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW) # honour CMAKE_REQUIRED_LIBRARIES in config checks
endif ()

project (wsjtx
  VERSION 2.6.1.0
  LANGUAGES C CXX Fortran
  )
set (PROJECT_DESCRIPTION "WSJT-X: Digital Modes for Weak Signal Communications in Amateur Radio")
set (CMAKE_PROJECT_DESCRIPTION ${PROJECT_DESCRIPTION})

#
# Local CMake modules and support files
#
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake/Modules ${CMAKE_MODULE_PATH})

set (PROJECT_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
if (NOT PROJECT_ARCHITECTURE)
  # This is supposed to happen already on Windows
  if (CMAKE_SIZEOF_VOID_P MATCHES 8)
    set (PROJECT_ARCHITECTURE "x64")
  else ()
    set (PROJECT_ARCHITECTURE "$ENV{PROCESSOR_ARCHITECTURE}")
  endif ()
endif ()
message (STATUS "******************************************************")
message (STATUS "Building for for: ${CMAKE_SYSTEM_NAME}-${PROJECT_ARCHITECTURE}")
message (STATUS "******************************************************")

include (set_build_type)
# RC 0 or omitted is a development build, GA is a General Availability release build
set_build_type (GA)
set (wsjtx_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${BUILD_TYPE_REVISION}")

#
# project information
#
set (PROJECT_BUNDLE_NAME "WSJT-X")
set (PROJECT_VENDOR "Joe Taylor, K1JT")
set (PROJECT_CONTACT "Joe Taylor <k1jt@arrl.net>")
set (PROJECT_COPYRIGHT "Copyright (C) 2001-2023 by Joe Taylor, K1JT")
set (PROJECT_HOMEPAGE https://wsjt.sourceforge.io/wsjtx.html)
set (PROJECT_MANUAL wsjtx-main)
set (PROJECT_MANUAL_DIRECTORY_URL https://wsjt.sourceforge.io/wsjtx-doc/)
set (PROJECT_SAMPLES_URL http://downloads.sourceforge.net/project/wsjt/)
set (PROJECT_SAMPLES_UPLOAD_DEST frs.sourceforge.net:/home/frs/project/wsjt/)

# make sure that the default configuration is a RELEASE
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RELEASE CACHE STRING
    "Choose the type of build, options are: None Debug Release."
    FORCE)
endif (NOT CMAKE_BUILD_TYPE)
if (CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
  set (is_debug_build 1)
endif ()


#
# Options & features
#
#	Some of these directly effect compilation by being defined in
#	wsjtx_config.h.in which makes them available to the C/C++
#	pre-processor.
#
include (CMakeDependentOption)

# Allow the developer to select if Dynamic or Static libraries are built
OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
# Set the LIB_TYPE variable to STATIC
SET (LIB_TYPE STATIC)
if (BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  set (LIB_TYPE SHARED)
endif (BUILD_SHARED_LIBS)

option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts
files (WARNING: make clean will delete the source .ts files! Danger!)")
option (WSJT_SHARED_RUNTIME "Debugging option that allows running from a shared Cloud directory.")
option (WSJT_QDEBUG_TO_FILE "Redirect Qt debuging messages to a trace file.")
option (WSJT_SOFT_KEYING "Apply a ramp to CW keying envelope to reduce transients." ON)
option (WSJT_SKIP_MANPAGES "Skip *nix manpage generation.")
option (WSJT_GENERATE_DOCS "Generate documentation files." ON)
option (WSJT_RIG_NONE_CAN_SPLIT "Allow split operation with \"None\" as rig.")
option (WSJT_TRACE_UDP "Debugging option that turns on UDP message protocol diagnostics.")
option (WSJT_BUILD_UTILS "Build simulators and code demonstrators." ON)
CMAKE_DEPENDENT_OPTION (WSJT_QDEBUG_IN_RELEASE "Leave Qt debugging statements in Release configuration." OFF
  "NOT is_debug_build" OFF)
CMAKE_DEPENDENT_OPTION (WSJT_ENABLE_EXPERIMENTAL_FEATURES "Enable features not fully ready for public releases." ON
  is_debug_build OFF)
CMAKE_DEPENDENT_OPTION (WSJT_CREATE_WINMAIN
  "The wsjtx target is normally built as GUI executable with a WinMain entry point on Windows,
if you want a console application instead then set this option to OFF.

If you just want to see the debug output from the application then the easiest way is to
attach a debugger which will then receive the console output inside its console." ON
  "WIN32" OFF)

#
# install locations
#

if (APPLE)
  set (CMAKE_INSTALL_BINDIR ${CMAKE_PROJECT_NAME}.app/Contents/MacOS)
  set (CMAKE_INSTALL_DATAROOTDIR ${CMAKE_PROJECT_NAME}.app/Contents/Resources)
endif ()

include (GNUInstallDirs)

set (PLUGIN_DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins)
set (QT_CONF_DESTINATION ${CMAKE_INSTALL_BINDIR})
if (WIN32)
  set (PLUGIN_DESTINATION plugins)
elseif (APPLE)
  set (PLUGIN_DESTINATION ${CMAKE_INSTALL_BINDIR}/../PlugIns)
  set (QT_CONF_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR})
endif ()

set (WSJT_PLUGIN_DESTINATION ${PLUGIN_DESTINATION} CACHE PATH "Path for plugins")
set (WSJT_QT_CONF_DESTINATION ${QT_CONF_DESTINATION} CACHE PATH "Path for the qt.conf file")


#
# Project sources
#
set (fort_qt_CXXSRCS
  lib/shmem.cpp
  )

set (wsjt_qt_CXXSRCS
  helper_functions.cpp
  qt_helpers.cpp
  widgets/MessageBox.cpp
  MetaDataRegistry.cpp
  Network/NetworkServerLookup.cpp
  revision_utils.cpp
  L10nLoader.cpp
  WFPalette.cpp
  Radio.cpp
  RadioMetaType.cpp
  NonInheritingProcess.cpp
  models/IARURegions.cpp
  models/Bands.cpp
  models/Modes.cpp
  models/FrequencyList.cpp
  models/StationList.cpp
  widgets/FrequencyLineEdit.cpp
  widgets/FrequencyDeltaLineEdit.cpp
  item_delegates/CandidateKeyFilter.cpp
  item_delegates/ForeignKeyDelegate.cpp
  item_delegates/MessageItemDelegate.cpp
  validators/LiveFrequencyValidator.cpp
  GetUserId.cpp
  Audio/AudioDevice.cpp
  Transceiver/Transceiver.cpp
  Transceiver/TransceiverBase.cpp
  Transceiver/EmulateSplitTransceiver.cpp
  Transceiver/TransceiverFactory.cpp
  Transceiver/PollingTransceiver.cpp
  Transceiver/HamlibTransceiver.cpp
  Transceiver/HRDTransceiver.cpp
  Transceiver/DXLabSuiteCommanderTransceiver.cpp
  Network/NetworkMessage.cpp
  Network/MessageClient.cpp
  widgets/LettersSpinBox.cpp
  widgets/HintedSpinBox.cpp
  widgets/RestrictedSpinBox.cpp
  widgets/HelpTextWindow.cpp
  SampleDownloader.cpp
  SampleDownloader/DirectoryDelegate.cpp
  SampleDownloader/Directory.cpp
  SampleDownloader/FileNode.cpp
  SampleDownloader/RemoteFile.cpp
  DisplayManual.cpp
  MultiSettings.cpp
  validators/MaidenheadLocatorValidator.cpp
  validators/CallsignValidator.cpp
  widgets/SplashScreen.cpp
  EqualizationToolsDialog.cpp
  widgets/DoubleClickablePushButton.cpp
  widgets/DoubleClickableRadioButton.cpp
  Network/LotWUsers.cpp
  models/DecodeHighlightingModel.cpp
  widgets/DecodeHighlightingListView.cpp
  models/FoxLog.cpp
  widgets/AbstractLogWindow.cpp
  widgets/FoxLogWindow.cpp
  widgets/CabrilloLogWindow.cpp
  item_delegates/CallsignDelegate.cpp
  item_delegates/MaidenheadLocatorDelegate.cpp
  item_delegates/FrequencyDelegate.cpp
  item_delegates/FrequencyDeltaDelegate.cpp
  item_delegates/SQLiteDateTimeDelegate.cpp
  models/CabrilloLog.cpp
  logbook/AD1CCty.cpp
  logbook/WorkedBefore.cpp
  logbook/Multiplier.cpp
  Network/NetworkAccessManager.cpp
  widgets/LazyFillComboBox.cpp
  widgets/CheckableItemComboBox.cpp
  widgets/BandComboBox.cpp
  )

set (wsjt_qtmm_CXXSRCS
  Audio/BWFFile.cpp
  )

set (jt9_FSRCS
  lib/jt9.f90
  lib/jt9a.f90
  )

set (wsjtx_CXXSRCS
  WSJTXLogging.cpp
  logbook/logbook.cpp
  Network/PSKReporter.cpp
  Modulator/Modulator.cpp
  Detector/Detector.cpp
  widgets/logqso.cpp
  widgets/displaytext.cpp
  Decoder/decodedtext.cpp
  getfile.cpp
  Audio/soundout.cpp
  Audio/soundin.cpp
  widgets/meterwidget.cpp
  widgets/signalmeter.cpp
  widgets/plotter.cpp
  widgets/widegraph.cpp
  widgets/echograph.cpp
  widgets/echoplot.cpp
  widgets/fastgraph.cpp
  widgets/fastplot.cpp
  widgets/about.cpp
  widgets/astro.cpp
  widgets/messageaveraging.cpp
  widgets/activeStations.cpp
  widgets/colorhighlighting.cpp
  WSPR/WsprTxScheduler.cpp
  widgets/mainwindow.cpp
  Configuration.cpp
  main.cpp
  Network/wsprnet.cpp
  WSPR/WSPRBandHopping.cpp
  widgets/ExportCabrillo.cpp
  )

set (wsjt_CXXSRCS
  Logger.cpp
  lib/crc10.cpp
  lib/crc13.cpp
  lib/crc14.cpp
  )
# deal with a GCC v6 UB error message
set_source_files_properties (
  lib/crc10.cpp
  lib/crc13.cpp
  lib/crc14.cpp
  PROPERTIES COMPILE_FLAGS -fpermissive)

if (WIN32)
  set (wsjt_CXXSRCS
    ${wsjt_CXXSRCS}
    killbyname.cpp
    )

  set (wsjt_qt_CXXSRCS
    ${wsjt_qt_CXXSRCS}
    Transceiver/OmniRigTransceiver.cpp
    )
endif (WIN32)

set (wsjt_FSRCS
  # put module sources first in the hope that they get rebuilt before use
  lib/types.f90
  lib/C_interface_module.f90
  lib/shmem.f90
  lib/crc.f90
  lib/fftw3mod.f90
  lib/hashing.f90
  lib/iso_c_utilities.f90
  lib/jt4.f90
  lib/jt4_decode.f90
  lib/jt65_decode.f90
  lib/jt65_mod.f90
  lib/ft8_decode.f90
  lib/ft4_decode.f90
  lib/fst4_decode.f90
  lib/jt9_decode.f90
  lib/options.f90
  lib/packjt.f90
  lib/77bit/packjt77.f90
  lib/qra/q65/q65.f90
  lib/q65_decode.f90
  lib/readwav.f90
  lib/timer_C_wrapper.f90
  lib/timer_impl.f90
  lib/timer_module.f90
  lib/wavhdr.f90
  lib/qra/q65/q65_encoding_modules.f90
  lib/ft8/ft8_a7.f90

  # remaining non-module sources
  lib/addit.f90
  lib/afc65b.f90
  lib/afc9.f90
  lib/ana64.f90
  lib/ana932.f90
  lib/analytic.f90
  lib/astro.f90
  lib/astrosub.f90
  lib/astro0.f90
  lib/avecho.f90
  lib/averms.f90
  lib/azdist.f90
  lib/ft8/baseline.f90
  lib/ft4/ft4_baseline.f90
  lib/blanker.f90
  lib/bpdecode40.f90
  lib/bpdecode128_90.f90
  lib/ft8/bpdecode174_91.f90
  lib/baddata.f90
  lib/calibrate.f90
  lib/ccf2.f90
  lib/ccf65.f90
  lib/ft8/chkcrc13a.f90
  lib/ft8/chkcrc14a.f90
  lib/chkcall.f90
  lib/chkhist.f90
  lib/chkmsg.f90
  lib/chkss2.f90
  lib/ft4/clockit.f90
  lib/ft8/compress.f90
  lib/coord.f90
  lib/db.f90
  lib/decode4.f90
  lib/decode65a.f90
  lib/decode65b.f90
  lib/decode9w.f90
  lib/ft8/decode174_91.f90
  lib/decoder.f90
  lib/deep4.f90
  lib/deg2grid.f90
  lib/degrade_snr.f90
  lib/demod64a.f90
  lib/determ.f90
  lib/downsam9.f90
  lib/echosim.f90
  lib/echo_snr.f90
  lib/encode232.f90
  lib/encode4.f90
  lib/encode_msk40.f90
  lib/encode_128_90.f90
  lib/ft8/encode174_91.f90
  lib/ft8/encode174_91_nocrc.f90
  lib/entail.f90
  lib/ephem.f90
  lib/extract.f90
  lib/extract4.f90
  lib/extractmessage77.f90
  lib/fano232.f90
  lib/fast9.f90
  lib/fast_decode.f90
  lib/fchisq.f90
  lib/fchisq0.f90
  lib/fchisq65.f90
  lib/fil3.f90
  lib/fil3c.f90
  lib/fil4.f90
  lib/fil6521.f90
  lib/filbig.f90
  lib/ft8/filt8.f90
  lib/fitcal.f90
  lib/flat1.f90
  lib/flat1a.f90
  lib/flat1b.f90
  lib/flat2.f90
  lib/flat4.f90
  lib/flat65.f90
  lib/fmtmsg.f90
  lib/foldspec9f.f90
  lib/four2a.f90
  lib/fspread_lorentz.f90
  lib/ft8/foxfilt.f90
  lib/ft8/foxgen.f90
  lib/ft8/foxgen_wrap.f90
  lib/freqcal.f90
  lib/ft8/ft8apset.f90
  lib/ft8/ft8b.f90
  lib/ft8/ft8code.f90
  lib/ft8/ft8_downsample.f90
  lib/ft8/ft8sim.f90
  lib/gen4.f90
  lib/gen65.f90
  lib/gen9.f90
  lib/genwave.f90
  lib/ft8/genft8.f90
  lib/qra/q65/genq65.f90
  lib/genmsk_128_90.f90
  lib/genmsk40.f90
  lib/ft4/ft4code.f90
  lib/ft4/genft4.f90
  lib/ft4/gen_ft4wave.f90
  lib/ft8/gen_ft8wave.f90
  lib/ft8/genft8refsig.f90
  lib/genwspr.f90
  lib/geodist.f90
  lib/ft8/get_crc14.f90
  lib/getlags.f90
  lib/getmet4.f90
  lib/ft8/get_spectrum_baseline.f90
  lib/ft2/gfsk_pulse.f90
  lib/graycode.f90
  lib/graycode65.f90
  lib/grayline.f90
  lib/grid2deg.f90
  lib/ft8/h1.f90
  lib/hash.f90
  lib/hint65.f90
  lib/hspec.f90
  lib/indexx.f90
  lib/init_random_seed.f90
  lib/interleave4.f90
  lib/interleave63.f90
  lib/interleave9.f90
  lib/inter_wspr.f90
  lib/jplsubs.f
  lib/jt9fano.f90
  lib/libration.f90
  lib/lorentzian.f90
  lib/fst4/lorentzian_fading.f90
  lib/lpf1.f90
  lib/map65_mmdec.f90
  lib/mixlpf.f90
  lib/makepings.f90
  lib/moondopjpl.f90
  lib/morse.f90
  lib/move.f90
  lib/msk40decodeframe.f90
  lib/msk144decodeframe.f90
  lib/msk40spd.f90
  lib/msk144spd.f90
  lib/msk40sync.f90
  lib/msk144sync.f90
  lib/msk40_freq_search.f90
  lib/msk144_freq_search.f90
  lib/mskrtd.f90
  lib/msk144signalquality.f90
  lib/msk144sim.f90
  lib/mskrtd.f90
  lib/nuttal_window.f90
  lib/ft4/ft4sim.f90
  lib/ft4/ft4sim_mult.f90
  lib/ft4/ft4_downsample.f90
  lib/77bit/my_hash.f90
  lib/wsprd/osdwspr.f90
  lib/ft8/osd174_91.f90
  lib/osd128_90.f90
  lib/pctile.f90
  lib/peakdt9.f90
  lib/peakup.f90
  lib/plotsave.f90
  lib/platanh.f90
  lib/pltanh.f90
  lib/polyfit.f90
  lib/prog_args.f90
  lib/ps4.f90
  lib/qra/q65/q65_ap.f90
  lib/qra/q65/q65_loops.f90
  lib/qra/q65/q65_set_list.f90
  lib/refspectrum.f90
  lib/savec2.f90
  lib/save_dxbase.f90
  lib/save_echo_params.f90
  lib/sec0.f90
  lib/sec_midn.f90
  lib/setup65.f90
  lib/sh65.f90
  lib/sh65snr.f90
  lib/slasubs.f
  lib/sleep_msec.f90
  lib/slope.f90
  lib/smo.f90
  lib/smo121.f90
  lib/softsym.f90
  lib/softsym9f.f90
  lib/softsym9w.f90
  lib/shell.f90
  lib/spec64.f90
  lib/spec9f.f90
  lib/stdmsg.f90
  lib/subtract65.f90
  lib/ft8/subtractft8.f90
  lib/ft4/subtractft4.f90
  lib/sun.f90
  lib/symspec.f90
  lib/symspec2.f90
  lib/symspec65.f90
  lib/sync4.f90
  lib/sync65.f90
  lib/ft4/getcandidates4.f90
  lib/ft4/get_ft4_bitmetrics.f90
  lib/ft8/sync8.f90
  lib/ft8/sync8d.f90
  lib/ft4/sync4d.f90
  lib/sync9.f90
  lib/sync9f.f90
  lib/sync9w.f90
  lib/test_snr.f90
  lib/timf2.f90
  lib/tweak1.f90
  lib/twkfreq.f90
  lib/ft8/twkfreq1.f90
  lib/twkfreq65.f90
  lib/update_recent_calls.f90
  lib/update_msk40_hasharray.f90
  lib/ft8/watterson.f90
  lib/wav11.f90
  lib/wav12.f90
  lib/xcor.f90
  lib/xcor4.f90
  lib/wqdecode.f90
  lib/wqencode.f90
  lib/wspr_downsample.f90
  lib/zplot9.f90
  lib/fst4/decode240_101.f90
  lib/fst4/decode240_74.f90
  lib/fst4/encode240_101.f90
  lib/fst4/encode240_74.f90
  lib/fst4/fst4sim.f90
  lib/fst4/gen_fst4wave.f90
  lib/fst4/genfst4.f90
  lib/fst4/get_fst4_bitmetrics.f90
  lib/fst4/get_fst4_bitmetrics2.f90
  lib/fst4/ldpcsim240_101.f90
  lib/fst4/ldpcsim240_74.f90
  lib/fst4/osd240_101.f90
  lib/fst4/osd240_74.f90
  lib/fst4/fastosd240_74.f90
  lib/fst4/get_crc24.f90
  lib/fst4/fst4_baseline.f90
  )

# temporary workaround for a gfortran v7.3 ICE on Fedora 27 64-bit
set_source_files_properties (lib/slasubs.f PROPERTIES COMPILE_FLAGS -O2)

set (ka9q_CSRCS
  lib/ftrsd/decode_rs.c
  lib/ftrsd/encode_rs.c
  lib/ftrsd/init_rs.c
  )
set_source_files_properties (${ka9q_CSRCS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare)

set (qra_CSRCS
  lib/qra/qracodes/qra12_63_64_irr_b.c
  lib/qra/qracodes/qra13_64_64_irr_e.c
  lib/qra/q65/npfwht.c
  lib/qra/q65/pdmath.c
  lib/qra/q65/qracodes.c
  lib/qra/q65/normrnd.c
  lib/qra/q65/qra15_65_64_irr_e23.c
  lib/qra/q65/q65.c
  lib/qra/q65/q65_subs.c
  )

set (wsjt_CSRCS
  ${ka9q_CSRCS}
  lib/ftrsd/ftrsdap.c
  lib/sgran.c
  lib/golay24_table.c
  lib/gran.c
  lib/igray.c
  lib/init_random_seed.c
  lib/ldpc32_table.c
  lib/wsprd/nhash.c
  lib/tab.c
  lib/tmoonsub.c
  lib/usleep.c
  lib/vit213.c
  lib/wisdom.c
  lib/wrapkarn.c
  ${ldpc_CSRCS}
  ${qra_CSRCS}
  )

set (wsjt_qt_UISRCS
  wf_palette_design_dialog.ui
  widgets/FoxLogWindow.ui
  widgets/CabrilloLogWindow.ui
  )

set (wsprsim_CSRCS
  lib/wsprd/wsprsim.c
  lib/wsprd/wsprsim_utils.c
  lib/wsprd/wsprd_utils.c
  lib/wsprd/fano.c
  lib/wsprd/tab.c
  lib/wsprd/nhash.c
  )

set (wsprd_CSRCS
  lib/wsprd/wsprd.c
  lib/wsprd/wsprsim_utils.c
  lib/wsprd/wsprd_utils.c
  lib/wsprd/fano.c
  lib/wsprd/jelinek.c
  lib/wsprd/tab.c
  lib/wsprd/nhash.c
  lib/init_random_seed.c
  )

set (wsjtx_UISRCS
  widgets/mainwindow.ui
  widgets/about.ui
  widgets/astro.ui
  widgets/colorhighlighting.ui
  widgets/echograph.ui
  widgets/fastgraph.ui
  widgets/messageaveraging.ui
  widgets/activeStations.ui
  widgets/widegraph.ui
  widgets/logqso.ui
  Configuration.ui
  widgets/ExportCabrillo.ui
  )

set (UDP_library_CXXSRCS
  Radio.cpp
  RadioMetaType.cpp
  Network/NetworkMessage.cpp
  UDPExamples/MessageServer.cpp
  )

set (UDP_library_HEADERS
  Radio.hpp
  UDPExamples/MessageServer.hpp
  ${PROJECT_BINARY_DIR}/udp_export.h
  )

set (message_aggregator_CXXSRCS
  UDPExamples/MessageAggregator.cpp
  UDPExamples/MessageAggregatorMainWindow.cpp
  UDPExamples/DecodesModel.cpp
  UDPExamples/BeaconsModel.cpp
  UDPExamples/ClientWidget.cpp
  validators/MaidenheadLocatorValidator.cpp
  )

set (message_aggregator_STYLESHEETS
  UDPExamples/qss/default.qss
  )

set (qcp_CXXSRCS
  qcustomplot-source/qcustomplot.cpp
  )

set (all_CXXSRCS
  ${wsjt_CXXSRCS}
  ${fort_qt_CXXSRCS}
  ${wsjt_qt_CXXSRCS}
  ${wsjt_qtmm_CXXSRCS}
  ${wsjtx_CXXSRCS}
  ${qcp_CXXSRCS}
  )

set (all_C_and_CXXSRCS
  ${wsjt_CSRCS}
  ${wsprsim_CSRCS}
  ${wsprd_CSRCS}
  ${all_CXXSRCS}
  )

set (TOP_LEVEL_RESOURCES
  icons/Darwin/wsjtx.iconset/icon_128x128.png
  contrib/gpl-v3-logo.svg
  artwork/splash.png
  )

set (PALETTE_FILES
  Palettes/Banana.pal
  Palettes/Blue1.pal
  Palettes/Blue2.pal
  Palettes/Blue3.pal
  Palettes/Brown.pal
  Palettes/Cyan1.pal
  Palettes/Cyan2.pal
  Palettes/Cyan3.pal
  Palettes/Default.pal
  Palettes/Digipan.pal
  Palettes/Fldigi.pal
  Palettes/Gray1.pal
  Palettes/Gray2.pal
  Palettes/Green1.pal
  Palettes/Green2.pal
  Palettes/Jungle.pal
  Palettes/Linrad.pal
  Palettes/Negative.pal
  Palettes/Orange.pal
  Palettes/Pink.pal
  Palettes/Rainbow.pal
  Palettes/Scope.pal
  Palettes/Sunburst.pal
  Palettes/VK4BDJ.pal
  Palettes/YL2KF.pal
  Palettes/Yellow1.pal
  Palettes/Yellow2.pal
  Palettes/ZL1FZ.pal
)

if (APPLE)
  set (WSJTX_ICON_FILE ${CMAKE_PROJECT_NAME}.icns)
  set (ICONSRCS
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_16x16.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_16x16@2x.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_32x32.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_32x32@2x.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_128x128.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_128x128@2x.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_256x256.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_256x256@2x.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_512x512.png
    icons/Darwin/${CMAKE_PROJECT_NAME}.iconset/icon_512x512@2x.png
    )
  add_custom_command (
    OUTPUT ${WSJTX_ICON_FILE}
    COMMAND iconutil -c icns --output "${CMAKE_BINARY_DIR}/${WSJTX_ICON_FILE}" "${CMAKE_SOURCE_DIR}/icons/Darwin/${CMAKE_PROJECT_NAME}.iconset"
    DEPENDS ${ICONSRCS}
    COMMENT "Building Icons"
    )
else ()
  set (WSJTX_ICON_FILE icons/windows-icons/wsjtx.ico)
endif (APPLE)

set_source_files_properties (${WSJTX_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# suppress intransigent compiler diagnostics
set_source_files_properties (lib/decoder.f90 PROPERTIES COMPILE_FLAGS "-Wno-unused-dummy-argument")
set_source_files_properties (lib/filbig.f90 PROPERTIES COMPILE_FLAGS "-Wno-aliasing")

## disable Qt trace and warning messages from release configurations
#set_property (DIRECTORY APPEND PROPERTY
#  COMPILE_DEFINITIONS $<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG_OUTPUT;QT_NO_WARNING_OUTPUT>
#  )

set_property (SOURCE ${all_C_and_CXXSRCS} APPEND_STRING PROPERTY COMPILE_FLAGS " -include wsjtx_config.h")
set_property (SOURCE ${all_C_and_CXXSRCS} APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/wsjtx_config.h)

#
# find some useful tools
#
include (CheckTypeSize)
include (CheckCSourceCompiles)
include (CheckIncludeFiles)
include (CheckSymbolExists)
include (generate_version_info)

find_program(CTAGS ctags)
find_program(ETAGS etags)

#
# Platform checks
#
check_include_files ("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)
check_include_files (stdio.h HAVE_STDIO_H)
check_include_files (stdlib.h HAVE_STDLIB_H)
check_include_files (unistd.h HAVE_UNISTD_H)
check_include_files (sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_files (sys/types.h HAVE_SYS_TYPES_H)
check_include_files (fcntl.h HAVE_FCNTL_H)
check_include_files (sys/stat.h HAVE_SYS_STAT_H)
check_include_files ("linux/ppdev.h;linux/parport.h" HAVE_LINUX_PPDEV_H)
check_include_files ("dev/ppbus/ppi.h;dev/ppbus/ppbconf.h" HAVE_DEV_PPBUS_PPI_H)

#
# Standard C Math Library
#
set (LIBM_TEST_SOURCE "#include<math.h>\nfloat f; int main(){sqrt(f);return 0;}")
check_c_source_compiles ("${LIBM_TEST_SOURCE}" HAVE_MATH)
if (HAVE_MATH)
  set (LIBM_LIBRARIES)
else ()
  set (CMAKE_REQUIRED_LIBRARIES m)
  check_c_source_compiles ("${LIBM_TEST_SOURCE}" HAVE_LIBM_MATH)
  unset (CMAKE_REQUIRED_LIBRARIES)
  if (NOT HAVE_LIBM_MATH)
    message (FATAL_ERROR "Unable to use C math library functions")
  endif ()
  set (LIBM_LIBRARIES m)
endif ()

#
# Boost
#
#if (WIN32)
#  set (Boost_USE_STATIC_LIBS OFF)
#endif ()
#find_package (Boost 1.62 REQUIRED COMPONENTS log_setup log)

#
# OpenMP
#
# find_package (OpenMP)
set(OPENMP_FOUND 0)

#
# fftw3 single precision library
#
find_package (FFTW3 COMPONENTS single threads REQUIRED)

#
# Library building setup
#
include (GenerateExportHeader)
set (CMAKE_CXX_VISIBILITY_PRESET hidden)
set (CMAKE_C_VISIBILITY_PRESET hidden)
set (CMAKE_Fortran_VISIBILITY_PRESET hidden)
set (CMAKE_VISIBILITY_INLINES_HIDDEN ON)
#set (CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)


#
# C & C++ setup
#
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -fexceptions -frtti")

if (NOT APPLE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pragmas")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fdata-sections -ffunction-sections")
  set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -fdata-sections -ffunction-sections")
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fdata-sections -ffunction-sections")
  set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -fdata-sections -ffunction-sections")
endif (NOT APPLE)

if (APPLE AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
else ()
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=gnu++11 -pthread")
endif ()


#
# Fortran setup
#
set (General_FFLAGS "-Wall -Wno-conversion -fno-second-underscore")

# FFLAGS depend on the compiler
get_filename_component (Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)

if (Fortran_COMPILER_NAME MATCHES "gfortran.*")
  # gfortran

  # CMake compiler test is supposed to do this but doesn't yet
  if (CMAKE_OSX_DEPLOYMENT_TARGET)
    set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif (CMAKE_OSX_DEPLOYMENT_TARGET)
  if (CMAKE_OSX_SYSROOT)
    set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT}")
  endif (CMAKE_OSX_SYSROOT)

  set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} -fbounds-check -funroll-all-loops -fno-f2c -ffpe-summary=invalid,zero,overflow,underflow ${General_FFLAGS}")

set (CMAKE_Fortran_FLAGS_DEBUG   "${CMAKE_Fortran_FLAGS_DEBUG} -g -fbacktrace -fbounds-check -fno-f2c -ffpe-summary=invalid,zero,overflow,underflow ${General_FFLAGS}")
  
  # FPE traps currently disabled in Debug configuration builds until
  # we decide if they are meaningful, without these FP instructions
  # run in nonstop mode and do not trap
  #set (CMAKE_Fortran_FLAGS_DEBUG   "${CMAKE_Fortran_FLAGS_DEBUG} ${CMAKE_Fortran_FLAGS_DEBUG}  -ffpe-trap=invalid,zero,overflow")
  
elseif (Fortran_COMPILER_NAME MATCHES "ifort.*")
  # ifort (untested)
  set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} -f77rtl ${General_FFLAGS}")
  set (CMAKE_Fortran_FLAGS_DEBUG   "${CMAKE_Fortran_FLAGS_DEBUG} -f77rtl ${General_FFLAGS}")
elseif (Fortran_COMPILER_NAME MATCHES "g77")
  # g77
  set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} -funroll-all-loops -fno-f2c -m32 ${General_FFLAGS}")
  set (CMAKE_Fortran_FLAGS_DEBUG   "${CMAKE_Fortran_FLAGS_DEBUG} -fbounds-check -fno-f2c -m32 ${General_FFLAGS}")
else (Fortran_COMPILER_NAME MATCHES "gfortran.*")
  message ("CMAKE_Fortran_COMPILER full path: " ${CMAKE_Fortran_COMPILER})
  message ("Fortran compiler: " ${Fortran_COMPILER_NAME})
  message ("No optimized Fortran compiler flags are known, we just try -O3...")
  set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} -O3 ${General_FFLAGS}")
  set (CMAKE_Fortran_FLAGS_DEBUG   "${CMAKE_Fortran_FLAGS_DEBUG} -fbounds-check ${General_FFLAGS}")
endif (Fortran_COMPILER_NAME MATCHES "gfortran.*")


#
# Linker setup
#
if (NOT APPLE)
  set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
  set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -Wl,--gc-sections")
endif (NOT APPLE)


#
# setup and test Fortran C/C++ interaction
#

include (FortranCInterface)
FortranCInterface_VERIFY (CXX)
FortranCInterface_HEADER (FC.h MACRO_NAMESPACE "FC_" SYMBOL_NAMESPACE "FC_"
  SYMBOLS
  grayline
  )


#
# sort out pre-requisites
#

#
# Setup RPATH so that built executable targets will run in both the
# build tree and the install location without having to set a
# (DYLD|LD)_LIBRARY_PATH override.
#

# use the full RPATH of the build tree
set (CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH, it will still be used
# later on in the install phase
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# add the automaticaly determined parts of the RPATH which point to
# directories outside of the build tree to the install RPATH
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_definitions (-DQT5 -DCMAKE_BUILD -DBIGSYM=1 -DBOOST_ALL_DYN_LINK)
if (CMAKE_HOST_UNIX)
  add_definitions (-DUNIX)
elseif (CMAKE_HOST_WIN32)
  add_definitions (-DWIN32)
endif ()

# build a library of package functionality (without and optionally with OpenMP support)
add_library (wsjt_cxx STATIC ${wsjt_CSRCS} ${wsjt_CXXSRCS})
target_link_libraries (wsjt_cxx ${LIBM_LIBRARIES} ${LIBM_LIBRARIES})

# build an OpenMP variant of the Fortran library routines
add_library (wsjt_fort STATIC ${wsjt_FSRCS})
target_link_libraries (wsjt_fort  ${FFTW3_LIBRARIES})

# build the wsprd WSPR mode decoder driver
generate_version_info (wsprd_VERSION_RESOURCES
  NAME wsprd
  BUNDLE ${PROJECT_BUNDLE_NAME}
  ICON ${WSJTX_ICON_FILE}
  FILE_DESCRIPTION "WSPR mode decoder"
  )
add_executable (wsprd ${wsprd_CSRCS} lib/indexx.f90 lib/wsprd/osdwspr.f90 ${wsprd_VERSION_RESOURCES})
target_include_directories (wsprd PRIVATE ${FFTW3_INCLUDE_DIRS})
target_link_libraries (wsprd ${FFTW3_LIBRARIES} ${LIBM_LIBRARIES})


#
# targets dependent on Qt
#

# build a library of package Qt functionality used in Fortran utilities
add_library (fort_qt STATIC ${fort_qt_CXXSRCS})

# build the jt9 slow mode decoder driver
generate_version_info (jt9_VERSION_RESOURCES
  NAME jt9
  BUNDLE ${PROJECT_BUNDLE_NAME}
  ICON ${WSJTX_ICON_FILE}
  FILE_DESCRIPTION "jt9 - WSJT-X slow mode decoder"
  )

add_executable (jt9 ${jt9_FSRCS} ${jt9_VERSION_RESOURCES})
target_link_libraries (jt9 wsjt_fort wsjt_cxx fort_qt)

#
# versioning and configuration
#
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/wsjtx_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/wsjtx_config.h"
  )

