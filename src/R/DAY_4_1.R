#######################################################
## Detail: 양적 자료의 분석(기초)
## Date: 2015/11/19
#######################################################

#######################################################
#######################################################
# 양적 자료 유용한 패키지
# psych::describe(), describeBy()
#######################################################
#######################################################
install.packages("psych")
require(psych)
describe(diamonds$price)
describeBy(diamonds$price, diamonds$cut)


# 2. 양적 자료(1개) 분석
# (1) 표(구간의 빈도, 백분율)
# (2) 그래프: 히스토그램, 상자그림
# (3) 기술통계량 = 요약통계량
#
#    i. 중심 = 대표값
#   ii. 퍼짐 = 산포값
#  iii. 분포의 모양

# 예제 데이터 : diamons
require(ggplot2)

str(diamonds)

# 7개의 양적 자료 중에서 price
# 1. 빈도
diamonds$price
table(diamonds$price)
head(sort(table(diamonds$price), decreasing=TRUE))
max(diamonds$price)
min(diamonds$price)

# 구간을 만들어 보자
# cut(데이터명$양적자료 변수명, 
#     breaks=c(구간정보)
# )
cut(diamonds$price, breaks=c(0, 5000, 10000, 15000, 20000))[0:10]
# [1] (0,5e+03] (0,5e+03] (0,5e+03] (0,5e+03] (0,5e+03] (0,5e+03] (0,5e+03] (0,5e+03] (0,5e+03] (0,5e+03]
# '(' 의미 - 포함 X
# '[' 의미 - 포함 O
# (0,5e+03] ==> 0 ~ 5000 : 첫번째 구간
diamonds$price[9999]


diamonds$gprice = cut(diamonds$price, breaks=c(0, 5000, 10000, 15000, 20000))
levels(diamonds$gprice) = c("0~5000", "5000~10000", "10000~15000", "15000~20000")
table(diamonds$gprice)

# 2. 백분율
round(prop.table(table(diamonds$gprice))*100, digits=1)

#############################################################################
# 문제 gprice2 새로운 변수를 만들고 빈도와 백분율을 구하시오.
# 구간
# 0 ~ 1000
# 1000 ~ 2000
# ....
# 18000 ~ 19000
diamonds$gprice2 = cut(diamonds$price, breaks=seq(from=0, to=19000, by=1000))
levels(diamonds$gprice2) = seq(from=0, to=19000, by=1000)
# 빈도
table(diamonds$gprice2)
# 백분율
round(prop.table(table(diamonds$gprice2))*100, digits=1)



# 3. 히스토그램
# 각 구간의 현황, 가장 많은 곳, 가장 적은 곳, 중심, 대칭여부, 이상치(outlier) 유무
# hist(데이터명$변수명)
hist(diamonds$price)
hist(diamonds$price, breaks=c(0, 5000, 10000, 15000, 20000))
hist(diamonds$price, breaks=10)

mean(diamonds$price)

# install.packages("googleVis")
# require(googleVis)
# demo(googleVis)
# demo(WorldBank)
# return

# 4. 상자그림(Boxplot) : 이상치 유무 파악
# boxplot(데이터명$변수명)
boxplot(diamonds$price)
boxplot(diamonds$clarity)
boxplot(diamonds$x)

money = c(100, 30, 100, 50, 60, 10, 60, 600)
mean(money)
boxplot(money)
boxplot(money, range=3)
?boxplot

#       양적자료와       질적자료 와의 상관관계
boxplot(diamonds$price ~ diamonds$cut)


# 5. 기술통계량
# representation value
# (1) 중심 = 대표값 : 평균(데이터 값의 반), 
#                     중위수(데이터 면적의 반반) - 그래프 적분의 1/2, 
#                     최빈수(빈도가 가장 많은)
#
# 평균: mean
mean(diamonds$price)
boxplot(diamonds$price)
mean(diamonds$price, trim=0.05) # 최대/최소 5% 절삭하고 평균
median(diamonds$price) # 중위수
which.max(table(diamonds$price))

# (2) 퍼짐 = 산포 : 범위, 분산, 표준편차
sd(diamonds$price) # 표준편차

# tips
summary(diamonds$price)
summary(diamonds)
