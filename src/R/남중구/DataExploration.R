#######################################################
## Detail: 
## Date: 2015/11/23
#######################################################


#dim
dim(iris)

#names
names(iris)

#str : structure
str(iris)

#attributes
attributes(iris)	

#head
head(iris)

#tail
tail(iris)

#summary
summary(iris)

#quantile
quantile(iris$Sepal.Length)

#table
pie(table(iris$Species))

#density 
density(iris$Sepal.Length)

#levels : 몇 종류 인가?
levels(iris$Species)

#cov : 공분산
cov(rnorm(10000),rnorm(10000))
cov(iris$Sepal.Length, iris$Petal.Length)
cov(iris[,1:4])

#cor : 상관분석
cor(iris[,1:4])

#aggregate : 통계 요약
aggregate(Sepal.Length ~ Species, summary, data=iris)
?aggregate

#boxplot : 최대 최소 평균
boxplot(Sepal.Length~Species, data=iris)	

#with : 인자별 scatter Plot을 보여줌
with(iris, plot(Sepal.Length, Sepal.Width, col=Species, pch=as.numeric(Species)))
plot(iris$Sepal.Length, iris$Sepal.Width, col=iris$Species, pch=as.numeric(iris$Species))
?plot
?with

#jitter : 겹치는 것 방지를 위해 노이즈를 줌
with(iris, plot(jitter(Sepal.Length), jitter(Sepal.Width), col=Species, pch=as.numeric(Species)))

#pairs : 
?pairs
(1:10)^3

pairs(cbind(1:10,seq(10,1,-1),(1:10)^3)) # <-n차원을 2차원으로 분석함.
pairs(iris)
pairs(iris[1:4], main = "Anderson's Iris Data -- 3 species",
      pch = 21, bg = c("red", "green3", "blue")[unclass(iris$Species)])

#scatterplot3d
install.packages("scatterplot3d")
library(scatterplot3d)
z <- seq(-10, 10, 0.01);x <- cos(z);y <- sin(z)
scatterplot3d(x, y, z)
scatterplot3d(x, y, z, highlight.3d=TRUE, col.axis="blue",
              col.grid="lightblue", main="scatterplot3d - 1", pch=20)
scatterplot3d(iris$Petal.Width, iris$Sepal.Length, iris$Sepal.Width)  

#levelplot    
install.packages("lattice")
library(lattice)
print(levelplot(b~a*b,df))
df=data.frame(a=1:10,b=1:10)
print(levelplot(Petal.Width~Sepal.Length*Sepal.Width, iris))

#contour
v=matrix(c(1:10000),100,100)
head(v)
for(x in 1:100)
  for(y in 1:100)
    v[x,y]<- sin(x/100*pi)+sin(y/100*pi)
filled.contour(v,plot.axes=contour(v, add=T))
filled.contour(volcano, color=terrain.colors, asp=1,plot.axes=contour(volcano, add=T))

#persp
persp(v,theta=45,phi=30)
persp(volcano, theta = 25, phi = 30, expand = 0.5, col = "lightblue")

#plot3d
install.packages("rgl")	
library(rgl)
plot3d(iris$Petal.Width, iris$Sepal.Length, iris$Sepal.Width)

persp(v,theta=45,phi=30)