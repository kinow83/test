package main
import (
	"crypto/rsa"
	"crypto/rand"
	"fmt"
)

func main() {

	privateKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		fmt.Println("1:", err)
		return
	}

	publicKey := &privateKey.PublicKey

	s := "(1절) 동해물과 백두산이 마르고 닳도록 하느님이 보우하사 "

	/*
	fmt.Println(len(s))
	fmt.Println((publicKey.N.BitLen()+7)/8)
	k := (pub.N.BitLen() + 7) / 8
	if len(msg) > k-11 {
		err = ErrMessageTooLong
		return
	}
	 */

	ciphertext, err := rsa.EncryptPKCS1v15(rand.Reader, publicKey, []byte(s))
	if err != nil {
		fmt.Println("2:", err)
		return
	}

	fmt.Printf("%x\n", ciphertext)

	plaintext, err := rsa.DecryptPKCS1v15(rand.Reader, privateKey, ciphertext)
	if err != nil {
		fmt.Println("3:", err)
		return
	}

	fmt.Println(string(plaintext))


}