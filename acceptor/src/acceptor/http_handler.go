package acceptor

import (
	"deepscore"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"time"
)

//stream part begin
const CHUNK_SIZE = 4 * 1024

func waitForNotify(key string) string {
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

func sendNotify(key string, message string) bool {
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

	host := "127.0.0.1"
	var port int32 = 8081

	addr := "localhost:1463"
	rich_client, err := NewDataStreamClient(addr)
	if err != nil {
		fmt.Printf("create thrift client failed!, err:%s\n", err)
		return
	}

	defer rich_client.Transport.Close()

	var number int32
	var slice_flag deepscore.SliceFlag = deepscore.SliceFlag_START

	var session_key string

	for {
		p, err := mr.NextPart()
		if err == io.EOF {
			break
		}

		formname := p.FormName()
		filename := p.FileName()

		//form-key-value
		if filename == "" {
			buffer := make([]byte, CHUNK_SIZE)
			size, err := p.Read(buffer)
			if err == io.EOF {
				fmt.Printf("read eof on key:%s\n, break it now.", formname)
				return
			}
			if formname == "session_key" {
				session_key = string(buffer[0:size])
			}
			continue
		}

		//form-file
		for {
			buffer := make([]byte, CHUNK_SIZE)
			size, err := p.Read(buffer)
			if err == io.EOF {
				break
			}
			if session_key != "" {
				sendMessage(rich_client.Client,
					&deepscore.DataSlice{Key: session_key, Val: buffer[0:size], Number: number, Flag: slice_flag, Host: host, Port: port})
			} else {
				fmt.Println("session key is null, will not send message!")
			}
			slice_flag = deepscore.SliceFlag_MIDDLE
			number += 1
		}
	}

	//send last message
	slice_flag = deepscore.SliceFlag_FINISH
	if session_key != "" {
		sendMessage(rich_client.Client,
			&deepscore.DataSlice{Key: session_key, Val: nil, Number: number, Flag: slice_flag, Host: host, Port: port})

		result := waitForNotify(session_key)
		fmt.Fprintf(w, "result is:%s \n", result)
	} else {
		fmt.Println("session key is null, will not send message!")
	}
}

func Notify(w http.ResponseWriter, r *http.Request) {
	fmt.Println("receive message!")
	var session_key string
	var message string
	/*
		if r.Method == "POST" {
			session_key = r.PostFormValue("session_key")
			message = r.PostFormValue("message")
		}*/
	if r.Method == "GET" {
		session_keys := r.URL.Query()["session_key"]
		if len(session_keys) != 0 {
			session_key = session_keys[0]
		}
		messages := r.URL.Query()["message"]
		if len(messages) != 0 {
			message = messages[0]
			fmt.Printf("session_key:%s, message:%s", session_key, message)
		}
	}

	if session_key == "" || message == "" {
		fmt.Printf("receive message session_key:%s or message:%s part is empty!\n", session_key, message)
		return
	}

	result := sendNotify(session_key, message)
	if result == false {
		fmt.Fprintf(w, "process message failed for key:%s!\n", session_key)
		return
	}
	fmt.Fprintf(w, "process message ok for key:%s!\n", session_key)
}

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
