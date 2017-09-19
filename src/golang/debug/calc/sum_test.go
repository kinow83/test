package calc
import "testing"

func TestSum(t *testing.T) {

	r := Sum(1, 2)
	if r != 31 {
		t.Error("결과값이 3이 아닙니다. r=", r)
	}
}
