package acceptor

import (
	"deep_score_service"
	"fmt"
	"git.apache.org/thrift.git/lib/go/thrift"
)

func handleClient(client *deep_score_service.DeepScorerServiceClient) (err error) {
	//compose messages
	capacity := 2
	messages := make([]*deep_score_service.DataEntry, 0, capacity)
	for i := 0; i < 2; i++ {
		key := fmt.Sprintf("key_%v", i)
		messages = append(messages, &deep_score_service.DataEntry{Key: key, Val: []byte{0x00, 0x01}, Number: 1, Last: false})
	}

	for _, v := range messages {
		fmt.Printf("key:%s\n", v.Key)
	}

	//send messages
	r, err := client.AddDataEntryStream(messages)
	if err != nil {
		fmt.Printf("add messages failed,err:", err)
	} else {
		fmt.Printf("add messages ok, r:%v", r)
	}
	return err
}

func runClient(transportFactory thrift.TTransportFactory, protocolFactory thrift.TProtocolFactory, addr string) error {
	var transport thrift.TTransport
	var err error
	transport, err = thrift.NewTSocket(addr)
	if err != nil {
		fmt.Println("Error opening socket:", err)
		return err
	}
	transport = transportFactory.GetTransport(transport)
	defer transport.Close()
	if err := transport.Open(); err != nil {
		return err
	}
	return handleClient(deep_score_service.NewDeepScorerServiceClientFactory(transport, protocolFactory))
}

func TestThriftClient(addr string) {
	transportFactory := thrift.NewTFramedTransportFactory(thrift.NewTTransportFactory())
	protocolFactory := thrift.NewTBinaryProtocolFactoryDefault()
	runClient(transportFactory, protocolFactory, addr)
}
