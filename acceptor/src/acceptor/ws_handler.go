package acceptor

import (
	"deepscore"
	"fmt"
	"golang.org/x/net/websocket"
)

func sendResponse(ws *websocket.Conn, code int, msg string, data string) {
	var rsp = Response{Code: code, Msg: msg, Data: []byte(data)}
	if err := websocket.JSON.Send(ws, rsp); err != nil {
		fmt.Printf("send response failed, err:%v\n", err)
		return
	}
}

func HandleVoiceStream(ws *websocket.Conn) {
	//auth
	code := 0
	msg := ""
	data := ""
	failed := false

	var sessionKey string
	var seq int32 = 0
	var flag = deepscore.SliceFlag_BROKEN

	authReq := AuthRequest{}
	if err := websocket.JSON.Receive(ws, &authReq); err != nil {
		failed, code, msg = true, -1, "get auth data failed"
		goto end
	} else {
		fmt.Printf("auth user with userid:%s, passwd:%s\n", authReq.UserId, authReq.Passwd)
	}

	sessionKey = authReq.UserId
	if err := RegisterNotify(sessionKey); err != nil {
		fmt.Printf("register notify failed, err:%v\n", err)
		failed, code, msg = true, -1, "register key failed"
		goto end
	}

	defer UnRegisterNotify(sessionKey)

	//receive data
	for {
		req := DataRequest{Flag: -1}
		if err := websocket.JSON.Receive(ws, &req); err != nil {
			fmt.Printf("receive data slice from wsocket failed, err:%v\n", err)
			failed, code, msg = true, -2, fmt.Sprintf("receive data slice from wsocket failed, err:%v\n", err)
			goto end
		}

		if req.Flag == Start {
			flag = deepscore.SliceFlag_START
		} else if req.Flag == Middle {
			flag = deepscore.SliceFlag_MIDDLE
		} else if req.Flag == Finish {
			flag = deepscore.SliceFlag_FINISH
		}

		err := SendMessage(sessionKey, &Message{seq: seq, flag: flag, data: req.Data})
		if err != nil {
			fmt.Printf("send msg failed for key:%s, err:%s\n", sessionKey, err)
			failed, code, msg = true, -3, fmt.Sprintf("send msg failed for key:%s", sessionKey)
			goto end
		}
		seq += 1

		if req.Flag == Finish || req.Flag == Broken {
			//fmt.Printf("meet end message\n")
			break
		}
	}

end:
	if failed {
		sendResponse(ws, code, msg, data)
		return
	}

	result, err := WaitForNotify(sessionKey)
	if err != nil {
		fmt.Printf("wait for notify failed,err:%v\n", err)
		sendResponse(ws, -4, "wait for response timeout", data)
	}
	sendResponse(ws, code, msg, result)
}
