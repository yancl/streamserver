package acceptor

import (
	"deepscore"
	"fmt"
	"git.apache.org/thrift.git/lib/go/thrift"
)

type ThriftConn struct {
	Conn
	Client    *deepscore.DeepScorerServiceClient
	Transport thrift.TTransport
	addr      string
	connected bool
}

func NewThriftConn(addr string) (Conn, error) {
	fmt.Printf("create new connect to addr:%s", addr)
	conn := ThriftConn{addr: addr, connected: false}
	err := conn.Connect()
	if err != nil {
		return conn, err
	}
	return conn, nil
}

func (thriftConn ThriftConn) Connect() error {
	if thriftConn.connected {
		fmt.Println("connection already connected!")
		return nil
	}

	transportFactory := thrift.NewTFramedTransportFactory(thrift.NewTTransportFactory())
	protocolFactory := thrift.NewTBinaryProtocolFactoryDefault()
	var transport thrift.TTransport
	var err error
	transport, err = thrift.NewTSocket(thriftConn.addr)
	if err != nil {
		fmt.Println("Error opening socket:", err)
		return err
	}
	transport = transportFactory.GetTransport(transport)
	if err := transport.Open(); err != nil {
		return err
	}

	thriftConn.Client = deepscore.NewDeepScorerServiceClientFactory(transport, protocolFactory)
	thriftConn.Transport = transport

	thriftConn.connected = true
	return nil
}

func (thriftConn ThriftConn) Reconnect() error {
	err := thriftConn.Close()
	if err != nil {
		return err
	}
	err = thriftConn.Connect()
	return err
}

func (thriftConn ThriftConn) Close() error {
	thriftConn.Transport.Close()
	thriftConn.connected = false
	return nil
}
