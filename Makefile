HDR_DIR = header C:\dev\lib\include C:\dev\lib\include\Shar\headers
HEADER = $(addprefix -I,$(HDR_DIR))
LIB_DIR = C:\dev\lib\lib-glew\Release\x64\ C:\dev\lib\lib-mingw\ C:\dev\lib\lib-sharo
LIB = $(addprefix -L, $(LIB_DIR))
LIB_NAMES = glfw3 glew32s opengl32 gdi32 sharo
LIBRARY = $(addprefix -l,$(LIB_NAMES))
CCOPTION = Wall std=gnu++11 D__USE_MINGW_ANSI_STDIO=1 g
FLAGS = $(addprefix -,$(CCOPTION))
CC = g++
OBJ_DIR = obj
OBJ = main.o sh_circle.o sh_shapes.o 
BUILD_DIR = build
SRC_DIR = S:\Test\sh_pong\src

$(BUILD_DIR)\sh_pong.exe: $(addprefix $(OBJ_DIR)\,$(OBJ)) 
	 $(CC) $(FLAGS) $(addprefix $(OBJ_DIR)\,$(OBJ)) -o $(BUILD_DIR)\sh_pong.exe $(LIB) $(LIBRARY)

$(OBJ_DIR)\main.o: $(SRC_DIR)\main.cpp 
	 $(CC) $(FLAGS) -c $(SRC_DIR)\main.cpp -o $(OBJ_DIR)\main.o $(HEADER)

$(OBJ_DIR)\sh_circle.o: $(SRC_DIR)\sh_circle.cpp 
	 $(CC) $(FLAGS) -c $(SRC_DIR)\sh_circle.cpp -o $(OBJ_DIR)\sh_circle.o $(HEADER)

$(OBJ_DIR)\sh_shapes.o: $(SRC_DIR)\sh_shapes.cpp 
	 $(CC) $(FLAGS) -c $(SRC_DIR)\sh_shapes.cpp -o $(OBJ_DIR)\sh_shapes.o $(HEADER)


clean: 
	del /Q $(OBJ_DIR)\*.o
	del /Q $(BUILD_DIR)\*.exe
