#######################################################
## Detail: About Data or Object
## Date: 2015/11/16
#######################################################

# Scalar
x<-3
y<-"KANG"
z<-FALSE

#################################################
# Vector (extension of Scalar, only exist one column)
# 1. c(Scalar, Scalar, Scalar, ...)
# vector of Numeric or Character or Logical
#################################################
v1<-c(1,2,3,4,5)
typeof(v1)
is.double(v1)

v2<-c("Kim", "Lee", "Park", v1)
v3<-c(FALSE, FALSE, TRUE)
v4<-c(1==1, "a"=="a")
v5<-c(3,10,20,30)

# extract vector (index of vector)
# start index MUST be '1'
age <- c(29, 50, 100, 18)
age[c(1,3)]
#rep(c(1,3), times=10, each=c(1,2))
age[-3] # '-' is exclusive index '3'
age[-c(1,2)]

# operation vector
v1 = 1:3
v2 = 4:6
v1 + v2
v1 - v2
v1 * v2
v1 / v2
v1 ^ v2
v1*3
1:3*3
(1:3)*3
c(1:3)*3

v3 = 1:6
v1 + v3 # ?????????????????????????????
        # TODO: Recycling Rule
        # v1: 1 2 3 [1 2 3]  <- Recycling Rule
        # v3: 1 2 3  4 5 6
   # v1 + v3: 2 3 5  6 7 8

# vector is only one type.
# priority of vecotr's data type (Character > Numeric > Logical)
v6<-c(1, "Lee", TRUE)
v6<-c(1, TRUE)

v4 = 1:8
v1 + v4



# as.xxx(data)
# type casting
x = c(1,2,3)
y = as.character(x)
y = c("S", "k", "Y")
z = as.numeric(y)
x == z
as.logical(x)

#################################################
# 2. :
# Only numeric (iterator number of vector and case 1 increment)
# start < end : +1
# start > end : -1
# start = end : start(or end)
#################################################
1:5
1:100
1:-100
1:3.14
1:"100"
-3.3:2 # > -3.3 -2.3 -1.3 -0.3  0.7  1.7
1:-3.3

#################################################
# 3. seq(from, to, by)
# Only numeric (extension of ':')
#################################################
seq(-3.3, 2, 2) # > -3.3 -1.3  0.7

seq(from=5,to=1,by=-0.1)

#################################################
# 4. sequenc(numeric)
# 1 ~ integer number
#################################################
sequence(5)
sequence(3.3)
sequence(1)
sequence(-2) # Error

# > 1 1 2 1 2 3 1 2 3 4 1 2 3 4 5 1 2 3 4 5 6
sequence(1:6)
sequence(1:"6")
sequence(1:3.14)
sequence(1:-3) # Error

#################################################
# 5. 배열 복사
# rep(data, times=, each=)
# replicate
#################################################
?rep
rep(1:4, times=5) # 1 2 3 4 1 2 3 4 1 2 3 4 1 2 3 4 1 2 3 4
rep(1:4, each=5)  # 1 1 1 1 1 2 2 2 2 2 3 3 3 3 3 4 4 4 4 4
rep(sequence(4), times=10)
rep(1:4, each=10)
rep("a,b", times=10)
rep(FALSE, times=10)
rep(c('a','b'), times=10)
rep(c('a','b'), each=10)
rep(1:2, times=3, each=3) # each > times

# 1: 100, 2:50, 3:10
rep(c(1,2,3), times=c(100,50,10))
c(rep(1, times=100), rep(2, times=50), rep(3, times=10))

#################################################
# 6. paste(data1, data2, ..., sep=)
#################################################
paste(1, 1, 1)
paste("Love", "is", "choice.")
paste(1, 1, 1, sep="")
paste(1, 1, 1, sep="_")

# "V1" "V2" "V1" ... "V10"
paste("V", 1:10, sep="") # <- Recycling Rule

#################################################
# 7. length(vector var)
#################################################
x = c(7,77,107,207)
length(x):1
x[length(x):1]

#################################################
# 8. mode(x)
#################################################
mode(x)
eval("xx=10")
