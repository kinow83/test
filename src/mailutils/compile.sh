
if [ -f md2.c ]; then
	gcc md2.c -o md2 mailbox/.libs/libmailutils.a -lpthread -lcrypt -g -I include/ bm_timediff.c
fi
if [ -f md3.c ]; then
	gcc md3.c -o md3 libmailutils/.libs/libmailutils.a -lpthread -lcrypt -I include -g bm_timediff.c
fi
