package main

import (
	"acceptor"
	"config"
	"flag"
	"fmt"
	"log"
	"net/http"
	"strconv"
	"strings"
)

func main() {
	filenamePtr := flag.String("f", "", "conf file name for the server")
	flag.Parse()

	conf, err := config.LoadConfig(*filenamePtr)
	if err != nil {
		fmt.Printf("parser conf file failed, filename:%s, err:%v\n", *filenamePtr, err)
		return
	}

	//init global notify map
	acceptor.InitGlobalMap(conf.MapSize)

	var (
		computeCellAddrs = conf.ComputeCellAddrs
		computeCellNum   = len(computeCellAddrs)
	)

	computeCells := make([]acceptor.ComputeCell, 0, computeCellNum)
	for _, addr := range computeCellAddrs {
		connPool := acceptor.NewConnPool(acceptor.NewThriftConn, addr, conf.ConnPool.MaxActive, conf.ConnPool.MaxIdle, conf.ConnPool.Wait)
		hostport := strings.Split(addr, ":")
		host := hostport[0]
		port, err := strconv.Atoi(hostport[1])
		if err != nil {
			fmt.Printf("parse host port:%s failed, err:%v\n", hostport, err)
			return
		}
		computeCells = append(computeCells, acceptor.ComputeCell{Host: host, Port: port, Pool: connPool})
	}

	//init request router
	acceptor.ReqRouter = &acceptor.RequestRouter{ComputeCellNum: uint32(computeCellNum), ComputeCells: computeCells}

	//dispatch requests to different servers
	ClientReqServeMux := http.NewServeMux()
	ClientReqServeMux.HandleFunc("/upload_all", acceptor.UploadAll)
	ClientReqServeMux.HandleFunc("/upload", acceptor.UploadStream)

	CallbackReqServeMux := http.NewServeMux()
	CallbackReqServeMux.HandleFunc("/notify", acceptor.Notify)

	go func() {
		fmt.Printf("start acceptor callback server at [%s]!\n", conf.ServerCallbackAddr)
		log.Fatal(http.ListenAndServe(conf.ServerCallbackAddr, CallbackReqServeMux))
	}()

	fmt.Printf("start acceptor serve server at [%s]!\n", conf.ServerServeAddr)
	log.Fatal(http.ListenAndServe(conf.ServerServeAddr, ClientReqServeMux))
}
