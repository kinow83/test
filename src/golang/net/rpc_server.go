package main
import (
	"net/rpc"
	"fmt"
	"net"
)


///////////////////////////////////////////////////////////
// Common 구조체
///////////////////////////////////////////////////////////
type Args struct {
	A, B int
}

type Reply struct {
	C int
}


///////////////////////////////////////////////////////////
// RPC 서버에 함수를 등록하려면 함수만으로는 안 되고,
// 구조체나 일반 자료형과 같은 타입에 메서드 형태로 구성되어야 함.
///////////////////////////////////////////////////////////
type Calc int

func (c *Calc) Sum(args Args, reply *Reply) error {
	reply.C = args.A + args.B
	return nil
}

func main() {
	err := rpc.Register(new(Calc))
	if err != nil {
		fmt.Println(err)
		return
	}

	listen, err := net.Listen("tcp", ":6000")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer listen.Close()

	for {
		conn, err := listen.Accept()
		if err != nil {
			fmt.Println(err)
			continue
		}
		defer conn.Close()

		go rpc.ServeConn(conn)
	}

}