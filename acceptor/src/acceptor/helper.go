package acceptor

import (
	"deepscore"
	//"fmt"
)

var (
	callbackHost string
	callbackPort int32
)

type Message struct {
	flag deepscore.SliceFlag
	seq  int32
	data []byte
}

func InitCallbackParams(host string, port int32) {
	callbackHost = host
	callbackPort = port
}

func SendMessage(sessionKey string, message *Message) error {
	//compose messages
	length := 0
	capacity := 1
	messages := make([]*deepscore.DataSlice, length, capacity)
	messages = append(messages, &deepscore.DataSlice{Key: sessionKey, Val: message.data, Number: message.seq, Flag: message.flag,
		Host: callbackHost, Port: callbackPort})
	return ReqRouter.SendMessage(sessionKey, messages)
}
