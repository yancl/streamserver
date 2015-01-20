package acceptor

import (
	"deepscore"
	"errors"
	"fmt"
	"hash/fnv"
)

func hash(s string) uint32 {
	h := fnv.New32a()
	h.Write([]byte(s))
	return h.Sum32()
}

type computeCell struct {
	host string
	port int32
	pool *ConnPool
}

type RequestRouter struct {
	computeCellNum uint32
	computeCells   []computeCell
}

var ReqRouter *RequestRouter

func (rr *RequestRouter) AddComputeCell(host string, port int32, pool *ConnPool) error {
	rr.computeCellNum += 1
	rr.computeCells = append(rr.computeCells, computeCell{host: host, port: port, pool: pool})
	return nil
}

func (rr *RequestRouter) SendMessage(sessionKey string, message []*deepscore.DataSlice) error {
	slotNum := hash(sessionKey) % rr.computeCellNum
	pool := rr.computeCells[slotNum].pool
	c, err := pool.Get()
	if err != nil {
		return errors.New(fmt.Sprintf("get conn from pool failed, err:%v", err))
	}
	defer pool.Put(c, false)
	rv, err := c.Client.AddDataSliceStream(message)
	if err != nil {
		fmt.Println("send message failed!, reconnect.")
		c.Reconnect()
		rv, err := c.Client.AddDataSliceStream(message)
		if err != nil {
			err = errors.New(fmt.Sprintf("retry failed...,give up now !, err:%v\n", err))
		} else {
			fmt.Println("retry succeed!, rv:%v\n", rv)
		}
		return err
	} else if rv != 0 {
		return errors.New(fmt.Sprintf("send message rv:%d for key:%s!\n", rv, sessionKey))
	}
	//fmt.Printf("send message ok, rv:%v\n", rv)
	return nil
}
