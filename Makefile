all: 
	thrift -out processor/gen-cpp --gen cpp if/DeepScorerService.thrift
	thrift -out acceptor/gen-go --gen go if/DeepScorerService.thrift

clean:
	rm -rf processor/gen-cpp/*
	rm -rf acceptor/gen-go/*
