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

	//init global notify map
	acceptor.KEY_CHANNEL_MAPPER = make(map[string]chan string)
	acceptor.KEY_CHANNEL_MUTEX = &sync.Mutex{}

	config.LoadConfig()

	http.HandleFunc("/upload_all", acceptor.UploadAll)
	http.HandleFunc("/upload", acceptor.UploadStream)
	http.HandleFunc("/notify", acceptor.Notify)

	fmt.Printf("start acceptor at 127.0.0.1:8081!\n")
	log.Fatal(http.ListenAndServe(":8081", nil))
}
