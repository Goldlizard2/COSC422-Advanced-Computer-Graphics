cmake_minimum_required(VERSION 2.8)

project(OM_MeshViewer)

add_executable(OM_MeshViewer.out OM_MeshViewer.cpp)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenMesh REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${OpenMesh_INCLUDE_DIRS})

target_link_libraries(OM_MeshViewer.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${OpenMesh_LIBRARIES}  OpenMeshCore OpenMeshTools)

