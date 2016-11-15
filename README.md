### Learn Protobuf in VS2013 Platform
Caffe代码里大量使用Google的Protobuf开源代码作为网络模型参数的载体，用来输入和输出模型和网络参数，要想深入了解Caffe代码，免不了需要学习Protobuf的使用。同时，Protobuf可以快速实现内存和硬盘介质的数据交换，是非常好用的一种工具。下面快速学习使用它。

## proto文件的编译 
作者直接从Github上下载[微软移植的Caffe](https://github.com/BVLC/caffe/tree/windows),编译后会自动下载第三方工具包NugetPackages，在包里找到**protobuf-v120.2.6.1**和**protoc_x64.2.6.1**两个文件夹，一个是编译好的include和lib文件，一个是编译好的protoc.exe文件。然后编写proto文件，可以从网上参考proto语言。这里，syntax = "proto2" 表示是proto2语法格式版本(还有proto3语法等)，package Mynamespace 相当于 C++ 里的命名空间，防止相同类和函数命名的冲突，message Blob 在转化C++ 语法后，会生成Blob类，因此直观理解为package->namespace，message->class。optional int64 num 变量 表示Blob类里可以包含或者不包含int64类型的变量num,(其实还有required int64这种格式，表示必须包含该变量，但在变量初始化上可能会存在无法解决的问题，一般Google的程序员都直接使用optional格式)。 repeated int64 dim 变量在转化为 C++ 语言后大致相当于定义了一个 vector<int64> 的dim变量，表示可以存在一个或者多个dim。最后，将代码保存为proto格式的后缀名文件(Mynamespace.proto文件)，文件保存到和 protoc.exe文件 相同的目录，打开cmd命令提示符并且转到该目录下，执行如下命令**protoc.exe --cpp_out=F:\Protobuf Mynamespace.proto** 后，在F:\Protobuf文件夹里会出现Mynamespace.pb.h和Mynamespace.pb.cc文件,这就是protoc生成的Blob、BlobProto类的C++文件。
``` C++
syntax = "proto2";
package Mynamespace;
message Blob {
  repeated int64 dim = 1 [packed = true];
  optional int64 num = 2 [default = 0];
}
message BlobProto {
// 4D dimensions -- deprecated.  Use "shape" instead.
  optional int32 num = 1 [default = 0];
  optional int32 channels = 2 [default = 0];
  optional int32 height = 3 [default = 0];
  optional int32 width = 4 [default = 0];
  repeated float data = 5 [packed = true];
  repeated float diff = 6 [packed = true];
  optional Blob blob = 7;
  repeated Blob blobs = 8;
}
```
## 学习使用Protobuf类
方法很简单，直接看生成的.h和.cc文件里类的代码，学习怎么使用，然后研读Caffe源代码，学习protobuf文件io操作。直接码代码，单步调试学习即可。通过执行代码可以看到，Protobuf能将完整的message类整个输出到二进制文件，然后从二进制文件直接解析出来，确实很方便。
```
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
```

```
@prototxt文件，代码会自动根据变量名和子类名去解析数据而且跟顺序无关，非常好用。
num: 21
channels: 22
height: 117
width: 123
blob{
num : 99
dim: 777
dim   : 888
dim :999
}
blobs{
num : 9
dim: 77
dim   : 88
dim :99
}
blobs{
num : 12
dim: 7
dim   : 8
dim :9
}
```

