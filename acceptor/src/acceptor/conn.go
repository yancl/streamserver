package acceptor

type Conn interface {
	//connect to the remote
	Connect() error

	//reconnect when broken
	Reconnect() error

	//close the connection
	Close() error
}
