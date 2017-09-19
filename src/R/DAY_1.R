#######################################################
## Detail: base of R
## Date: 2015/11/16
#######################################################

3+4
3-4
3*4
3**4
3^4

# root 5
5**(1/2)
5**0.5

# assign variable
# <-, =, ->
x<-5
y=6
7->z

x^3 - x*(y^2)*z - 2*(z^4)

# compare expression
3>4
3>=4
3<4
3<=4
3==4
3!=4
!(3==4)

# type of data
# Numeric
# Character
# Logical (Boolean)
x=4
y="Kang"
z=TRUE

# What is kind of type of data?
# mode(data)
mode(x)
strx<-mode(x)
mode(strx)
mode(y)
mode(z)
# is.xxx()
is.numeric(x); is.numeric(y); is.numeric(z)
is.character(x); is.character(y); is.character(z); 
is.logical(x); is.logical(y); is.logical(z);
isx=is.numeric(x)
mode(isx)

# control data of user
# ls
ls()
rm(x, y, z)
remove(l1, strx, isx)

ls()
x=1; y="2"; z=TRUE
arr=ls()
typeof(arr)
arr

# remove all
rm(list=ls())

