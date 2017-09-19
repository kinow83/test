package main
import (
	"net/rpc"
	"fmt"
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


func main() {
	client, err := rpc.Dial("tcp", "10.10.200.151:6000")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer client.Close()


	// 동기 호출
	args  := &Args{1, 2}
	reply := new(Reply)
	err = client.Call("Calc.Sum", args, reply)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println("동기 호출 : ", reply.C)


	// 비동기 호출
	args.A = 10
	args.B = 20
	sumCall := client.Go("Calc.Sum", args, reply, nil)
	<-sumCall.Done
	fmt.Println("비동기 호출 : ", reply.C)
}