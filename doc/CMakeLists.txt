set (LANGUAGES
  en
  )

set (common_SRCS
  common/communication.adoc
  common/license.adoc
  common/links.adoc
  )

set (UG_SRCS
  docinfo.html
  docinfo.xml
  acknowledgements.adoc
  astro_data.adoc
  config-details.adoc
  controls-functions-center.adoc
  controls-functions-left.adoc
  controls-functions-main-window.adoc
  controls-functions-menus.adoc
  controls-functions-messages.adoc
  controls-functions-status-bar.adoc
  controls-functions-wide-graph.adoc
  cooperating-programs.adoc
  decoder_notes.adoc
  faq.adoc
  font-sizes.adoc
  install-from-source.adoc
  install-linux.adoc
  install-mac.adoc
  install-windows.adoc
  introduction.adoc
  intro_subsections.adoc
  logging.adoc
  make-qso.adoc
  measurement_tools.adoc
  new_features.adoc
  platform-dependencies.adoc
  protocols.adoc
  settings-advanced.adoc
  settings-audio.adoc
  settings-colors.adoc
  settings-frequencies.adoc
  settings-general.adoc
  settings-radio.adoc
  settings-reporting.adoc
  settings-txmacros.adoc
  support.adoc
  system-requirements.adoc
  transceiver-setup.adoc
  tutorial-example1.adoc
  tutorial-example3.adoc
  tutorial-example4.adoc
  tutorial-example5.adoc
  tutorial-example6.adoc
  tutorial-download-samples.adoc
  tutorial-wide-graph-settings.adoc
  utilities.adoc
  message-creator.adoc
  vhf-features.adoc
  wsjtx-main.adoc
  wspr.adoc
  )

set (UG_IMGS
  images/active_stations.png
  images/Add_Frequency.png
  images/Add_station_info.png
  images/AstroData_2.png
  images/Astronomical_data.png
  images/auto-seq.png
  images/band-settings.png
  images/Best_S+P.png
  images/colors.png
  images/config-menu.png
  images/decode-menu.png
  images/download_samples.png
  images/Echo_1296.png
  images/EME_Deep_0.png
  images/EME_Deep_1.png
  images/EME_Deep_2.png
  images/file-menu.png
  images/FreqCal.png
  images/FreqCal_Graph.png
  images/FreqCal_Results.png
  images/freemsg.png
  images/FST4_center.png
  images/FST4_Decoding_Limits.png
  images/FST4W_RoundRobin.png
  images/ft4_decodes.png
  images/ft4_waterfall.png
  images/ft8_decodes.png
  images/FT8_waterfall.png
  images/help-menu.png
  images/JT4F.png
  images/JT65B.png
  images/keyboard-shortcuts.png
  images/MSK144.png
  images/Q65_6m_ionoscatter.png
  images/WSPR_WideGraphControls.png
  images/WSPR_1a.png
  images/WSPR_2.png
  images/jtalert.png
  images/keyboard-shortcuts.png
  images/log-qso.png
  images/MacAppMenu.png
  images/main-ui.png
  images/main-ui-controls.png
  images/misc-controls-center.png
  images/misc-main-ui.png
  images/mode-menu.png
  images/new-msg-box.png
  images/psk-reporter.png
  images/r3666-config-screen-80.png
  images/r3666-main-ui-80.png
  images/r4148-txmac-ui.png
  images/mc1.png
  images/mc2.png
  images/mc3.png
  images/mc4.png
  images/mc5.png
  images/mc6.png
  images/mc7.png
  images/Split_settings.png
  images/letter-s.png
  images/RadioTab.png
  images/reporting.png
  images/save-menu.png
  images/settings-advanced.png
  images/settings-audio.png
  images/settings-frequencies.png
  images/settings-general.png
  images/setup-menu.png
  images/special-mouse-commands.png
  images/status-bar-a.png
  images/tools-menu.png
  images/traditional-msg-box.png
  images/tx-macros.png
  images/VHF_controls.png
  images/view-menu.png
  images/wide-graph-controls.png
  images/Windows_Dark.png
)

