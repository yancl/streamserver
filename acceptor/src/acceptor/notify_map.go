package acceptor

import (
	"sync"
)

//var KEY_CHANNEL_MAPPER = make(map[string]chan string)
//var KEY_CHANNEL_MUTEX = &sync.Mutex{}
var KEY_CHANNEL_MAPPER map[string]chan string
var KEY_CHANNEL_MUTEX *sync.Mutex

func AddKeyChanToMap(key string, receiver chan string) bool {
	KEY_CHANNEL_MUTEX.Lock()
	defer KEY_CHANNEL_MUTEX.Unlock()
	if _, exist := KEY_CHANNEL_MAPPER[key]; exist {
		return false
	}
	KEY_CHANNEL_MAPPER[key] = receiver
	return true
}

func DelKeyChanFromMap(key string) {
	KEY_CHANNEL_MUTEX.Lock()
	defer KEY_CHANNEL_MUTEX.Unlock()
	delete(KEY_CHANNEL_MAPPER, key)
}

func GetKeyChanFromMap(key string) (chan string, bool) {
	KEY_CHANNEL_MUTEX.Lock()
	defer KEY_CHANNEL_MUTEX.Unlock()
	receiver, exist := KEY_CHANNEL_MAPPER[key]
	return receiver, exist
}
