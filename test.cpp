#include <iostream>
#include <fstream>
#include "Mynamespace.pb.h"
#include <google/protobuf/message.h>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#if defined(_MSC_VER)
#include <io.h>
#endif

#define O_BINARY    0x8000
#define O_RDONLY    0x0000

using namespace std;
using namespace Mynamespace;

using google::protobuf::Message;
using google::protobuf::io::FileInputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::CodedInputStream;

#pragma comment( lib,"Debug/libprotobuf.lib")


const int kProtoReadBytesLimit = INT_MAX;  // Max size of 2 GB minus 1 byte.

bool ReadProtoFromBinaryFile(const char* filename, Message* proto)
{
#if defined (_MSC_VER)  // for MSC compiler binary flag needs to be specified
	int fd = open(filename, O_RDONLY | O_BINARY);
#else
	int fd = open(filename, O_RDONLY);
#endif
	
	ZeroCopyInputStream* raw_input = new FileInputStream(fd);
	CodedInputStream* coded_input = new CodedInputStream(raw_input);
	coded_input->SetTotalBytesLimit(kProtoReadBytesLimit, 536870912);
	bool success = proto->ParseFromCodedStream(coded_input);

	delete coded_input;
	delete raw_input;
	close(fd);
	return success;
}

inline bool ReadProtoFromBinaryFile(string filename, Message* proto)
{
	return  ReadProtoFromBinaryFile(filename.c_str(), proto);
}


void WriteProtoToBinaryFile(const Message& proto, const char* filename)
{
	fstream output(filename, ios::out | ios::trunc | ios::binary);
	proto.SerializeToOstream(&output);
}
inline void WriteProtoToBinaryFile(const Message& proto, const string& filename) 
{
	WriteProtoToBinaryFile(proto, filename.c_str());
}



int main()
{
	Blob aa,bb;
	BlobProto bb_proto;

	bb.set_num(99);
	//bb.set_dim(0,1);
	bb.add_dim(1);
	bb.add_dim(2);
	bb.add_dim(3);
	bb.set_dim(2, 7);
	int aaa = bb.dim(2);
	aa.Swap(&bb);

	bb.add_dim(9);
	int aaa1 = bb.dim(0);
	int aaa2 = aa.dim(2);
	const ::google::protobuf::RepeatedField< ::google::protobuf::int64 > bbb = bb.dim();

	string path = "F:\\Protobuf\\MyProtobuf\\lee.caffemodel";
	WriteProtoToBinaryFile(aa,path);
	Blob cc;
	ReadProtoFromBinaryFile(path, &cc);
	int a0 = cc.dim(0);
	int a1 = cc.dim(1);
	int a2 = cc.dim(2);

	return 0;
}