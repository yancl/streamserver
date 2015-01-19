framework:
	g++ -g -o ../bin/deep_score_server \
	deep_score_server.cpp \
	stream_handler.cpp \
	env_default.cpp \
	compute_unit.cpp \
	store.cpp \
	fifo_map.cpp \
	message.cpp \
	callback.cpp \
	http_client.cpp \
	url_coder.cpp \
	gen-cpp/deep_score_service_constants.cpp \
	gen-cpp/DeepScorerService.cpp \
	gen-cpp/deep_score_service_types.cpp \
	-L/usr/local/lib/ -L/usr/lib64/ -lthrift -lthriftnb -levent -lcurl -lglog -lboost_thread -lboost_system -lpthread -lrt

deepscorer:
	/opt/centos/devtoolset-1.1/root/usr/bin/g++ \
  -o ../bin/deep_score_server \
  deep_score_server.cpp \
	stream_handler.cpp \
	env_default.cpp \
  compute_unit_with_hook.cpp \
	store.cpp \
	fifo_map.cpp \
	message.cpp \
  callback.cpp \
	http_client.cpp \
	url_coder.cpp \
  gen-cpp/deep_score_service_constants.cpp \
  gen-cpp/DeepScorerService.cpp \
  gen-cpp/deep_score_service_types.cpp \
  /home/yxf/deep-scorer/src/lls/lib/libdeepscorer.a \
  /opt/ATLAS/lib/libcblas.a /opt/ATLAS/lib/libatlas.a /opt/ATLAS/lib/libf77blas.a /opt/ATLAS/lib/liblapack.a \
  /opt/ATLAS/lib/libcblas.a /opt/ATLAS/lib/libatlas.a \
  -L/usr/local/lib/ -L/usr/lib64/ -L/opt/ATLAS/lib/ \
  -lthrift -lthriftnb -levent -lcurl -lglog -lboost_thread -lboost_system \
  -lpthread -lstdc++ -lrt -lm -ldl

deepscorer_test:
	/opt/centos/devtoolset-1.1/root/usr/bin/g++ \
  -o ../bin/deepscorer_test \
  deepscorer_test.cpp \
  /home/yxf/deep-scorer/src/lls/lib/libdeepscorer.a \
  /opt/ATLAS/lib/libcblas.a /opt/ATLAS/lib/libatlas.a /opt/ATLAS/lib/libf77blas.a /opt/ATLAS/lib/liblapack.a \
  /opt/ATLAS/lib/libcblas.a /opt/ATLAS/lib/libatlas.a \
  -L/usr/local/lib/ -L/usr/lib64/ -L/opt/ATLAS/lib/ \
  -lpthread -lstdc++ -lrt -lm -ldl

clean:
	rm -f ../bin/deep_score_server
