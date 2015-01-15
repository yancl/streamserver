all:
	g++ -o ../bin/deep_score_server \
	deep_score_server.cpp \
	env_default.cpp \
	gen-cpp/deep_score_service_constants.cpp \
	gen-cpp/DeepScorerService.cpp \
	gen-cpp/deep_score_service_types.cpp \
	-L/usr/local/lib/ -L/usr/lib64/ -lthrift -lthriftnb -levent -lboost_system -lpthread -lrt

clean:
	rm -f ../bin/deep_score_server
