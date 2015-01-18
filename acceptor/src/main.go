package main

import (
	"acceptor"
	"config"
	"deepscore"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"sync"
	"time"
)

// 1MB, more data will write to disk file tempory
const MAX_MEMORY = 1 * 1024 * 1024

func UploadAll(w http.ResponseWriter, r *http.Request) {
	if err := r.ParseMultipartForm(MAX_MEMORY); err != nil {
		log.Println(err)
		http.Error(w, err.Error(), http.StatusForbidden)
	}

	for key, value := range r.MultipartForm.Value {
		fmt.Fprintf(w, "%s:%s ", key, value)
		log.Printf("%s:%s\n", key, value)
	}

	for _, fileHeaders := range r.MultipartForm.File {
		for _, fileHeader := range fileHeaders {
			file, _ := fileHeader.Open()
			path := fmt.Sprintf("downloads/%s", fileHeader.Filename)
			buf, _ := ioutil.ReadAll(file)
			ioutil.WriteFile(path, buf, os.ModePerm)
		}
	}
}

const CHUNK_SIZE = 4 * 1024

var KEY_CHANNEL_MAPPER = make(map[string]chan string)
var KEY_CHANNEL_MUTEX = &sync.Mutex{}

func AddKeyChanToMap(key string, receiver chan string) bool {
	KEY_CHANNEL_MUTEX.Lock()
	defer KEY_CHANNEL_MUTEX.Unlock()
	if _, exist := KEY_CHANNEL_MAPPER[key]; exist {
		return false
	}
	KEY_CHANNEL_MAPPER[key] = receiver
	return true
}

func DelKeyChanFromMap(key string) {
	KEY_CHANNEL_MUTEX.Lock()
	defer KEY_CHANNEL_MUTEX.Unlock()
	delete(KEY_CHANNEL_MAPPER, key)
}

func GetKeyChanFromMap(key string) (chan string, bool) {
	KEY_CHANNEL_MUTEX.Lock()
	defer KEY_CHANNEL_MUTEX.Unlock()
	receiver, exist := KEY_CHANNEL_MAPPER[key]
	return receiver, exist
}

func WaitForNotify(key string) string {
	receiver := make(chan string, 1)
	if AddKeyChanToMap(key, receiver) == false {
		return fmt.Sprintf("key already in map, key:%s", key)
	}

	//release resource
	defer DelKeyChanFromMap(key)

	var result string

	select {
	case msg := <-receiver:
		result = msg
	case <-time.After(time.Second * 3):
		result = "Timeout"
	}
	return result
}

func SendNotify(key string, message string) bool {
	receiver, exsit := GetKeyChanFromMap(key)
	if exsit {
		// in normal scene it will block sender
		// but for robust, add it to select block
		select {
		case receiver <- message:
		case <-time.After(time.Second * 1):
		}
		return true
	}
	fmt.Printf("send message failed for key:%s \n", key)
	return false
}

func sendMessage(client *deepscore.DeepScorerServiceClient, message *deepscore.DataSlice) error {
	//compose messages
	capacity := 1
	messages := make([]*deepscore.DataSlice, 0, capacity)
	messages = append(messages, message)

	//send messages
	r, err := client.AddDataSliceStream(messages)
	if err != nil {
		fmt.Printf("add messages failed,err:%s \n", err)
	} else {
		fmt.Printf("add messages ok, r:%v\n", r)
	}
	return err
}

func UploadStream(w http.ResponseWriter, r *http.Request) {

	mr, err := r.MultipartReader()
	if err != nil {
		fmt.Printf("make multipart reader failed!\n")
		return
	}

	for key, value := range r.MultipartForm.Value {
		fmt.Printf("%s:%s\n", key, value)
	}

	//"Transfer-Encoding: chunked" request doesn't have *correct* ContentLength
	//length := r.ContentLength
	//fmt.Printf("content length:%v \n", length)

	key := "yancl"

	host := "127.0.0.1"
	var port int32
	port = 8081

	addr := "localhost:1463"
	rich_client, err := acceptor.NewDataStreamClient(addr)
	if err != nil {
		fmt.Printf("create thrift client failed!, err:%s\n", err)
		return
	}

	//release connection
	defer rich_client.Transport.Close()

	var number int32
	var slice_flag deepscore.SliceFlag = deepscore.SliceFlag_START

	//send data
	for {
		part, err := mr.NextPart()
		if err == io.EOF {
			break
		}
		wf, err := os.OpenFile("downloads/test.txt", os.O_WRONLY|os.O_CREATE, 0644)
		if err != nil {
			fmt.Printf("open file failed,%s \n", err)
			return
		}

		var read int64
		for {
			buffer := make([]byte, CHUNK_SIZE)
			size, err := part.Read(buffer)
			if err == io.EOF {
				break
			}
			read = read + int64(size)
			//fmt.Printf("read: %v \n",read )
			wf.Write(buffer[0:size])
			sendMessage(rich_client.Client, &deepscore.DataSlice{Key: key, Val: buffer[0:size], Number: number, Flag: slice_flag, Host: host, Port: port})
			slice_flag = deepscore.SliceFlag_MIDDLE
			number += 1
		}
	}

	//time.Sleep(time.Second * 10)

	//send last message
	slice_flag = deepscore.SliceFlag_FINISH
	sendMessage(rich_client.Client, &deepscore.DataSlice{Key: key, Val: nil, Number: number, Flag: slice_flag, Host: host, Port: port})

	result := WaitForNotify(key)
	fmt.Fprintf(w, "result is:%s \n", result)
}

func NotifyStream(w http.ResponseWriter, r *http.Request) {
	fmt.Println("receive message!")
	key := "yancl"
	message := "OK!"
	result := SendNotify(key, message)
	if result == false {
		fmt.Fprintf(w, "send message failed for key:%s!\n", key)
		return
	}
	fmt.Fprintf(w, "send message ok for key:%s!\n", key)
}

func main() {
	log.Printf("begin to run!\n")
	config.LoadConfig()

	http.HandleFunc("/upload_all", UploadAll)
	http.HandleFunc("/upload", UploadStream)
	http.HandleFunc("/notify", NotifyStream)
	fmt.Printf("start acceptor at 127.0.0.1:8081!\n")
	log.Fatal(http.ListenAndServe(":8081", nil))
}
