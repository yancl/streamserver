package config

import (
	"encoding/json"
	"os"
)

type Config struct {
	ServerServeAddr          string
	ServerWebSocketServeAddr string
	ServerCallbackAddr       string
	ChunkSize                int
	NotifierSize             int
	NotifierWaitTime         int
	ComputeCellAddrs         []string
	ConnPool                 struct {
		MaxActive int
		MaxIdle   int
		Wait      bool
	}
}

func LoadConfig(filename string) (Config, error) {
	file, _ := os.Open(filename)
	decoder := json.NewDecoder(file)
	conf := Config{}
	err := decoder.Decode(&conf)
	return conf, err
}
