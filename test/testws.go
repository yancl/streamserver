package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"golang.org/x/net/websocket"
	"io"
	"log"
	"os"
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

func printUsage() {
	fmt.Printf("Usage:\n./testws -f filename\n")
}

var chunkSize = 4096

func main() {
	filenamePtr := flag.String("f", "", "file name to compute")
	flag.Parse()

	if *filenamePtr == "" {
		printUsage()
		return
	}

	f, err := os.Open(*filenamePtr)
	if err != nil {
		fmt.Printf("open file:%s failed!,err:%s\n", *filenamePtr, err)
		return
	}

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

	firstMessage := true
	flag := Start

	for {
		var data = make([]byte, chunkSize)
		_, err := f.Read(data)
		if err == io.EOF {
			break
		}

		req := DataRequest{Flag: flag, Data: data}
		if err := websocket.JSON.Send(ws, req); err != nil {
			log.Fatal(err)
			return
		}

		if firstMessage {
			firstMessage = false
			flag = Middle
		}
	}

	//send end slice
	flag = Finish
	req := DataRequest{Flag: flag, Data: nil}
	if err := websocket.JSON.Send(ws, req); err != nil {
		log.Fatal(err)
	}

	rsp := Response{}
	if err := websocket.JSON.Receive(ws, &rsp); err != nil {
		log.Fatal(err)
	}

	if data, err := json.Marshal(rsp); err != nil {
		fmt.Printf("unmarshal response failed, err:%v\n", err)
	} else {
		fmt.Println(string(data))
	}

}
