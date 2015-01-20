package acceptor

import (
	"errors"
	"fmt"
	"sync"
	"time"
)

type concurrentMap struct {
	sync.Mutex
	m map[string]chan string
}

var notifier *concurrentMap
var responseWaitTimeout int = 30 //seconds

func InitNotifier(notifierSize int, waitTimeout int) {
	notifier = &concurrentMap{m: make(map[string]chan string, notifierSize)}
	responseWaitTimeout = waitTimeout
}

func (cm *concurrentMap) get(key string) (chan string, bool) {
	cm.Lock()
	defer cm.Unlock()
	receiver, exist := cm.m[key]
	return receiver, exist
}

func (cm *concurrentMap) add(key string, receiver chan string) bool {
	cm.Lock()
	defer cm.Unlock()
	if _, exist := cm.m[key]; exist {
		return false
	}
	cm.m[key] = receiver
	return true
}

func (cm *concurrentMap) del(key string) {
	cm.Lock()
	defer cm.Unlock()
	delete(cm.m, key)
}

func SendNotify(key string, message string) error {
	receiver, exsit := notifier.get(key)
	if exsit {
		// in normal scene it will not block sender
		// but for robust, add it to select block
		select {
		case receiver <- message:
			return nil
		case <-time.After(time.Second * 1):
			return errors.New(fmt.Sprintf("send message to receiver [TIMEOUT] for key:%s \n", key))
		}
	}
	return errors.New(fmt.Sprintf("send message to receiver [NOT FOUND] for key:%s \n", key))
}

func WaitForNotify(key string) (string, error) {
	receiver := make(chan string, 1)
	if notifier.add(key, receiver) == false {
		return "", errors.New(fmt.Sprintf("key already in map, key:%s \n", key))
	}

	//release resource
	defer notifier.del(key)

	select {
	case msg := <-receiver:
		return msg, nil
	case <-time.After(time.Second * time.Duration(responseWaitTimeout)):
		return "", errors.New(fmt.Sprintf("wait respose timeout:(%d), key:%s \n", responseWaitTimeout, key))
	}
}
