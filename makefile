CC = g++
BIN = zos_vfs
OBJ = Constants.o StringUtils.o VFSManager.o main.o

%.o: %.cpp
	$(CC) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $^ -o $@
	$(MAKE) clean

clean:
	-rm -f *.o

# make -f makefile
# make clean