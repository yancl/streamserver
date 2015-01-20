package acceptor

import (
	"deepscore"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"time"
)

var (
	processChunkSize int
)

func InitHttpHandlerParams(chunkSize int) {
	processChunkSize = chunkSize
}

func UploadStream(w http.ResponseWriter, r *http.Request) {
	start := time.Now()
	var sendOverEnd time.Time
	var responseEnd time.Time

	rsp := Response{Code: 0, Msg: ""}
	var number int32
	var slice_flag deepscore.SliceFlag = deepscore.SliceFlag_START
	var sessionKey string
	var result string
	var err error

	mr, err := r.MultipartReader()
	if err != nil {
		rsp.Code, rsp.Msg = -1, fmt.Sprintf("make multipart reader failed,err:%v!", err)
		goto end
	}

	for {
		p, err := mr.NextPart()
		if err == io.EOF {
			break
		}

		formName := p.FormName()
		fileName := p.FileName()

		//form-key-value
		if fileName == "" {
			buffer := make([]byte, processChunkSize)
			size, err := p.Read(buffer)
			if err == io.EOF {
				rsp.Code, rsp.Msg = -1, fmt.Sprintf("read eof on key:%s\n, break it now.", formName)
				goto end
			}
			if formName == "session_key" {
				sessionKey = string(buffer[0:size])
				//register message
				if err := RegisterNotify(sessionKey); err != nil {
					rsp.Code, rsp.Msg = -1, fmt.Sprintf("register notify failed, err:%v\n", err)
					goto end
				}
				defer UnRegisterNotify(sessionKey)
			}
			continue
		}

		//form-file
		for {
			buffer := make([]byte, processChunkSize)
			size, err := p.Read(buffer)
			if err == io.EOF {
				break
			}
			if sessionKey != "" {
				err := SendMessage(sessionKey, &Message{seq: number, flag: slice_flag, data: buffer[0:size]})
				if err != nil {
					rsp.Code, rsp.Msg = -1, fmt.Sprintf("send message failed for key:%s, err:%v\n", sessionKey, err)
					goto end
				}
			} else {
				rsp.Code, rsp.Msg = -1, fmt.Sprintf("session key is EMPTY, will not send message!")
				goto end
			}
			slice_flag = deepscore.SliceFlag_MIDDLE
			number += 1
		}
	}

	if sessionKey == "" {
		rsp.Code, rsp.Msg = -1, fmt.Sprintf("session key is EMPTY, will not send message!")
		goto end
	}

	//send last message
	slice_flag = deepscore.SliceFlag_FINISH
	if err = SendMessage(sessionKey, &Message{seq: number, flag: slice_flag, data: nil}); err != nil {
		rsp.Code, rsp.Msg = -1, fmt.Sprintf("send message failed for key:%s, err:%v\n", sessionKey, err)
		goto end
	}
	sendOverEnd = time.Now()
	fmt.Printf("send message cost time:%v\n", sendOverEnd.Sub(start))

	if result, err = WaitForNotify(sessionKey); err != nil {
		rsp.Code, rsp.Msg = -1, fmt.Sprintf("wait message failed for key:%s, err:%v\n", sessionKey, err)
		goto end
	}
	rsp.Data = []byte(result)

end:
	var data []byte
	data, err = json.Marshal(rsp)
	if err != nil {
		data = []byte("{code:-100}")
		fmt.Printf("xxx")
	}
	fmt.Fprintf(w, string(data))
	fmt.Println(string(data))
	responseEnd = time.Now()
	fmt.Printf("total time include response is:%v\n", responseEnd.Sub(start))
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

	fmt.Printf("receive notify from key:%s!\n", sessionKey)

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
		errMsg := fmt.Sprintf("process message failed for key:%s!, \nerr:%v", sessionKey, err)
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
