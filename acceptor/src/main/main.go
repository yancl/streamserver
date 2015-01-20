package main

import (
	"acceptor"
	"config"
	"flag"
	"fmt"
	"golang.org/x/net/websocket"
	"log"
	"net/http"
	"strconv"
	"strings"
)

func getHostPort(addr string) (string, int32, error) {
	hostport := strings.Split(addr, ":")
	host := hostport[0]
	port, err := strconv.Atoi(hostport[1])
	return host, int32(port), err
}

func printUsage() {
	fmt.Println("Usage:\n ./main -f config\n")
}

func main() {
	//parser config
	filenamePtr := flag.String("f", "", "conf file name for the server")
	flag.Parse()

	conf, err := config.LoadConfig(*filenamePtr)
	if err != nil {
		fmt.Printf("parser conf file failed, filename:%s, err:%v\n", *filenamePtr, err)
		printUsage()
		return
	}

	//init global notify map
	acceptor.InitNotifier(conf.NotifierSize, conf.NotifierWaitTime)

	host, port, err := getHostPort(conf.ServerCallbackAddr)
	if err != nil {
		fmt.Printf("parse host port:%s failed, err:%v\n", conf.ServerCallbackAddr, err)
		return
	}
	acceptor.InitHandlerParams(conf.ChunkSize, host, port)

	//init compute cells &request router
	//acceptor.ReqRouter = &acceptor.RequestRouter{ComputeCellNum: uint32(computeCellNum), ComputeCells: computeCells}
	acceptor.ReqRouter = &acceptor.RequestRouter{}

	for _, addr := range conf.ComputeCellAddrs {
		connPool := acceptor.NewConnPool(acceptor.NewThriftConn, addr, conf.ConnPool.MaxActive, conf.ConnPool.MaxIdle, conf.ConnPool.Wait)
		host, port, err := getHostPort(addr)
		if err != nil {
			fmt.Printf("parse host port:%s failed, err:%v\n", addr, err)
			return
		}
		acceptor.ReqRouter.AddComputeCell(host, port, connPool)
		//computeCells = append(computeCells, acceptor.ComputeCell{Host: host, Port: port, Pool: connPool})
	}

	//dispatch requests to different servers
	ClientReqServeMux := http.NewServeMux()
	ClientReqServeMux.HandleFunc("/upload_all", acceptor.UploadAll)
	ClientReqServeMux.HandleFunc("/upload", acceptor.UploadStream)

	ClientWsReqServeMux := http.NewServeMux()
	ClientWsReqServeMux.Handle("/ws/upload", websocket.Handler(acceptor.HandleVoidStream))

	CallbackReqServeMux := http.NewServeMux()
	CallbackReqServeMux.HandleFunc("/notify", acceptor.Notify)

	//start callback server first
	go func() {
		fmt.Printf("start acceptor callback server at [%s]!\n", conf.ServerCallbackAddr)
		log.Fatal(http.ListenAndServe(conf.ServerCallbackAddr, CallbackReqServeMux))
	}()

	//start websocket server
	go func() {
		fmt.Printf("start acceptor websocket server at [%s]!\n", conf.ServerWebSocketServeAddr)
		log.Fatal(http.ListenAndServe(conf.ServerWebSocketServeAddr, ClientWsReqServeMux))
	}()

	//start serve server last
	fmt.Printf("start acceptor serve server at [%s]!\n", conf.ServerServeAddr)
	log.Fatal(http.ListenAndServe(conf.ServerServeAddr, ClientReqServeMux))
}
