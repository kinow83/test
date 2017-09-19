#######################################################
## Detail: 
## Date: 2015/11/23
#######################################################

## 회귀분석 이해
x=1:100
y=2*x+30
y[50]
y[50]=150
cor(x,y) # <- 상관관계
?cor 
d=cbind(x,y)
plot(d)
lm(y~x)
x1=jitter(x)
y1=jitter(y)
plot(x1)
plot(y1)
lm(y~x) # <- 회귀분석: 에러율 최소로 하는 선형화
lm(y1~x1)
d1=cbind(x1,y1)
plot(d1)


#######################################################
# y          x
# 종속변인 ~ 독립변인(들)

# 아버지 키 대입 -> 아들 키 추론
## 실제 데이터 적용 
# ft=아버지 키
# st=아들 키
ft=c(156,159,168,177,183,183,192)
st=c(166,166,169,175,183,186,187)
cor(ft,st)
d=cbind(ft,st,rep(1,length(ft)))
plot(d)
m=lm(st~ft)
m

## 이하 검증
tft=seq(156,192)       #seq(min(ft),max(ft))
tst=0.667*tft+60
td=cbind(tft,tst,rep(2,length(tft)))
td
ret=rbind(d,td)
plot(ret,pch = 21,bg = c("red", "green3")[ret[,3]])

# predict를 통한 검증 (위와 같음. 단지 수식을 만들지 않아도 되는 장점이 있을 뿐임.)
predict(m,newdata=data.frame(ft=155))
predict(m,newdata=data.frame(ft=210))
predict(m,newdata=data.frame(ft=90))
ptft=seq(156,192)
ptst=predict(m,newdata=data.frame(ft=seq(156,192)))
ptd=cbind(ptft,ptst,rep(2,length(tft)))
pret=rbind(d,ptd)
plot(pret,pch = 21,bg = c("red", "green3")[pret[,3]])

summary(m)
