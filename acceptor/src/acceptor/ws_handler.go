package acceptor

import (
	"fmt"
	"golang.org/x/net/websocket"
)

const (
	Start  = 0
	Middle = 1
	Finish = 2
	Broken = 10
)

type WSRequest struct {
	UserId string `json:"userid"`
	Passwd string `json:"passwd"`
	Flag   int    `json:"flag"`
	Data   []byte `json."data"`
}

type WSResponse struct {
	Code int    `json:"code"`
	Msg  string `json:"msg"`
}

func HandleVoidStream(ws *websocket.Conn) {
	var req WSRequest
	if err := websocket.JSON.Receive(ws, &req); err != nil {
		fmt.Printf("receive from wsocket failed, err:%v\n", err)
		return
	}
	fmt.Printf("uid:%s\npasswd:%s\nflag:%d\ndata:%s\n", req.UserId, req.Passwd, req.Flag, req.Data)

	var rsp = WSResponse{Code: 0, Msg: "ok"}
	if err := websocket.JSON.Send(ws, rsp); err != nil {
		fmt.Printf("send response failed, err:%v\n", err)
		return
	}
}
