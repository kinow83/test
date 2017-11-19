gcc mime_decode.c -o mime_decode -I include mailbox/.libs/libmailutils.a -lpthread -lcrypt -g
gcc readline.c    -o readline    -I include mailbox/.libs/libmailutils.a -lpthread -lcrypt -g
