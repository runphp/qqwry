CC      = gcc.exe -fexec-charset=gbk
BIN     = qqwry.exe
WINDRES = windres.exe
RES     = resource.o
OBJ     = main.o qqwry.o localIp.o $(RES)
# D:\MinGW\lib\libwsock32.a
# LIBS =  -L"D:/MinGW/lib" -lwsock32
LIBS =  -L"lib" -lwsock32
RM      = del -f
 
$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LIBS)
 
# main.o: qqwry.h 
# qqwry.o: qqwry.h localIp.h
# localIp.o: localIp.h
 
$(RES): resource.rc
	$(WINDRES) -o $(RES) resource.rc
 
.PHONY:clean
clean:
	$(RM) $(BIN) $(OBJ) *~