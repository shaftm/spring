# See README.md for usage instructions

FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework)
If    (NOT Boost_FOUND)
	Message(STATUS "Note: Unit tests will not be built: Boost::test library was not found")
Else  (NOT Boost_FOUND)
	# defines spring_test_compile_fail macro
	INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/tools/CompileFailTest/CompileFailTest.cmake)

	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

	FIND_PACKAGE(SDL REQUIRED)
	INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src-generated/engine)

	SET(ENGINE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/rts")
	INCLUDE_DIRECTORIES(${ENGINE_SOURCE_DIR})
	If	(NOT (WIN32 OR Boost_USE_STATIC_LIBS))
		#Win32 tests links static
		add_definitions(-DBOOST_TEST_DYN_LINK)
	EndIf	(NOT (WIN32 OR Boost_USE_STATIC_LIBS))
	add_definitions(-DSYNCCHECK -DUNIT_TEST)

	Set(test_Log_sources
			"${ENGINE_SOURCE_DIR}/System/SafeCStrings.c"
			"${ENGINE_SOURCE_DIR}/System/Log/Backend.cpp"
			"${ENGINE_SOURCE_DIR}/System/Log/LogUtil.c"
			"${ENGINE_SOURCE_DIR}/System/Log/DefaultFilter.cpp"
			"${ENGINE_SOURCE_DIR}/System/Log/DefaultFormatter.cpp"
			"${ENGINE_SOURCE_DIR}/System/Log/FramePrefixer.cpp"
			"${ENGINE_SOURCE_DIR}/System/Log/ConsoleSink.cpp"
			"${ENGINE_SOURCE_DIR}/System/Log/StreamSink.cpp"
		)

	Add_Custom_Target(tests)
	#FIXME: hardcoded path (is used in buildbot/slave/make_installer.sh, too)
	add_custom_target(check WINEPATH=/tmp/spring/inst/${CMAKE_INSTALL_PREFIX} ${CMAKE_CTEST_COMMAND} --output-on-failure)



################################################################################
### UDPListener

	Set(test_UDPListener_src
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/Net/TestUDPListener.cpp"
			"${ENGINE_SOURCE_DIR}/Game/GameVersion.cpp"
			"${ENGINE_SOURCE_DIR}/System/BaseNetProtocol.cpp"
			"${ENGINE_SOURCE_DIR}/System/Net/UDPListener.cpp"
			"${ENGINE_SOURCE_DIR}/System/Net/RawPacket.cpp"
			"${ENGINE_SOURCE_DIR}/System/Net/PackPacket.cpp"
			"${ENGINE_SOURCE_DIR}/System/Net/ProtocolDef.cpp"
			"${ENGINE_SOURCE_DIR}/System/Net/UDPConnection.cpp"
			"${ENGINE_SOURCE_DIR}/System/Net/Connection.cpp"
			"${ENGINE_SOURCE_DIR}/System/Net/Socket.cpp"
			"${ENGINE_SOURCE_DIR}/System/CRC.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/NullGlobalConfig.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/Nullerrorhandler.cpp"
			${test_Log_sources}
		)

	ADD_EXECUTABLE(test_UDPListener ${test_UDPListener_src})
	TARGET_LINK_LIBRARIES(test_UDPListener
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
			${Boost_SYSTEM_LIBRARY}
			${SDL_LIBRARY}
			${WS2_32_LIBRARY}
			7zip
		)

	Add_Dependencies(test_UDPListener generateVersionFiles)

	ADD_TEST(NAME testUDPListener COMMAND test_UDPListener)
	Add_Dependencies(tests test_UDPListener)



################################################################################
### ILog

	Set(test_ILog_src
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/Log/TestILog.cpp"
			"${ENGINE_SOURCE_DIR}/System/Log/FileSink.cpp"
			"${ENGINE_SOURCE_DIR}/System/Log/OutputDebugStringSink.cpp"
			${test_Log_sources}
		)

	ADD_EXECUTABLE(test_ILog ${test_ILog_src})
	TARGET_LINK_LIBRARIES(test_ILog
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
		)

	ADD_TEST(NAME testILog COMMAND test_ILog)
	Add_Dependencies(tests test_ILog)



################################################################################
### SyncedPrimitive

	Set(test_SyncedPrimitive_src
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/Sync/TestSyncedPrimitive.cpp"
			"${ENGINE_SOURCE_DIR}/System/Sync/SyncChecker.cpp"
		)

	ADD_EXECUTABLE(test_SyncedPrimitive ${test_SyncedPrimitive_src})
	TARGET_LINK_LIBRARIES(test_SyncedPrimitive
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
		)

	ADD_TEST(NAME testSyncedPrimitive COMMAND test_SyncedPrimitive)
	Add_Dependencies(tests test_SyncedPrimitive)


################################################################################
### RectangleOptimizer

	Set(test_RectangleOptimizer_src
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/Misc/RectangleOptimizer.cpp"
			"${ENGINE_SOURCE_DIR}/System/Misc/RectangleOptimizer.cpp"
			${test_Log_sources}
		)

	ADD_EXECUTABLE(test_RectangleOptimizer ${test_RectangleOptimizer_src})
	TARGET_LINK_LIBRARIES(test_RectangleOptimizer
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
			${Boost_SYSTEM_LIBRARY}
		)

	ADD_TEST(NAME testRectangleOptimizer COMMAND test_RectangleOptimizer)
	Add_Dependencies(tests test_RectangleOptimizer)


################################################################################
### Float3

	Set(test_Float3_src
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/testFloat3.cpp"
		)

	ADD_EXECUTABLE(test_Float3 ${test_Float3_src})
	TARGET_LINK_LIBRARIES(test_Float3
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
			${Boost_SYSTEM_LIBRARY}
		)

	ADD_TEST(NAME testFloat3 COMMAND test_Float3)
	Add_Dependencies(tests test_Float3)


################################################################################
### BitwiseEnum

	Set(test_BitwiseEnum_src
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/Misc/TestBitwiseEnum.cpp"
		)

	ADD_EXECUTABLE(test_BitwiseEnum ${test_BitwiseEnum_src})
	TARGET_LINK_LIBRARIES(test_BitwiseEnum
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
		)

	# positive tests (should compile fine)
	ADD_TEST(NAME testBitwiseEnum COMMAND test_BitwiseEnum)
	Add_Dependencies(tests test_BitwiseEnum)

	# negative tests (must not compile)
	spring_test_compile_fail(testBitwiseEnum_fail1 ${test_BitwiseEnum_src} "-DTEST1")
	spring_test_compile_fail(testBitwiseEnum_fail2 ${test_BitwiseEnum_src} "-DTEST2")
	spring_test_compile_fail(testBitwiseEnum_fail3 ${test_BitwiseEnum_src} "-DTEST3")


################################################################################
### FileSystem

	Set(test_FileSystem_src
			"${ENGINE_SOURCE_DIR}/System/FileSystem/FileSystem.cpp"
			"${ENGINE_SOURCE_DIR}/System/FileSystem/FileSystemAbstraction.cpp"
			"${ENGINE_SOURCE_DIR}/System/Util.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/engine/System/FileSystem/TestFileSystem.cpp"
			${test_Log_sources}
		)

	ADD_EXECUTABLE(test_FileSystem ${test_FileSystem_src})
	TARGET_LINK_LIBRARIES(test_FileSystem
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
			${Boost_REGEX_LIBRARY}
		)

	ADD_TEST(NAME testFileSystem COMMAND test_FileSystem)
	Add_Dependencies(tests test_FileSystem)


################################################################################
### LuaSocketRestrictions
	add_definitions("-DTEST")
	Set(test_LuaSocketRestrictions_src
			"${CMAKE_CURRENT_SOURCE_DIR}/lib/luasocket/TestRestriction.cpp"
			"${ENGINE_SOURCE_DIR}/lib/luasocket/src/restrictions.cpp"
			${test_Log_sources}
		)

	ADD_EXECUTABLE(test_LuaSocketRestrictions ${test_LuaSocketRestrictions_src})
	TARGET_LINK_LIBRARIES(test_LuaSocketRestrictions
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
			${Boost_REGEX_LIBRARY}
		)

	ADD_TEST(NAME LuaSocketRestrictions COMMAND test_LuaSocketRestrictions)
	Add_Dependencies(tests test_LuaSocketRestrictions)


################################################################################


EndIf (NOT Boost_FOUND)

add_subdirectory(headercheck)

