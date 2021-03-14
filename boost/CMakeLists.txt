project(boost)

add_library(boost INTERFACE)
add_library(Boost ALIAS boost)

#set(BOOST_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/stage/lib)
#
#target_link_libraries(boost INTERFACE
#        ${BOOST_LIB_PATH}/libboost_date_time.a
#        ${BOOST_LIB_PATH}/libboost_regex.a
#        ${BOOST_LIB_PATH}/libboost_serialization.a
#        ${BOOST_LIB_PATH}/libboost_system.a
#        ${BOOST_LIB_PATH}/libboost_thread.a
#        ${BOOST_LIB_PATH}/libboost_wserialization.a
#        )

target_include_directories(boost INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
