#######################################################
## Detail: Handling external data
## Date: 2015/11/17
#######################################################

# support
# txt, xls, xlsx, csv, DB, 

# 1. txt
# - separator: blank
#   Rdata = read.table(file="filepath/filename.txt", 
#                      header=TRUE,
#                      sep=" ")
?read.table

# hobby.txt
hobby <- read.table(file="E:/kaka/R/hobby.txt", header=TRUE, sep=" ")
hobby
mode(hobby)
mean(hobby$id)
hobby['id']
hobby['hobby']
hobby['religion']
mode(hobby['religion'])
hobby[['religion']]
mode(hobby[['religion']])

# books.txt
books <- read.table(file="E:/kaka/R/books.txt", header=TRUE, sep=",")
books
books['ideal']

# home.txt
home <- read.table(file="E:/kaka/R/home.txt", header=TRUE, sep="\t", stringsAsFactors=FALSE)
home[['home']]
home[['home']][-c(1,2)]


# xls, xlsx
# You can install R's packages
# R's package is
# 1. Data
# 2. Function
# 3. Help
# 4. Program (Example Source code)

# install.packages("package name")
install.packages("readxl")
installed.packages("readxl")

# load package
# require(pakage name) or library(pakage name)
require(readxl)

# readxl::read_excel(path=, 
#                    sheet="sheet name", 
#                    col_names=TRUE)
?readxl::read_excel
rtime = readxl::read_excel(path="E:/kaka/R/time.xlsx", sheet="시트1", col_names=TRUE)
rtime
rtime[['stime']]
mean(rtime[['ttime']])

d1 = data.frame(rtime, home)
d1
d1$stime + d1$ttime
mean(d1$stime + d1$ttime)
matrix(d1$ttime)
matrix(d1$ttime, nrow=2)
array(d1$ttime, dim=c(1,4))
array(d1$ttime, dim=c(4,1))
array(d1$ttime, dim=c(2,2))

d1$stime
d1$ttime
array(c(d1$stime,d1$ttime))
array(c(d1$stime,d1$ttime), dim=c(4,4))


# csv
hope <- read.table(file="E:/kaka/R/hope.csv", header=TRUE)
hope

# 1. Working Directory
# getwd()
getwd()
setwd("E:/kaka/R")
wd <- "E:/kaka/R"
rtime = readxl::read_excel(path="time.xlsx", sheet="시트1", col_names=TRUE)
rtime

# save RData
# load RData
save(rtime, file="home.RData")
rm(list=ls())
?save
?load
load(file="home.RData")
rtime


# save external output file
# 1. by text
#    write.table(RData, file="", sep="")
write.table(home, file="home_output.txt", row.names=FALSE, quote=FALSE)
?write.table

# 2. by csv
write.table(home, file="home_output.csv", row.names=FALSE, quote=FALSE, sep=",")