find_program (ASCIIDOCTOR_EXECUTABLE NAMES asciidoctor)
if (NOT ASCIIDOCTOR_EXECUTABLE)
  message (FATAL_ERROR "asciidoctor is required to build the documentation

Building the documenation may optionally be turned off by setting the CMake
option WSJT_GENERATE_DOCS to OFF.")
endif (NOT ASCIIDOCTOR_EXECUTABLE)

find_program (FOPUB_EXECUTABLE NAMES fopub)

include (CMakeParseArguments)

# generate a document from asciidoc text files(s)
#
# HTML - generate an HTML document
# PDF - generate a PDF document
# SOURCE - top level asciidoc file
# ASCIIDOCTOR_OPTIONS - asciidoctor command options
# DEPENDS - dependent files
function (document)
  cmake_parse_arguments (_args "HTML" "SOURCE;LANG;OUTPUT" "ASCIIDOCTOR_OPTIONS;PDF;DEPENDS" ${ARGN})
  get_filename_component (_source_path ${_args_SOURCE} PATH)
  get_filename_component (_source_name ${_args_SOURCE} NAME)
  get_filename_component (_output_name_we ${_args_SOURCE} NAME_WE)

  # HTML
  if (${_args_HTML})
    set (_html_file ${CMAKE_CURRENT_BINARY_DIR}/${_output_name_we}_${lang}.html)
    add_custom_command (
      OUTPUT ${_html_file}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_source_path}/${lang}
      COMMAND ${ASCIIDOCTOR_EXECUTABLE} ${_args_ASCIIDOCTOR_OPTIONS}
      -b html5
      -a nofooter
      -a VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
      -a VERSION_MINOR=${PROJECT_VERSION_MINOR}
      -a VERSION_PATCH=${PROJECT_VERSION_PATCH}
      -a VERSION=${wsjtx_VERSION}
      --out-file=${_html_file} ${_source_name}
      DEPENDS ${_args_DEPENDS}
      COMMENT "Generating ${_html_file}"
      )
    list (APPEND _output_files ${_html_file})
  endif (${_args_HTML})

  # PDF
  if (_args_PDF AND EXISTS ${FOPUB_EXECUTABLE})
    set (_docbook_file ${CMAKE_CURRENT_BINARY_DIR}/${_output_name_we}_${lang}.xml)
    set (_pdf_file_we ${CMAKE_CURRENT_BINARY_DIR}/${_output_name_we}_${lang})
    if (${lang} MATCHES "^(en|es|fr)$") # en-us, fr-ca and es-{mx,co} use US-Letter or equivalent
      set (_usl_commands
	COMMAND ${FOPUB_EXECUTABLE} ARGS ${_docbook_file} ${_args_PDF} -param paper.type USLetter
	COMMAND ${CMAKE_COMMAND} ARGS -E rename ${_pdf_file_we}.pdf '${_pdf_file_we} \(USLetter\).pdf'
	)
      list (APPEND _output_files "${_pdf_file_we} (USLetter).pdf")
    endif ()
    list (APPEND _output_files "${_pdf_file_we}.pdf")
    add_custom_command (
      OUTPUT ${_docbook_file} ${_output_files}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_source_path}/${lang}
      COMMAND ${ASCIIDOCTOR_EXECUTABLE} ARGS ${_args_ASCIIDOCTOR_OPTIONS}
      -b docbook
      -a data-uri!
      -a VERSION_MAJOR=${wsjtx_VERSION_MAJOR}
      -a VERSION_MINOR=${wsjtx_VERSION_MINOR}
      -a VERSION_PATCH=${wsjtx_VERSION_PATCH}${BUILD_TYPE_REVISION}
      -a VERSION=${wsjtx_VERSION}
      -D ${CMAKE_CURRENT_BINARY_DIR}
      -o ${_docbook_file} ${_source_name}
      ${_usl_commands}
      COMMAND ${FOPUB_EXECUTABLE} ARGS ${_docbook_file} ${_args_PDF} -param paper.type A4
      COMMAND ${CMAKE_COMMAND} ARGS -E rename ${_pdf_file_we}.pdf '${_pdf_file_we}.pdf'
      DEPENDS ${_args_DEPENDS}
      COMMENT "Generating ${_output_files}"
      )
  endif (_args_PDF AND EXISTS ${FOPUB_EXECUTABLE})

  set (${_args_OUTPUT} ${_output_files} PARENT_SCOPE)
endfunction (document)

set (htmls)
set (pdfs)
foreach (lang ${LANGUAGES})
  set (_sources)
  foreach (_src ${UG_SRCS} ${UG_IMGS})
    list (APPEND _sources "user_guide/${lang}/${_src}")
  endforeach ()
  document(
    HTML
    SOURCE user_guide/wsjtx-main.adoc
    LANG "${lang}"
    OUTPUT html
    ASCIIDOCTOR_OPTIONS -d book -a data-uri -a toc=left
    DEPENDS ${common_SRCS} ${_sources}
    )
  document(
    PDF -param body.font.master 11 -param body.font.family "'Noto Sans, Helvetica, sans-serif'" -param title.font.family "'Noto Serif, Times New Roman, serif'" -param page.margin.inner 1cm -param page.margin.outer 1cm -param page.margin.top 0.75cm -param page.margin.bottom 0.5cm -param generate.toc 0
    SOURCE user_guide/wsjtx-main.adoc
    LANG "${lang}"
    OUTPUT pdf
    ASCIIDOCTOR_OPTIONS -d book
    DEPENDS ${common_SRCS} ${_sources}
    )
  list (APPEND htmls "${html}")
  list (APPEND pdfs "${pdf}")
endforeach ()

add_custom_target (docs ALL DEPENDS ${htmls} ${pdfs})

foreach (_html ${htmls})
  get_filename_component (_path ${_html} PATH)
  get_filename_component (_nwe ${_html} NAME_WE)
  get_filename_component (_ext ${_html} EXT)
  string (REGEX REPLACE "_en$" "" _nwe ${_nwe})
  install (FILES
    ${_html}
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    RENAME ${_nwe}-${wsjtx_VERSION}${_ext}
    #COMPONENT runtime
    )
endforeach ()
