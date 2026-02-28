CCC=g++
CFLAGS=-Wall -I/urs/local/include
CFLAGS=
LDFLAGS= -lGL -lGLEW -lGLU -lglut  -larmadillo 


SRC=main.cpp Icosphere.cpp OpenGLManager.cpp Scene.cpp InputManager.cpp BezierCurve.cpp Renderable.cpp BezierSurface.cpp ParametricSpace.cpp ParametricMapping.cpp UVTriangleManager.cpp BSplineCurve.cpp BSplineSurface.cpp Curve.cpp RevolutionSurface.cpp HalfEdge.cpp IFSEdge.cpp IFSManager.cpp IFSTransform.cpp UVTriangleIFS.cpp
DEBUGFLAG=-g

OBJ= $(SRC:.cpp=.o)
BIN=./
EXEC=main

all: $(EXEC)

main: $(OBJ)
		$(CCC) -g  -o $(BIN)$@ $^ $(LDFLAGS)

%.o: %.cpp
		$(CCC) -g -o $@ -c $< $(CFLAGS)


clean:
		rm -rf *.o

mrproper: clean
		rm -rf $(BIN)$(EXEC)

