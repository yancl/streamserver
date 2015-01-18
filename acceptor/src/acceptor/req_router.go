package acceptor

import (
	"deepscore"
	"fmt"
	"hash/fnv"
)

type ComputeCell struct {
	Host string
	Port uint32
	Pool *ConnPool
}

type RequestRouter struct {
	ComputeCellNum uint32
	ComputeCells   []ComputeCell
}

func hash(s string) uint32 {
	h := fnv.New32a()
	h.Write([]byte(s))
	return h.Sum32()
}

func (rr *RequestRouter) SendMessage(sessionKey string, message []*deepscore.DataSlice) error {
	slotNum := hash(sessionKey) % rr.ComputeCellNum
	pool := rr.ComputeCells[slotNum].Pool
	c, err := pool.Get()
	if err != nil {
		fmt.Printf("get conn from pool failed, err:%v", err)
		return err
	}
	defer pool.Put(c, false)
	rv, err := c.(ThriftConn).Client.AddDataSliceStream(message)
	if err != nil {
		fmt.Printf("send message failed!")
		return err
	}
	fmt.Printf("send message ok, rv:%v", rv)
	return nil
}
