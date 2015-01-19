package acceptor

import (
	"sync"
)

//var KeyChannelMapper = make(map[string]chan string)
//var KeyChannelMutex = &sync.Mutex{}
var KeyChannelMapper map[string]chan string
var KeyChannelMutex *sync.Mutex

func AddKeyChanToMap(key string, receiver chan string) bool {
	KeyChannelMutex.Lock()
	defer KeyChannelMutex.Unlock()
	if _, exist := KeyChannelMapper[key]; exist {
		return false
	}
	KeyChannelMapper[key] = receiver
	return true
}

func DelKeyChanFromMap(key string) {
	KeyChannelMutex.Lock()
	defer KeyChannelMutex.Unlock()
	delete(KeyChannelMapper, key)
}

func GetKeyChanFromMap(key string) (chan string, bool) {
	KeyChannelMutex.Lock()
	defer KeyChannelMutex.Unlock()
	receiver, exist := KeyChannelMapper[key]
	return receiver, exist
}
