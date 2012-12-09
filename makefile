all: httpTest

clean:
	rm *.exe

httpTest: httpTest.c
	$(CC) -Os -D httpTestMain=main  -Wno-pointer-sign httpTest.c  -o httpTest -lpthread

httpTest.exe: httpTest.c
	$(CC) -Os -D httpTestMain=main -I../i httpTest.c -lws2_32 -lpsapi -o httpTest.exe


