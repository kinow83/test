#######################################################
## Detail: 
## Date: 2015/11/23
#######################################################

set.seed(1234)
#모델을 생성하기 위한 집단과 평가를 위한 집단을 분리하기 위해서
ind <- sample(2, nrow(iris), replace=TRUE, prob=c(0.7, 0.3))
trainData <- iris[ind==1,] 
testData <- iris[ind==2,]

#install.packages("party")
library(party)
myFormula <- Species ~ Sepal.Length + Sepal.Width + Petal.Length + Petal.Width
iris_ctree <- ctree(myFormula, data=trainData)
table(predict(iris_ctree), trainData$Species)

#결과 출력
print(iris_ctree)
plot(iris_ctree)
# 꽃잎의 길이가 1.9이하면 setosa
# 꽃입의 넓이가 1.7보다 크면 versinica
# 나머지는 versicolor이다. 
plot(iris_ctree,type="simple")

#평가
testPred <- predict(iris_ctree, newdata = testData)
table(testPred, testData$Species)




# 같은 정도.... 대각선 값만 존재하는 경우 c1과 c2가 같음
c1=c(1,1,1,2,2,3)
c2=c(1,1,1,2,3,3)
table(c1,c2)
