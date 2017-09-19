#######################################################
## Detail: 데이터 핸들리(전처리)
## Date: 2015/11/18
#######################################################

#######################################################
# R 그래픽 추천 도서
# R Graphics Cook Book
# R Cook Book
# R을 활용한 데이터 시각화 (류충현)
#######################################################

# 예제 데이터: ggplot2 패키지에 있는 diamonds 데이터 사용
install.packages("ggplot2")
require(ggplot2)

# 데이터 전체보기
# 데이터명
diamonds
mode(diamonds)
head(diamonds)
head(diamonds, n=2)
tail(diamonds)
head(diamonds["cut"])

diamonds[100:120, ]

# nrow : 행의 갯수
# ncol : 열의 갯수
nrow(diamonds)
ncol(diamonds)

# rownames: 행의 이름 나열
# colnames: 열의 이름 나열
rownames(diamonds)
colnames(diamonds)
colnames(diamonds)[8:10]
diamonds[1:10, colnames(diamonds)[8:10]]
diamonds[1:10, colnames(diamonds)[-10]]


seq(from=5,to=1,by=-0.1)
# 행 추출하기
# 1, 101, 201, 301, ....행을 가져오세요.
x=diamonds[seq(from=1, to=nrow(diamonds), by=100), ]
mode(x)
(x=diamonds[seq(from=1, to=nrow(diamonds), by=100), 1:2])
mode(x)
diamonds[seq(from=1, to=nrow(diamonds), by=100), c(1,2)]

13 %%  4 # 나머지
13 %/% 4 # 몫


# 열(변수) 추출하기
# 데이터명[, 열]
# 하나의 열만 가져오면 그 결과는 벡터
x=diamonds[,1]
mode(x)
x=diamonds[,1, drop=FALSE]
mode(x)
diamonds[1:5,7:9, drop=FALSE]

# FALSE:6, TRUE:3, FALSE:1
rep(c(FALSE, TRUE, FALSE), c(6, 3, 1))
diamonds[1:5, rep(c(FALSE, TRUE, FALSE), c(6, 3, 1))] # diamonds[1:5,7:9, drop=FALSE] 와 동일

rep(c(0,1,0), c(6,3,1))
as.logical(rep(c(0,1,0), c(6,3,1))) # 0->FALSE, 1->TRUE
diamonds[1:5, as.logical(rep(c(0,1,0), c(6,3,1)))] # diamonds[1:5,7:9, drop=FALSE] 와 동일

# str(): 구조체 정보 보기
str(diamonds)

diamonds$cut
mode(diamonds$cut)

# variable 이름으로 행렬 조회
diamonds[1:10, c("price", "x", "z")]

# A %in% B
# A는 B에 포함됩니까?
# A는 B에 존재합니까?
1 %in% 1:3
5 %in% 1:3

c("price", "x", "y", "w") %in% colnames(diamonds)
as.integer(c("price", "x", "y", "w") %in% colnames(diamonds))

# 
str(diamonds)
bt = c("B", "AB", "A")
f_bt = factor(bt)
f_bt
mode(f_bt)

##############################################
# levels : 범주를 표현하는 데이터 심볼
# levels(x)
# levels(x) <- value
# Arguments x
#           an object, for example a factor.
##############################################
levels(f_bt) = c("A형", "AB형", "B형")
levels(f_bt) = c("1", "2", "3")
f_bt
diamonds$cut
levels(diamonds$cut)

x=c(1,2,3,'A','B',c(1,2),1,1,1,1,1,1,1,1)
x
levels(factor(x))
?levels()

########################################
# 문제1> cut이 Ideal인 것만 추출
diamonds$cut == "Ideal"
diamonds[diamonds$cut == "Ideal", ]


levels(diamonds$color)
# 문제2> color가 D, E, G, H, J 색을 가져오세요
diamonds[c('D', 'E', 'G', 'H', 'J') %in% diamonds$color, ]

# 문제2> 응용
diamonds$color != 'F'
diamonds[diamonds$color != "F", ]

# 문제3> 
# cut은 Ideal, color는 F인 것을 가져오세요
# & : and 조건
diamonds$cut == "Ideal" & diamonds$color == "F"
head(diamonds[(diamonds$cut == "Ideal") & (diamonds$color == "F"), ], n=10)

