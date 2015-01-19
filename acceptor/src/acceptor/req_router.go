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
	rv, err := c.Client.AddDataSliceStream(message)
	if err != nil {
		fmt.Println("send message failed!, reconnect.")
		c.Reconnect()
		rv, err := c.Client.AddDataSliceStream(message)
		if err != nil {
			fmt.Printf("retry failed...,give up now !, err:%v\n", err)
		} else {
			fmt.Println("retry succeed!, rv:%v\n", rv)
		}
		return err
	}
	fmt.Printf("send message ok, rv:%v\n", rv)
	return nil
}
