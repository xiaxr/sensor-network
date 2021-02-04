package systemd

import (
	"github.com/coreos/go-systemd/dbus"
	"context"
)

func NewConnection() (dbus.Conn*){
	conn, err := dbus.NewConnection(context.TODO)
	if err!= nil{
		panic("cannot connect to dbus")
	}
}