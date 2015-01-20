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

var chunkSize int
var (
	host string
	port int32
)

func InitHandlerParams(chunkSize int, host string, port int32) {
	chunkSize = chunkSize
	host = host
	port = port
}

func sendMessage(sessionKey string, message *deepscore.DataSlice) error {
	//compose messages
	length := 0
	capacity := 1
	messages := make([]*deepscore.DataSlice, length, capacity)
	messages = append(messages, message)
	return ReqRouter.SendMessage(sessionKey, messages)
}

func UploadStream(w http.ResponseWriter, r *http.Request) {
	start := time.Now()

	mr, err := r.MultipartReader()
	if err != nil {
		fmt.Printf("make multipart reader failed!\n")
		return
	}

	var number int32
	var slice_flag deepscore.SliceFlag = deepscore.SliceFlag_START

	var sessionKey string

	for {
		p, err := mr.NextPart()
		if err == io.EOF {
			break
		}

		formname := p.FormName()
		filename := p.FileName()

		//form-key-value
		if filename == "" {
			buffer := make([]byte, chunkSize)
			size, err := p.Read(buffer)
			if err == io.EOF {
				fmt.Printf("read eof on key:%s\n, break it now.", formname)
				return
			}
			if formname == "session_key" {
				sessionKey = string(buffer[0:size])
			}
			continue
		}

		//form-file
		for {
			buffer := make([]byte, chunkSize)
			size, err := p.Read(buffer)
			if err == io.EOF {
				break
			}
			if sessionKey != "" {
				err := sendMessage(sessionKey,
					&deepscore.DataSlice{Key: sessionKey, Val: buffer[0:size], Number: number, Flag: slice_flag, Host: host, Port: port})
				if err != nil {
					fmt.Printf("send message failed for key:%s, err:%v\n", sessionKey, err)
					return
				}
			} else {
				fmt.Println("session key is EMPTY, will not send message!")
			}
			slice_flag = deepscore.SliceFlag_MIDDLE
			number += 1
		}
	}

	if sessionKey == "" {
		fmt.Println("session key is null, will not send message!")
		return
	}

	//send last message
	slice_flag = deepscore.SliceFlag_FINISH
	err = sendMessage(sessionKey,
		&deepscore.DataSlice{Key: sessionKey, Val: nil, Number: number, Flag: slice_flag, Host: host, Port: port})
	sendOverEnd := time.Now()
	fmt.Printf("send message cost time:%v\n", sendOverEnd.Sub(start))

	result, err := WaitForNotify(sessionKey)
	responseEnd := time.Now()
	fmt.Printf("total time include response is:%v\n", responseEnd.Sub(start))
	if err != nil {
		fmt.Fprintf(w, "%s\n", result)
	} else {
		fmt.Fprintf(w, "{}")
		fmt.Println("%v", err)
	}
}

func Notify(w http.ResponseWriter, r *http.Request) {
	var (
		sessionKey string
		message    string
	)

	if r.Method == "POST" {
		sessionKey = r.PostFormValue("session_key")
		message = r.PostFormValue("message")
	} else {
		errMsg := fmt.Sprintf("request method is not POST, it is:%s", r.Method)
		fmt.Fprintln(w, errMsg)
		fmt.Println(errMsg)
		return
	}

	fmt.Printf("receive notify from key:%s!", sessionKey)

	/*
		if r.Method == "GET" {
			sessionKeys := r.URL.Query()["session_key"]
			if len(sessionKeys) != 0 {
				sessionKey = sessionKeys[0]
			}
			messages := r.URL.Query()["message"]
			if len(messages) != 0 {
				message = messages[0]
				fmt.Printf("sessionKey:%s, message:%s", sessionKey, message)
			}
		}*/

	if sessionKey == "" || message == "" {
		errMsg := fmt.Sprintf("receive message sessionKey:%s or message:%s part is empty!", sessionKey, message)
		fmt.Fprintln(w, errMsg)
		fmt.Println(errMsg)
		return
	}

	err := SendNotify(sessionKey, message)
	if err != nil {
		errMsg := fmt.Sprintf("process message failed for key:%s!, err:%v", sessionKey, err)
		fmt.Fprintln(w, errMsg)
		fmt.Println(errMsg)
		return
	}
	errMsg := fmt.Sprintf("process message ok for key:%s!\n", sessionKey)
	fmt.Fprintln(w, errMsg)
	fmt.Println(errMsg)
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
			path := fmt.Sprintf("%s", fileHeader.Filename)
			buf, _ := ioutil.ReadAll(file)
			ioutil.WriteFile(path, buf, os.ModePerm)
		}
	}
}
