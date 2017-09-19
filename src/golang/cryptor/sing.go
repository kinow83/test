package main
import (
	"crypto/rsa"
	"crypto/rand"
	"fmt"
	_"crypto/md5"
	"crypto"
	"crypto/sha1"
)



func main() {
	privateKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		fmt.Println(err)
		return
	}
	publicKey := &privateKey.PublicKey
	fmt.Println(publicKey)

	message := "(1절) 동해물과 백두산이 마르고 닳도록 하느님이 보우하사 "

//	hash := md5.New()
	hash := sha1.New()
	hash.Write([]byte(message))
	digest := hash.Sum(nil)
	fmt.Println(digest)

	var h1 crypto.Hash
	signature, err := rsa.SignPKCS1v15(
		rand.Reader,
		privateKey,
		h1,
		digest)

	var h2 crypto.Hash
	err = rsa.VerifyPKCS1v15(
		publicKey,
		h2,
		digest,
		signature)

	if err != nil {
		fmt.Println("검증 실패")
	} else {
		fmt.Println("검증 성공")
	}
}