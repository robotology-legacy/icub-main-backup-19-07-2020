
set(LIBNAME eyecub)

if(NOT LIBEYECUB_FOUND)

    message(STATUS "Using LIBEYECUBConfig.cmake")
    
    set(LIB_DIR ${LIBEYECUB_DIR})
    set(LIBEYECUB_INCLUDE_DIRS ${LIB_DIR}/src)
    
    
    if(NESTED_BUILD)
    
      set(LIBEYECUB_LIBRARIES ${LIBNAME})
    
    else(NESTED_BUILD)
    
      find_library(LIBEYECUB_LIBRARIES ${LIBNAME} ${ICUB_DIR}/lib ${LIB_DIR})  
    
      if(NOT LIBEYECUB_LIBRARIES)
    
        find_library(LIBEYECUB_LIBRARIES_RELEASE ${LIBNAME} 
    		         ${ICUB_DIR}/lib/Release ${LIB_DIR}/Release NO_DEFAULT_PATH)
        find_library(LIBEYECUB_LIBRARIES_DEBUG ${LIBNAME} 
    		         ${ICUB_DIR}/lib/Debug ${LIB_DIR}/Debug NO_DEFAULT_PATH)
    
        if(LIBEYECUB_LIBRARIES_RELEASE AND NOT LIBEYECUB_LIBRARIES_DEBUG)
           set(LIBEYECUB_LIBRARIES ${LIBEYECUB_LIBRARIES_RELEASE} CACHE PATH "release version of library" FORCE)
        endif(LIBEYECUB_LIBRARIES_RELEASE AND NOT LIBEYECUB_LIBRARIES_DEBUG)
    
        if(LIBEYECUB_LIBRARIES_DEBUG AND NOT LIBEYECUB_LIBRARIES_RELEASE)
           set(LIBEYECUB_LIBRARIES ${LIBEYECUB_LIBRARIES_DEBUG} CACHE PATH "debug version of library" FORCE)
        endif(LIBEYECUB_LIBRARIES_DEBUG AND NOT LIBEYECUB_LIBRARIES_RELEASE)
    
        if(LIBEYECUB_LIBRARIES_DEBUG AND LIBEYECUB_LIBRARIES_RELEASE)
           set(LIBEYECUB_LIBRARIES optimized ${LIBEYECUB_LIBRARIES_RELEASE}
    			                 debug ${LIBEYECUB_LIBRARIES_DEBUG} CACHE PATH "debug and release version of library" FORCE)
        endif(LIBEYECUB_LIBRARIES_DEBUG AND LIBEYECUB_LIBRARIES_RELEASE)
    
        mark_as_advanced(LIBEYECUB_LIBRARIES_RELEASE)
        mark_as_advanced(LIBEYECUB_LIBRARIES_DEBUG)
    
      endif(NOT LIBEYECUB_LIBRARIES)
    
    endif(NESTED_BUILD)
    
    
    # Add YARP & libvislab dependencies
    find_package(CTRLLIB REQUIRED)
    find_package(IKIN REQUIRED)
    find_package(LIBVISLAB REQUIRED)
    set(LIBEYECUB_LIBRARIES ${LIBEYECUB_LIBRARIES} ${IKIN_LIBRARIES} ${CTRLLIB_LIBRARIES} ${YARP_LIBRARIES} ${LIBVISLAB_LIBRARIES})
    set(LIBEYECUB_INCLUDE_DIRS ${LIBEYECUB_INCLUDE_DIRS} ${IKIN_INCLUDE_DIRS} ${CTRLLIB_INCLUDE_DIRS} ${YARP_INCLUDE_DIRS} ${LIBVISLAB_INCLUDE_DIRS})

#    if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}${CMAKE_PATCH_VERSION} LESS 2.64)

      if(LIBEYECUB_INCLUDE_DIRS AND LIBEYECUB_LIBRARIES)
        SET(LIBEYECUB_FOUND 1)
      else(LIBEYECUB_INCLUDE_DIRS AND LIBEYECUB_LIBRARIES)
        SET(LIBEYECUB_FOUND 0)
      endif(LIBEYECUB_INCLUDE_DIRS AND LIBEYECUB_LIBRARIES)

#	else(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}${CMAKE_PATCH_VERSION} LESS 2.64)

#    	find_package_handle_standard_args(LIBEYECUB DEFAULT_MSG LIBEYECUB_INCLUDE_DIRS LIBEYECUB_LIBRARIES)

#    endif(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}${CMAKE_PATCH_VERSION} LESS 2.64)
    
endif(NOT LIBEYECUB_FOUND)
