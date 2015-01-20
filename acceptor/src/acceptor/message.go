package acceptor

const (
	Start  = 0
	Middle = 1
	Finish = 2
	Broken = 10
)

type AuthRequest struct {
	UserId string `json:"userid"`
	Passwd string `json:"passwd"`
}

type DataRequest struct {
	Flag int    `json:"flag"`
	Data []byte `json."data"`
}

type Response struct {
	Code int    `json:"code"`
	Msg  string `json:"msg"`
	Data []byte `json:"data"`
}
