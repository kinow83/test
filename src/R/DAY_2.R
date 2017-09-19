#######################################################
## Detail: base of R
## Date: 2015/11/17
#######################################################

# 3. Matrix

# 1.
# rbind(vector1, vector2, ...)
# cbind(vector1, vector2, ...)
v1 <- 1:3
v2 <- 4:6
m1 <- rbind(v1, v2)
m1
m2 <- cbind(v1, v2)
m2

(m2 <- cbind(v1, v2))

v3 = 1:6
(m3 = rbind(v1, v2, v3)) # <- Recycling Rule

letters
LETTERS

v4 = letters[1:3]
(m4 = cbind(v1, v2, v4)) # <- cast by character


# 2. matrix(vector, nrow=, ncol=, byrow=TRUE)
matrix(1:4, nrow=1)
matrix(1:4, nrow=2)
matrix(1:4, nrow=2, byrow=TRUE)
matrix(1:4, ncol=2)
matrix(1:4, ncol=2, byrow=TRUE)
matrix(1:6, nrow=3, ncol=2)
matrix(1:6, nrow=3, ncol=4) # <- Recycling Rule

matrix(c('a',1,'c'))

mode(matrix(1:6))
mode(m4)
m1
length(m1)
m4
length(m4)


m1 = matrix(c(7,6,4,31,25,19), nrow=2, ncol=3)
m1
m1 = matrix(c(7,6,4,31,25,19), nrow=2, ncol=3, byrow=TRUE)
m1
m1[1,]
m1[,1]
m1[2,3]
m1[,c(1,3)]
m1[,-2]
m1[c(1,2), c(1,1)]
m1[1:2, 1:2]
m1[1:2, 2:3]
m1[1:1, 2:3, drop=FALSE]

# 3.
# 하나의 행이나 하나의 열만 가져오면
# 그 결과는 벡터가 됨
#
# 하나의 행(또는 열)만 가져와도
# 행렬은 유지되도록 하고 싶을 때
# 행렬명[행, 열, drop=FALSE]
m1[1, ,drop=FALSE]
m1[1, ]

# 4. array
# expasion of matrix
# array(data, dim=2)          
# array(data, dim=c(2,3,2))   3 차원
# array(data, dim=c(2,3,2,4)) n 차원
?array
array(1:12)
array(1:12, dim=5)
array(1:12, dim=c(2,3))
array(1:12, dim=c(2,3,2))
array(c('a',1), dim=c(2,3,2))
array(c('a',1,2), dim=c(2,3,2))
array(c('a',1), dim=c(2,3,2,2))
?array

# 5. Data Frame
# Popular Data 형태
# 행렬은 하나의 데이터 유형만 갖지만
# 데이터 프레임은 다양한 데이터 유형을 가짐
# data.frame(vector, matrix, ...)
(id = 1:3)
(bt = c("O", "O", "B"))
(money = c(30, 100, 50))

(d1 = data.frame(id, bt, money))
mean(d1$money)
mean(d1$id)

# 6. Factor
# 어떤 자료를 
# 범주형 자료 = 질적 자료 = 그룹(집단)
bt

(f_bt = factor(bt))
f_bt[1]
f_bt[2]
f_bt[3]
f_bt[[3]]
# 7. List
# 모든 데이터 형태를 데이터의 일부로 가질 수 있음
# 자기 자신 마저도...
# 통계분석의 결과를 저장하는 경우 많이 사용
# list(Scalar, Vector, Matrix, Array, DataFrame, Factor, List, ...)
x  = 1
v = 1:10
m = matrix(1:4, nrow=2)
d = d1

list2 = list(x, v, m, d, f_bt)
list2

# 리스트에서 일부를 가져오기
# 리스트명[index]
# 리스트명[[index]]
list2[2]
list2[[2]]

rm(list=ls())
(list2 = list(x, v, m, d, f_bt, list2))

list2[6][[1]]
