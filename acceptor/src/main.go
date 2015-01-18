package main

import (
	"acceptor"
	"config"
	"fmt"
	"log"
	"net/http"
	"sync"
)

func main() {
	log.Printf("begin to run!\n")
	config.LoadConfig()

	//init global notify map
	acceptor.KEY_CHANNEL_MAPPER = make(map[string]chan string)
	acceptor.KEY_CHANNEL_MUTEX = &sync.Mutex{}

	//init request router
	computeCells := make([]acceptor.ComputeCell, 0, 2)

	//func NewConnPool(newFn func(addr string) (Conn, error), addr string, maxIdle int, maxActive int, wait bool) *ConnPool {
	connPool := acceptor.NewConnPool(acceptor.NewThriftConn, "127.0.0.1:1463", 10, 10, true)
	computeCells = append(computeCells, acceptor.ComputeCell{Host: "127.0.0.1", Port: 1463, Pool: connPool})
	connPool2 := acceptor.NewConnPool(acceptor.NewThriftConn, "127.0.0.1:1463", 10, 10, true)
	computeCells = append(computeCells, acceptor.ComputeCell{Host: "127.0.0.1", Port: 1463, Pool: connPool2})
	acceptor.ReqRouter = &acceptor.RequestRouter{ComputeCellNum: 2, ComputeCells: computeCells}

	http.HandleFunc("/upload_all", acceptor.UploadAll)
	http.HandleFunc("/upload", acceptor.UploadStream)
	http.HandleFunc("/notify", acceptor.Notify)

	fmt.Printf("start acceptor at 127.0.0.1:8081!\n")
	log.Fatal(http.ListenAndServe(":8081", nil))
}
