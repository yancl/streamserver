all: 
	thrift -out processor/gen-cpp --gen cpp protocol/DeepScorerService.thrift
	thrift -out acceptor/gen-go --gen go protocol/DeepScorerService.thrift

clean:
	rm -rf processor/gen-cpp/*
	rm -rf acceptor/gen-go/*
