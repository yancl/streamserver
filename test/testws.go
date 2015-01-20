package main

import (
	"fmt"
	"golang.org/x/net/websocket"
	"log"
)

const (
	Start  = 0
	Middle = 1
	Finish = 2
	Broken = 10
)

type AuthRequest struct {
	UserId string `json:"userid"`
	Passwd string `json:"passwd"`
}

type DataRequest struct {
	Flag int    `json:"flag"`
	Data []byte `json."data"`
}

type Response struct {
	Code int    `json:"code"`
	Msg  string `json:"msg"`
	Data []byte `json:"data"`
}

func main() {
	origin := "http://localhost/"
	url := "ws://localhost:8082/ws/upload"
	ws, err := websocket.Dial(url, "", origin)
	if err != nil {
		log.Fatal(err)
	}

	authReq := AuthRequest{UserId: "yancl", Passwd: "abcd"}

	if err := websocket.JSON.Send(ws, authReq); err != nil {
		log.Fatal(err)
	}

	req := DataRequest{Flag: Start, Data: []byte("this is datax")}

	if err := websocket.JSON.Send(ws, req); err != nil {
		log.Fatal(err)
	}

	req = DataRequest{Flag: Finish, Data: []byte("this is datay")}

	if err := websocket.JSON.Send(ws, req); err != nil {
		log.Fatal(err)
	}

	rsp := Response{}
	if err := websocket.JSON.Receive(ws, &rsp); err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Received: code:%d, msg:%s, data:%s.\n", rsp.Code, rsp.Msg, rsp.Data)
}
