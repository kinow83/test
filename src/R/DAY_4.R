#######################################################
## Detail: 질적 자료의 분석(기초)
## Date: 2015/11/19
#######################################################


#######################################################
#######################################################
# Quick-R
# www.statmethods.net
# 
# 서민구: R을 이용한 데이터 처리 & 분석 실무
#       : THE R BOOK (번역)
#######################################################
#######################################################
# 통계교육원 : 대전...
#              사이버교육
#              Coursera
#######################################################
#######################################################
# data.table 패키지 <- 데이터가 엄청 많을 때
# doBy 패키지 <- 집단 관련 
# plyr, dplyr 패키지 <- 데이터 핸들링
# googleVis 패키지
# ggmap 패키지 <- GIS 핸들링
# tm
# rpart ?? <- 집단에 대한 분류 (classfy by domain)
# httr <- about http crawlering
# rvest <- about http crawlering
#######################################################
#######################################################


# 자료: 질적 자료(범주형 자료) VS 양적 자료
#
# 질적자료: 문자. 숫자가 의미를 가지지 못하는 자료
# ex> 혈액형, 인종, 국적, 직업
#
# 양적자료: 숫자, 숫자가 의미를 가짐
# ex> 몸무게, 시력, IQ, 나이


# 1. 질적자료 (1개)
#  (1) 표(빈도, 백분율)
#  (2) 그래프: 막대그래프, 원그래프
# ***********< 가장중요한 요소는 DATA의 이해>*****************
# 
# 예제 데이터: ggplot2 패키지 (그래프 작성시 유용함.)의 diamonds 자료 이용
require(ggplot2)

str(diamonds)
# diamons의 질적자료(변수): cut, color, clarity
#
# $ cut    : Ord.factor w/ 5 levels "Fair"<"Good"<..: 5 4 2 4 2 3 3 3 1 3 ...
# $ color  : Ord.factor w/ 7 levels "D"<"E"<"F"<"G"<..: 2 2 2 6 7 7 6 5 2 5 ...
# $ clarity: Ord.factor w/ 8 levels "I1"<"SI2"<"SI1"<..: 2 3 5 4 2 6 7 3 4 5 ...
#
# 1-1.빈도(frequency)
# table(데이터명$변수명)
str(diamonds$cut)
table(diamonds$cut)
table(diamonds$cut[1:length(diamonds$cut)])

table(diamonds$color)
table(diamonds$clarity)

# 1-2. 백분율(percent)
# prop.table( table(데이터명$변수명) )
p1 = prop.table(table(diamonds$cut)) * 100
p2 = prop.table(table(diamonds$color)) * 100
p3 = prop.table(table(diamonds$clarity)) * 100
order(p1)
levels(p1) = levels(diamonds$cut)
levels(p1)
p1

# 백분율의 소수점 자리수
# 특별한 얘기가 없으면 소수점 첫째짜리까지 표현함
# round(숫자, disits=1) <- 1=소수점 자릿수
round(p1, digits=2)
round(p1, digits=1)
round(p1, digits=0)
round(p1, digits=-1)

round(p1, digits=2)
round(p2, digits=2)
round(p3, digits=2)

p2[order(p2)]
mode(p2)
sort(round(p1, digits=2), decreasing=TRUE)


# 문제: 질적 자료가 100개 라면
# 코딩의 라인이 약 4개 정도면 빈도와 백분율 구함
diamonds[,2] # cut
diamonds[,3] # color
diamonds[,4] # clarity

for (i in c("cut", "color", "clarity")) {
  out = table(diamonds[, i])
  print(out)
  out = round(prop.table(table(diamonds[, i])) * 100, digits=1)
  print(out)
}


# 1-3. 막대그래프(Bar plot, Bar chart)
# barplot(빈도정보)
# barplot(table(데이터명$변수명))
# 막대 색상 변경 :col="색상"
# 치트 제목: main="제목"
# axis Y: ylab="y축제목"
# ylimi=c(최소값,최대값)
# horiz
barplot(table(diamonds$cut))
barplot(table(diamonds$color))
barplot(table(diamonds$clarity))

cs = c("red", "blue", "green", "black", "purple", "white", "yellow", "brown", "pink")
barplot(sort(table(diamonds$cut), decreasing=TRUE), col=colors()[1:20], 
        main="품질의 현황", 
        ylab="freq",
        ylim=c(0, max(table(diamonds$cut))))

barplot(sort(table(diamonds$cut), decreasing=TRUE), col=colors()[1:20], 
        main="품질의 현황", 
        xlab="freq",
        xlim=c(0, max(table(diamonds$cut))),
        horiz=TRUE)

barplot(sort(table(diamonds$color), decreasing=TRUE), col=rainbow(5))
barplot(sort(table(diamonds$clarity), decreasing=TRUE), col=c("red", "blue"))

barplot(c(1,2,3,4), c(11,12,13,14), c(21,22,23,24))

?barplot

colors()[1:20]

?sample()


# 1-4. 원그래프
# pie(빈도)
# pie(table(데이터명$변수명))
pie(table(diamonds$cut))
?pie

pdf(file="E:/kaka/R/20151119.pdf")
## 문제 : 만약에 질적자료가 100개 라면 각 자료에 대한 막대그래프와 원그래프를 작성
for (i in c("cut", "color", "clarity")) {
  barplot(table(diamonds[, i]), main=i, ylab="freq")
  pie(table(diamonds[, i]), main=i, border=NA)
}
dev.off()

n <- 200
pie(rep(1, n), labels = "", col = rainbow(n), border = NA, main = "pie(*, labels=\"\", col=rainbow(n), border=NA,..")

