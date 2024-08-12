set(COMMS_SRCS 
  src/local_channel_container.cpp
  src/local_communicator.cpp
)

add_library(communication SHARED ${COMMS_SRCS})
target_include_directories(communication PRIVATE ../)
target_include_directories(communication PRIVATE include)

set(TEST_INCLUDES include ../ ${DOCTEST_INCLUDE_DIR})

add_cxx_doctest_driver(TestSendRecv SRCS tests/send_recv.test.cpp INCLUDES ${TEST_INCLUDES} LINKS thread_pool communication TEST_FANCY_NAME send-recv)
add_cxx_doctest_driver(TestSendRecv2 SRCS tests/send_recv_2.test.cpp INCLUDES ${TEST_INCLUDES} LINKS thread_pool communication TEST_FANCY_NAME send-recv-2)
