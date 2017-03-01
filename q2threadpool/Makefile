threadpool: ThreadPool.o ThreadPoolMain.o 
	g++ -g ThreadPool.o ThreadPoolMain.o -o threadpool -lpthread

ThreadPoolMain.o: ThreadPoolMain.cpp ThreadPool.h
	g++ -g -c ThreadPoolMain.cpp

ThreadPool.o: ThreadPool.cpp ThreadPool.h
	g++ -g -c ThreadPool.cpp 
	
clean:
	rm -rf *.o core threadpool
