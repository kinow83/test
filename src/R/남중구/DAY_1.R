#######################################################
## Detail: 
## Date: 2015/11/23
#######################################################


x=matrix(1:100, nrow=10)
x[3:6, 3:5]

head(iris)


mode(iris$Sepal.Length)
iris$Sepal.Length
iris[1,1]

iris

head(as.matrix(iris[1:2,1:4]))
head(iris[,1:4])




dept=data.frame(
  deptno=c(10,20,30,40),  
  dname=c("ACCOUNTING","RESEARCH","SALES","OPERATIONS"),       
  loc=c("NEW YORK","DALLAS","CHICAGO","BOSTON")
)

dept

