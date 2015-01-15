all: 
	thrift -out processor/src/gen-cpp --gen cpp if/DeepScorerService.thrift
	thrift -out acceptor/src/gen-go --gen go if/DeepScorerService.thrift
	cd processor/src/ && make && cd -

clean:
	rm -rf processor/src/gen-cpp/*
	rm -rf acceptor/src/gen-go/*
	cd processor/src/ && make clean && cd -
