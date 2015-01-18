all:
	g++ -o ../bin/deep_score_server \
	deep_score_server.cpp \
	stream_handler.cpp \
	env_default.cpp \
	compute_unit.cpp \
	store.cpp \
	fifo_map.cpp \
	message.cpp \
	callback.cpp \
	http_client.cpp \
	gen-cpp/deep_score_service_constants.cpp \
	gen-cpp/DeepScorerService.cpp \
	gen-cpp/deep_score_service_types.cpp \
	-L/usr/local/lib/ -L/usr/lib64/ -lthrift -lthriftnb -levent -lcurl -lglog -lboost_thread -lboost_system -lpthread -lrt

clean:
	rm -f ../bin/deep_score_server
