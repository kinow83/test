package main
import (
	"testing"
	"calc"
)


func BenchmarkSum(b *testing.B) {
	for i:=0; i<b.N; i++ {
		calc.Sum(1, 2)
	}
}
