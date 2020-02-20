#
# Fix up the <project>Targets-[debug|release].cmake files by disabling 
# the check for imported targets. Otherwise a dependent project or work 
# area cannot properly configure if there was a build failure.
#
# As a second step relocate all absolute paths in the target files and
# replace them with ${some_variable}. This is used for relocating
# paths to LCG and DetCommon
#
set(PATHS_TO_FIX LCG_RELEASE_BASE;/cvmfs/sft.cern.ch/lcg/releases)
if("${PATHS_TO_FIX}" STREQUAL "")
  return()
endif()

# The Export directory where the target files are generated
set(TARGET_EXPORT_DIR /afs/cern.ch/work/n/nswdaq/public/sTGC_quick_and_dirty_baselines/NSWConfiguration//CMakeFiles/Export/share/cmake/NSWDAQ/x86_64-centos7-gcc8-opt)
file(GLOB targetFiles "${TARGET_EXPORT_DIR}/*Targets-*.cmake")
foreach(targetFile ${targetFiles})
  file(APPEND ${targetFile} "\nset(_IMPORT_CHECK_TARGETS)\n")
endforeach()

file(GLOB targetFile "${TARGET_EXPORT_DIR}/*Targets.cmake")
file(READ ${targetFile} input)

# This gymnastics is needed because a string with semicolons
# in it is a list in the CMake world....
set(output)
foreach(element ${input})
  set(replace ${PATHS_TO_FIX})
  while(replace)
    list(GET replace 0 var)
    list(GET replace 1 path)
    string(REPLACE ${path} \${${var}} element_out "${element}")
    if(output)
      list(APPEND output ${element_out})
    else()
      set(output ${element_out})
    endif()
    list(REMOVE_AT replace 0 1)
  endwhile()
endforeach()

file(WRITE ${targetFile} "${output}")

