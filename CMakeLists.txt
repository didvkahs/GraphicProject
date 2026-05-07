cmake_minimum_required(VERSION 3.15)

# 프로젝트 이름과 언어 지정
project(GraphicsProject LANGUAGES C CXX)

# 소스 파일 모음
set(SOURCES
    BmpUtil/BmpReader.cpp
    BmpUtil/BmpReader.h
    BmpUtil/typedef.h

    D3D11/D3DResources.cpp
    D3D11/D3DResources.h
    D3D11/Shaders/defaultShader.fx

    GLTFUtil/GLTFReader.cpp
    GLTFUtil/GLTFReader.h

    GraphicsProject.cpp
)

# 실행 파일 생성
add_executable(GraphicsProject ${SOURCES})

# include 경로 추가
target_include_directories(GraphicsProject PRIVATE
    ${CMAKE_SOURCE_DIR}/BmpUtil
    ${CMAKE_SOURCE_DIR}/D3D11
    ${CMAKE_SOURCE_DIR}/GLTFUtil
)

# --- Visual Studio 필터(소스 그룹) ---
source_group("BmpUtil" FILES
    BmpUtil/BmpReader.cpp
    BmpUtil/BmpReader.h
    BmpUtil/typedef.h
)

source_group("D3D11" FILES
    D3D11/D3DResources.cpp
    D3D11/D3DResources.h
)

source_group("D3D11\\Shaders" FILES
    D3D11/Shaders/defaultShader.fx
)

source_group("GLTFUtil" FILES
    GLTFUtil/GLTFReader.cpp
    GLTFUtil/GLTFReader.h
)

source_group("Source Files" FILES GraphicsProject.cpp)
