all: 
	thrift -out processor/src/gen-cpp --gen cpp if/deep_score_service.thrift
	thrift -out acceptor/src/gen-go/src --gen go if/deep_score_service.thrift
	cd processor/src/ && make framework && cd -

clean:
	rm -rf processor/src/gen-cpp/*
	rm -rf acceptor/src/gen-go/src/*
	cd processor/src/ && make clean && cd -
