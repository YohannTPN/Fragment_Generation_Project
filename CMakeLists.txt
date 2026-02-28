cmake_minimum_required(VERSION 3.10)
project(BSplineVolumeVoronoi)

# C++14 ou supérieur requis pour CGAL
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouver les packages requis
find_package(CGAL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Armadillo REQUIRED)

# Inclure les répertoires
include_directories(
    ${CGAL_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${ARMADILLO_INCLUDE_DIRS}
)

# Fichiers sources
set(SOURCES
    main.cpp
    Scene.cpp
    InputManager.cpp
    OpenGLManager.cpp
    ParametricMapping.cpp
    ParametricSpace.cpp
    Renderable.cpp
    RevolutionSurface.cpp
    UVTriangleManager.cpp
    Curve.cpp
    BezierCurve.cpp
    BezierSurface.cpp
    BSplineCurve.cpp
    BSplineSurface.cpp
    Icosphere.cpp
    HalfEdge.cpp
    IFSEdge.cpp
    IFSManager.cpp
    IFSTransform.cpp
    UVTriangleIFS.cpp
)

# Exécutable
add_executable(main ${SOURCES})

# Lier les bibliothèques
target_link_libraries(main
    ${CGAL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${ARMADILLO_LIBRARIES}
)

# Flags de compilation pour CGAL
target_compile_definitions(main PRIVATE ${CGAL_DEFINITIONS})