#######################################################
## Detail: if문, for문, 사용자 정의 함수
## Date: 2015/11/18
#######################################################

# 1. if문
# (1) if (cond..) { T1 }
# (2) if (cond..) { T1 } else { T2 }
# (3) if (cond..) { T1 } else if { T2 } else { T3 }
x=16
if (x>10) {
  print("Large number!!!\n")
} else if (x>5) {
  print("middle number!!!\n")
} else {
  print("Small number!!!\n")
}

# 벡터에서는 if 문 첫번째 만 실행되고 그 뒤로는 실행 안됨
z = c(200, 100, 50)
if (z>100) {
  print("Large number!!!\n")
} else if (z>50) {
  print("middle number!!!\n")
} else {
  print("Small number!!!\n")
}

# ifelse(cond, 참, 거짓)
gz = ifelse(z < 100, 1, 2)
gz

# 50 < x        => 1
# 50 <= x < 100 => 2
# 100 <= x      => 3
y = c(17, 121, 58, 83)
gy = ifelse(y < 50, 1, ifelse(y < 100, 2, 3))
gy

gy = factor(gy)
gy

levels(gy) = c("하", "중", "상")
gy

# 테이터$변수명 = ifelse()
survey$gage = ifelse(survey$age <= 20, 1, 2)
survey

survey$gage # <- 단순한 숫자배열
survey$gage = factor(survey$gage) # <- 이제는 집단별 벡터 데이터
survey$gage
levels(survey$gage) = c("어른", "어린이")
survey

# 2. for(cond...) { T1 }
for (i in 1:10) {
  print("Hello World!!!")  
}
rep(print("Hello World!!!"), times=10)
rep(print("Hello World!!!"), each=10)

# 3. cat
?cat
cat("Hello!!\n", "World", 1, c(1,2,3), survey$gage)
cat(survey$gage, labels="A")
print(survey$gage)
for (i in 1:10) {
  cat("Hello World!!!", i, "\n")  
}

# 구구단
for (x in 2:9) {
  for (y in 1:9) {
    cat(x, "*", y, "=", x*y, "\n")
  }
  cat("\n")
}

# 사용자 함수
# 함수명 = function() { T1 }
double_func = function(x) {
  if (!is.numeric(x)) {
    print("꼭 숫자!!!")
    return(NULL)
  }
  return(2*x)
}
#double_func(3)
double_func(letters)

x=3
mode(3) == "numeric"

is.nan(1/0)