# 문제4> 
# cut이 Ideal이거나 또는 color가 F인 것을 가져오세요
head(diamonds[(diamonds$cut == "Ideal") | (diamonds$color == "F"), ], n=10)

# 문제5> carat >= 0.5 and price >= 15000인 x,y,z
rowcon=(diamonds$carat >= 0.5) & (diamonds$price >= 15000)
rowcon
as.integer(rowcon)
sum(as.integer(rowcon))
colcon=c("x", "y", "z")
colcon
diamonds[rowcon, colcon]

##################################################################
# NA (Not Available)
# Missing Value
# 결측치, 결측값
# diamonds[c('D', 'E', 'G', 'H', 'J') %in% diamonds$color, ] = NA
# diamonds[c('D', 'E', 'G', 'H', 'J') %in% diamonds$color, ] = 0
(age=c(20,25,24,NA))
mean(age, na.rm=TRUE)


id     = 1:7
gender = c("m", "m", "m", "m", "m", "f", "f")
bt     = c("b", "ab", "o", "o", "o", "o", "o")
age    = c(29, 32, 18, 27, 24, 25, 20)
height = c(178, 170, 180, 180, 185, 165, 165)
survey = data.frame(id, gender, bt, age, height)

####################################################
# 정렬하기
# order(데이터명$변수명)
survey$age[order(survey$age)]
survey$age[order(survey$age, decreasing=TRUE)]

survey[order(survey$age), ]
survey[order(survey$age, decreasing=TRUE), ]

survey
# 연령과 키를 기준으로 오름차순 정렬
# '키'가 먼저, '연령'이 나중에 
ho_cond1 = order(survey$height, survey$age)
survey[ho_cond1, ]

# 연령과 키를 기준으로 내림차순 정렬
# '키'가 먼저, '연령'이 나중에 
ho_cond2 = order(survey$height, survey$age, decreasing=TRUE)
survey[ho_cond2, ]

# '키'는 오름차순, '연령'은 내림차순
ho_cond = order(survey$height, -survey$age)
survey[ho_cond, ]
# '키'는 내림차순, '연령'은 오름차순
ho_cond = order(survey$height, -survey$age, decreasing=TRUE)
survey[ho_cond, ]

# 성별로 오름차순으로 정렬하세요.
survey[order(survey$gender), ]

# 성별은 내림차순, 키는 오름차순으로 정렬하세요
#survey[order(-survey$gender, survey$height), ]
survey[order(survey$gender, -survey$height, decreasing=TRUE), ]

# 데이터 합치기
# rbind(data1, data2, ...) -> 행 방향으로 추가 (아래로)
# cbind(data1, data2, ...) -> 열 방향으로 추가 (우측으로)
# merge(data1, data2, ...)
d1 = data.frame(id=1:3, age=24:26)
d1
d2 = data.frame(id=4:5, age=30:31)
d2
d3 = rbind(d1, d2)
d3

d4 = data.frame(id=1:4, 
                gender=c("F", "M", "M", "F"))
d4
d5 = data.frame(id=c(1, 2, 7, 8, 9),
                age=c(32, 57, 23, 44, 18))
d5
# inner join
# merge(data1, data2, by=변수명)
# d4와 d5에는 PK(여기서는 id)가 있어야 함.
merge(d4, d5, by="id")

#  full join
# merge(data1, data2, by=변수명, all=TRUE)
merge(d4, d5, by="id", all=TRUE)
merge(d4, d5, by="id", all.y=TRUE, all.x=TRUE)

# left join
# merge(data1, data2, by=변수명, all.x=TRUE)
merge(d4, d5, by="id", all.x=TRUE)
merge(d4, d5, by="id", all.y=TRUE)



# TODO:
# 벡터에서 NA를 제외하고 데이터 가져오는 방법은??
# 아니면 NA를 0으로 치환하는 방법은? <- hint: is.na
age=c(20,25,24,NA)
f_age = data.frame(age)
is.na(f_age$age)
f_age[!is.na(f_age$age), ]
?is.na

!is.na(c(20,25,24,NA))
age[!is.na(c(20,25,24,NA))]
age[is.na(c(20,25,24,NA))]

# 벡터에서 NA가 포함하는 데이터의 index 어떻게 구하지????  >.<

