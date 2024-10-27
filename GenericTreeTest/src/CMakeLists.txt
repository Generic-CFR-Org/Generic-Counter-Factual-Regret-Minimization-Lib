
set(HEADER_LIST "${GenericTreeTest_SOURCE_DIR}/include/NoLimitHoldem.h" "${GenericTreeTest_SOURCE_DIR}/include/rock_paper_scissors.h")

add_library(GenericTreeTestLib main.cpp ${HEADER_LIST})


target_include_directories(GenericTreeTestLib PUBLIC ../include)

source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)