FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread regex unit_test_framework)

SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)

MACRO_CHECK_SHOULD_BUILD(procmeminfo _NOT_WIN32)
IF(procmeminfo_SHOULD_BUILD)
  MACRO_ADD_LIBRARY(procmeminfo
    SRCS
    ProcMemInfo.cpp
    MRandom.cpp
    random.cpp
    profiler/Profiler.cpp
    profiler/TimeChecker.cpp
    profiler/YString.cpp)
ENDIF(procmeminfo_SHOULD_BUILD)

MACRO_CHECK_SHOULD_BUILD(
  izene_log
  procmeminfo_SHOULD_BUILD
  Glog_FOUND
  )
IF(izene_log_SHOULD_BUILD)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  MACRO_ADD_LIBRARY(izene_log SRCS izene_log.cpp)
ENDIF(izene_log_SHOULD_BUILD)

SET(ticpp_SHOULD_BUILD TRUE)
MACRO_CHECK_SHOULD_BUILD(
  ticpp
  ticpp_SHOULD_BUILD
  )

IF(ticpp_SHOULD_BUILD)
  SET(ticpp_SRC
    ticpp/ticpp.cpp
    ticpp/tinyxmlerror.cpp
    ticpp/tinystr.cpp
    ticpp/tinyxml.cpp
    ticpp/tinyxmlparser.cpp
    )

  MACRO_ADD_LIBRARY(ticpp SRCS ${ticpp_SRC})

ENDIF(ticpp_SHOULD_BUILD)

MACRO_CHECK_SHOULD_BUILD(
  izene_util
  Boost_FOUND
  Threads_FOUND
  )

IF(izene_util_SHOULD_BUILD)

  SET(util_SRC
    scheduler.cpp
    mkgmtime.cpp
    block_pool.cpp
    thread-pool/ThreadObject.cpp
    thread-pool/ThreadObjectPool.cpp

    io/BufferedInput.cpp
    io/FSBufferedInput.cpp
    io/MMapBufferedInput.cpp
    io/BufferedOutput.cpp
    io/FSBufferedOutput.cpp

    compression/int/pfordelta_compressor.cpp
    compression/int/pfordelta_mix_compressor.cpp
    compression/int/pfordelta_mix_s16_compressor.cpp
    compression/int/s16_compressor.cpp
    compression/int/unpack.cpp
    compression/int/vbyte_compressor.cpp
    compression/int/Simple16.cpp
    compression/int/PForDelta.cpp
    compression/int/BitsWriter.cpp
    compression/int/err_utils.cpp
    compression/int/int_utils.cpp
    compression/int/OPTPForDelta.cpp
    compression/int/VSE-R.cpp
    compression/int/BitsReader.cpp
    compression/int/VSEncoding.cpp
    compression/int/VSEncodingNaive.cpp
    compression/int/decGamma.cpp
    compression/int/decDelta.cpp
    compression/int/decUnary.cpp

    compression/int/fastpfor/bitpackingaligned.cpp
    compression/int/fastpfor/bitpacking.cpp
    compression/int/fastpfor/bitpackingunaligned.cpp
    compression/int/fastpfor/bitpacksimd.cpp

    compression/int/compressedset/Codec.cpp
    compression/int/compressedset/CompressedDeltaChunk.cpp
    compression/int/compressedset/CompressedSet.cpp
    compression/int/compressedset/LazyAndSet.cpp
    compression/int/compressedset/LazyOrSet.cpp
    compression/int/compressedset/bitpacking/util.cpp

    compressed_vector/detail/Compressor.cpp

    sysinfo/commandparser.cpp
    sysinfo/dmiparser.cpp

    cronexpression.cpp
    timer.cpp
    CBitArray.cpp
    singleton.cpp
    streambuf.cpp
    stringprintf.cpp
    scd_parser.cpp
    DynamicLibrary.cpp
    modp_numtoa.cpp
    timestamp.cpp
    MemPool.cpp
    mem_pool.cpp
    kv2string.cpp
    csv.cpp

    driver/Keys.cpp
    driver/Request.cpp
    driver/Response.cpp
    driver/RestrictedObjectValue.cpp
    driver/Router.cpp
    driver/value/Value.cpp
    driver/value/ValueTypeHelper.cpp
    driver/value/formatters.cpp
    driver/Reader.cpp
    driver/readers/JsonReader.cpp
    driver/Writer.cpp
    driver/writers/DriverValue2JsonValue.cpp
    driver/writers/JsonWriter.cpp
    driver/writers/PrettyJsonWriter.cpp
    driver/Parser.cpp
    driver/DriverConnection.cpp
    driver/DriverConnectionContext.cpp
    driver/DriverConnectionFirewall.cpp
    driver/DriverServer.cpp
    driver/IPRestrictor.cpp

    osgi/BundleConfiguration.cpp
    osgi/IBundleContextImpl.cpp
    osgi/ConfigFileReader.cpp
    osgi/ServiceEvent.cpp
    osgi/ServiceListenerInfo.cpp
    osgi/BundleInfo.cpp
    osgi/OSGIException.cpp
    osgi/ServiceInfo.cpp
    osgi/ConfigurationException.cpp
    osgi/IServiceRegistrationImpl.cpp
    osgi/ServiceTracker.cpp
    osgi/BundleInfoBase.cpp
    osgi/Properties.cpp
    osgi/ServiceReference.cpp
    osgi/ObjectCreationException.cpp
    osgi/admin/StartBundleFromDLLCmd.cpp
    osgi/admin/IAdministrationServiceImpl.cpp
    osgi/admin/StartBundlesFromFileCmd.cpp
    osgi/admin/StartBundleCmd.cpp
    osgi/admin/AdministrationActivator.cpp
    osgi/admin/DumpAllBundlesCmd.cpp
    osgi/admin/StopBundleCmd.cpp
    osgi/admin/DumpBundleInfoCmd.cpp
    osgi/admin/StopAllBundlesCmd.cpp
    osgi/util/LoggerImpl.cpp
    osgi/util/StringTokenizer.cpp
    osgi/util/LoggerFactory.cpp
    datastream/sketch/madoka/sketch.cc
    datastream/sketch/madoka/file.cc

    string/StringUtils.cpp

    test/BoostTestThreadSafety.cpp
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
  )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(izene_util SRCS ${util_SRC})

  TARGET_LINK_LIBRARIES(izene_util
    ${Boost_LIBRARIES}
    json
    ${CMAKE_THREAD_LIBS_INIT}
    dl
    )

ENDIF(izene_util_SHOULD_BUILD)
