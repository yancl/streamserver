all:
	g++ -o ../bin/stream_server \
	stream_server.cpp \
	env_default.cpp \
	../gen-cpp/DeepScorerService_constants.cpp \
	../gen-cpp/DeepScorerService.cpp \
	../gen-cpp/DeepScorerService_types.cpp \
	-L/usr/local/lib/ -L/usr/lib64/ -lthrift -lthriftnb -levent -lboost_system -lpthread -lrt

clean:
	rm -f ../bin/stream_server
