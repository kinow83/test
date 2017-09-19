#######################################################
## Detail: 
## Date: 2015/11/23
#######################################################


#install.packages("arules") 
#install.packages("Matrix")
library("arules")
setwd("E:/kaka/R/남중구/AssociationRule")
asso=read.table("ex421.txt",header=T ,fileEncoding = "EUC-KR")
class(asso)
asso
trans<-as.matrix(asso,"Transaction")
trans
class(trans)
rules1<-apriori(trans,parameter=list(supp=0.4,conf=0.6,  target="rules"))
?apriori
rules1
inspect(sort(rules1)) # <- 결과 보기
as(rules1, "data.frame");

rules2<-apriori(trans,parameter=list(suppor=0.6))

#?????? ?????? 결과??? ???거?????? ????????? ???쪽 ????????? ?????? ?????? 결과?????? ???-
rules2.sub_1=subset(rules1,subset=lhs %pin% ""  & lift>0.6)
inspect(sort( rules2.sub_1)[1:3])
inspect(sort(rules2))
result=as(sort( rules2.sub_1)[1:3], "data.frame")
result


# 단점: 컬럼이 많을 수록 연산이 (n^3) 오래걸림