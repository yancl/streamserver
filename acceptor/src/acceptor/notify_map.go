package acceptor

import (
	"sync"
)

type ConcurrentMap struct {
	sync.Mutex
	m map[string]chan string
}

var GlobalMap *ConcurrentMap

func InitGlobalMap(mapSize int) {
	GlobalMap = &ConcurrentMap{m: make(map[string]chan string, mapSize)}
}

func (cm *ConcurrentMap) Add(key string, receiver chan string) bool {
	cm.Lock()
	defer cm.Unlock()
	if _, exist := cm.m[key]; exist {
		return false
	}
	cm.m[key] = receiver
	return true
}

func (cm *ConcurrentMap) Delete(key string) {
	cm.Lock()
	defer cm.Unlock()
	delete(cm.m, key)
}

func (cm *ConcurrentMap) Get(key string) (chan string, bool) {
	cm.Lock()
	defer cm.Unlock()
	receiver, exist := cm.m[key]
	return receiver, exist
}
