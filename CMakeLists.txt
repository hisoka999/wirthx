cmake_minimum_required(VERSION 3.8)

project(wirthx VERSION 0.1 DESCRIPTION "wirthx is a pascal interpreter and in future compiler")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


file(GLOB SRC 
    src/compare.cpp
    src/ast/VariableDefinition.cpp
    src/ast/ASTNode.cpp
    src/ast/PrintNode.cpp
    src/ast/NumberNode.cpp
    src/ast/StringConstantNode.cpp
    src/ast/BinaryOperationNode.cpp
    src/ast/InputNode.cpp
    src/ast/ComparrisionNode.cpp
    src/ast/VariableAccessNode.cpp
    src/ast/LogicalExpressionNode.cpp
    src/ast/IfConditionNode.cpp
    src/ast/WhileNode.cpp
    src/ast/VariableAssignmentNode.cpp
    src/ast/FunctionDefinitionNode.cpp
    src/ast/ReturnNode.cpp
    src/ast/FunctionCallNode.cpp
    src/ast/BlockNode.cpp
    src/ast/UnitNode.cpp
    src/ast/SystemFunctionCallNode.cpp
    src/interpreter/Stack.cpp
    src/interpreter/interpreter.cpp
    src/Lexer.cpp
    src/Parser.cpp
    main.cpp )
INCLUDE_DIRECTORIES("src")

include(FetchContent)

option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)

endif()

# Let's set compiler-specific flags
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # G++
    add_compile_options(-Wall -Wextra -Werror)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # MSVC
    target_compile_options(${PROJECT_NAME} PRIVATE /EHsc /MTd /W2 /c)

    # Set the DLLEXPORT variable to export symbols
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN_EXPORT)
endif()

add_executable(${PROJECT_NAME} "main.cpp" ${SRC} )
