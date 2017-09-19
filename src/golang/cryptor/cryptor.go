package main
import (
	"crypto/sha512"
	"fmt"
	"crypto/aes"
	"crypto/cipher"
	"io"
	"crypto/rand"
	"crypto/sha1"
)

func Sha512() {
	s := "유넷시스템 개발4팀"
	s1 := sha512.Sum512([]byte(s))
	fmt.Printf("%x\n", s1)

	hash := sha512.New()
	hash.Write([]byte("유넷시스템 "))
	hash.Write([]byte("개발4팀"))
	s2 := hash.Sum(nil)
	fmt.Printf("%x\n", s2)
}

func Aes() {
	key := "kakakinowunet1004"
	s := "1234567890123456"

	block, err := aes.NewCipher([]byte(key)[:16])
	if err != nil {
		fmt.Println(err)
		return
	}

	ciphertext := make([]byte, len(s))
	block.Encrypt(ciphertext, []byte(s))
	fmt.Printf("%x\n", ciphertext)

	plaintext := make([]byte, len(s))
	block.Decrypt(plaintext, ciphertext)
	fmt.Println("[",string(plaintext),"]")
}

//////////////////////////////
//
//////////////////////////////
type AesCBC struct {
	block cipher.Block
}
func NewAesCBC(keystr string) (*AesCBC, error) {
	b, err := aes.NewCipher(Getkey128(keystr));
	if err != nil {
		return nil, err
	}
	a := &AesCBC{b};
	return a, nil
}
func (a *AesCBC)Encrypt(plaintext []byte) []byte {
	if mod := len(plaintext) % aes.BlockSize; mod != 0 {
		padding := make([]byte, aes.BlockSize-mod)
		plaintext = append(plaintext, padding...)
	}
	ciphertext := make([]byte, len(plaintext)+aes.BlockSize)
	iv := ciphertext[:aes.BlockSize]
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		fmt.Println(err)
		return nil
	}

	mode := cipher.NewCBCEncrypter(a.block, iv)

	mode.CryptBlocks(ciphertext[aes.BlockSize:], plaintext)

	return ciphertext
}
func (a *AesCBC)Decrypt(ciphertext []byte) []byte {
	if len(ciphertext) % aes.BlockSize != 0 {
		fmt.Println("암호화된 데이터의 길이는 블록 크기의 배수가 되어야 합니다.")
		return nil
	}

	iv := ciphertext[:aes.BlockSize]
	ciphertext = ciphertext[aes.BlockSize:]

	plaintext := make([]byte, len(ciphertext))
	mode := cipher.NewCBCDecrypter(a.block, iv)

	mode.CryptBlocks(plaintext, ciphertext)

	return plaintext
}
func Getkey128(s string) []byte {
	hash := sha1.New()
	hash.Write([]byte(s))
	return hash.Sum(nil)[:16]
}


func main() {

	// Sha512()

	// Aes()

	ctx, err := NewAesCBC("kaka")
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(ctx)
	ciphertext := ctx.Encrypt([]byte("강석주 완전 굿"))
	fmt.Println("AES CBC 암호화 : ", ciphertext)
	plaintext := ctx.Decrypt(ciphertext)
	fmt.Println("AES CBC 복호화 : ", string(plaintext))


}
