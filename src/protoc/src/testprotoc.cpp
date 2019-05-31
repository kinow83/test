#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "msgspec.pb.h"
//#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>

void show_req(const char*desc, test_req *r)
{
	printf("%s\n", desc);
	printf("id: %d\n", r->id());
	printf("name: %s\n", r->name().c_str());
	printf("query: %s\n", r->query().c_str());
	printf("\n");
}

int main(int argc, char **argv)
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	test_req req, req2, req3;
	//req.ParseFromString("{id: 1}");

	req.set_id(1);
	req.set_name("aaaaaaaaaaaaa11");
	req.set_query("kkkkakaka강석주");

	std::string  output;
	int encodedsize = req.ByteSize();
	printf("encodedsize: %d\n", encodedsize);

	char *encoded = (char *)calloc(encodedsize, 1);
	int seriallen = req.SerializeToArray(encoded, encodedsize);
	printf("seriallen: %d\n", seriallen);
	int fd = open("/tmp/output.txt", O_CREAT|O_TRUNC|O_RDWR);
	if (fd == -1) perror("open");
	req.SerializeToFileDescriptor(fd);

	req.SerializeToString(&output);
	printf("%s\n", output.c_str());

	req2.ParseFromArray(encoded, encodedsize);
	show_req("req2", &req2);
	fd = open("/tmp/output.txt", O_RDONLY);
	req3.ParseFromFileDescriptor(fd);
	show_req("req3", &req3);
	close(fd);

	close(fd);
	google::protobuf::TextFormat::PrintToString(req2, &output);
	printf("show: %s\n", output.c_str());

	free(encoded);
//	google::protobuf::ShutdownProtobufLibrary();
}
