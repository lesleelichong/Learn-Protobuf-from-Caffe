#include <iostream>
#include <fstream>
#include "Mynamespace.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

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

bool ReadProtoFromTextFile(const char* filename, Message* proto) 
{
	int fd = open(filename, O_RDONLY);
	
	FileInputStream* input = new FileInputStream(fd);
	bool success = google::protobuf::TextFormat::Parse(input, proto);
	delete input;
	close(fd);
	return success;
}
inline void  ReadProtoFromTextFile(const string& filename, Message* proto)
{
	ReadProtoFromTextFile(filename.c_str(), proto);
}

void iShowBlobProto(BlobProto& blob_proto)
{
	printf("num = %d\n",blob_proto.num());
	printf("channels= %d\n", blob_proto.channels());
	printf("height = %d\n", blob_proto.height());
	printf("width = %d\n", blob_proto.width());
	for (int i = 0; i < blob_proto.data_size(); ++i)
	{
		printf("data = %f\n", blob_proto.data(i));
	}
	for (int i = 0; i < blob_proto.diff_size(); ++i)
	{
		printf("diff = %f\n", blob_proto.diff(i));
	}
	for (int i = 0; i < blob_proto.blob().dim_size(); ++i)
	{
		int num = blob_proto.blob().num();
		::google::protobuf::int64 dim = blob_proto.blob().dim(i);
		printf("num = %d,dim = %d\n", num, dim);
		//////下面这简化的一行代码在 vs2013的win32下输出显示有错。但是x64不会报错。
		//printf("num = %d,dim = %d\n", blob_proto.blob().num(),blob_proto.blob().dim(i));		
	}
	for (int i = 0; i < blob_proto.blobs_size(); ++i)
	{
		Blob tmp = blob_proto.blobs(i);
		printf("num = %d\n",tmp.num());
		for (int j = 0; j < tmp.dim_size(); ++j)
		{
			printf("dim = %d\n", tmp.dim(j));
		}
	}
	printf("*****************************\n");
}

int main()
{
	Blob* blob1 = new Blob;
	Blob blob2;
	BlobProto blob_proto;
	//////设置blob1
	blob1->set_num(99);
	for (int i = 0; i < 5; ++i)
	{
		blob1->add_dim(i);
	}
	//////设置blob2
	blob2.set_num(1);
	for (int i = 0; i < 5; ++i)
	{
		blob2.add_dim(int(5 - i));
	}
	blob1->Swap(&blob2);
	const ::google::protobuf::RepeatedField< ::google::protobuf::int64 >  dims = blob1->dim();
	//////设置blob_proto
	blob_proto.set_num(1);
	blob_proto.set_channels(2);
	blob_proto.set_height(3);
	blob_proto.set_width(1);
	for (int i = 0; i < 5; ++i)
	{
		blob_proto.add_data(0.5 + i);
		blob_proto.add_diff( 5.5 - i);
	}
	//////这里非常需要注意，是指针传入，但是最后析构函数又会 自动delete这个内存，因此不需要自己再delete，只需要new 即可。
	//////这也是protobuf不太方便的一个地方，可以自己修改代码，或者采用其他方法，但那样需要复制这块内存，影响速度。
	blob_proto.set_allocated_blob(blob1);

	string path = "..\\lee.caffemodel";
	WriteProtoToBinaryFile(blob_proto, path);
	BlobProto  blob1_proto;
	ReadProtoFromBinaryFile(path, &blob1_proto);
	string textpath = "F:\\Github\\Learn-Protobuf-from-Caffe\\BlobProto.prototxt";
	BlobProto  blob2_proto;
	ReadProtoFromTextFile(textpath, &blob2_proto);
	blob2_proto.add_blobs()->CopyFrom(blob2);
	iShowBlobProto(blob_proto);
	iShowBlobProto(blob1_proto);
	iShowBlobProto(blob2_proto);
	//delete blob1;
	return 0;
}