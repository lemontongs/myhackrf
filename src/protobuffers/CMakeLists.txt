
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER packet.proto)
add_library(protobuffers ${PROTO_HEADER} ${PROTO_SRC})
target_include_directories(protobuffers PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../protobuffers/)

PROTOBUF_GENERATE_PYTHON(PROTO_PY packet.proto)
add_custom_target(protobuffers_py ALL
                  COMMAND cp ${PROTO_PY} ${CMAKE_CURRENT_BINARY_DIR}/../../bin/
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/myhackrf.py       ${CMAKE_CURRENT_BINARY_DIR}/../../bin/
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/plot_waterfall.py ${CMAKE_CURRENT_BINARY_DIR}/../../bin/
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/plot_iq.py        ${CMAKE_CURRENT_BINARY_DIR}/../../bin/
                  DEPENDS ${PROTO_PY})
