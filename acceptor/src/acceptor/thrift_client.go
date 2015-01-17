package acceptor

import (
	"deep_score_service"
	"fmt"
	"git.apache.org/thrift.git/lib/go/thrift"
)

type RichClient struct {
  Client *deep_score_service.DeepScorerServiceClient
	Transport thrift.TTransport
}

func NewDataStreamClient(addr string) (*RichClient, error) {
	transportFactory := thrift.NewTFramedTransportFactory(thrift.NewTTransportFactory())
	protocolFactory := thrift.NewTBinaryProtocolFactoryDefault()
	var transport thrift.TTransport
	var err error
	transport, err = thrift.NewTSocket(addr)
	if err != nil {
		fmt.Println("Error opening socket:", err)
		return nil, err
	}
	transport = transportFactory.GetTransport(transport)
	if err := transport.Open(); err != nil {
		return nil, err
	}
  richclient := &RichClient{ Client:deep_score_service.NewDeepScorerServiceClientFactory(transport, protocolFactory), Transport:transport}
  return richclient, nil
}
