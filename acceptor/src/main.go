package main

import (
	"config"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"sync"
	"time"
	"deep_score_service"
	"acceptor"
)

// 1MB, more data will write to disk file tempory
const MAX_MEMORY = 1 * 1024 * 1024

func Upload(w http.ResponseWriter, r *http.Request) {
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

func sendMessage(client *deep_score_service.DeepScorerServiceClient, message *deep_score_service.DataEntry) error {
  //compose messages
	capacity := 1 
	messages := make([]*deep_score_service.DataEntry, 0, capacity)
  messages = append(messages, message)

	//send messages
	r, err := client.AddDataEntryStream(messages)
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

  addr := "localhost:1463"
  rich_client, err := acceptor.NewDataStreamClient(addr)
  if err != nil {
    fmt.Printf("create thrift client failed!, err:%s\n", err)
    return
  }

  //release connection
  defer rich_client.Transport.Close()

  var number int32
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
      sendMessage(rich_client.Client, &deep_score_service.DataEntry{Key: key, Val: buffer[0:size], Number: number, Last: false})
      number += 1
		}
	}

  //send last message
  sendMessage(rich_client.Client, &deep_score_service.DataEntry{Key: key, Val: nil, Number: number, Last: true})

	result := WaitForNotify(key)
	fmt.Fprintf(w, "result is:%s \n", result)
}

func NotifyStream(w http.ResponseWriter, r *http.Request) {
	key := "key"
	message := "OK!"
	result := SendNotify(key, message)
	if result == false {
		fmt.Fprintf(w, "send message failed for key:%s!\n", key)
		return
	}
	fmt.Fprintf(w, "send message ok for key:%s!\n", key)
}

func main() {
	config.LoadConfig()

	http.HandleFunc("/upload", UploadStream)
	http.HandleFunc("/notify", NotifyStream)
	log.Fatal(http.ListenAndServe(":8081", nil))
}
