package main

import(
	"github.com/xiaxr/sensor-network/hub/supervisor/pkg/systemd"
)

func main(){
	c := systemd.NewConnection()
	c.Close()
}