year <- rep(2008:2010, each=4)
quarter <- rep(1:4, 3)
cpi <- c(162.2, 164.6, 166.5, 166.0, 166.2, 167.0, 168.6, 169.5, 171.0, 172.1, 173.3, 174.0)
plot(cpi, xaxt="n", ylab="CPI", xlab="")
axis(1, labels=paste(year,quarter,sep="Q"), at=1:12, las=3)

cor(year,cpi)
cor(quarter,cpi)
fit <- lm(cpi ~ year + quarter)
fit
plot(fit)
