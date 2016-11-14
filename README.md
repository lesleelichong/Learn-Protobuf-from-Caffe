### Learn Protobuf in VS2013 Platform
Caffe代码里大量使用Google的Protobuf开源代码作为网络模型参数的载体，用来输入和输出模型和网络参数，要想深入了解Caffe代码，免不了需要学习Protobuf的使用。同时，Protobuf可以快速实现内存和硬盘介质的数据交换，是非常好用的一种工具。下面快速学习使用它。

## proto文件的编译 
作者直接从Github上下载[微软移植的Caffe](https://github.com/BVLC/caffe/tree/windows),编译后会自动下载第三方工具包NugetPackages，在包里找到**protobuf-v120.2.6.1**和**protoc_x64.2.6.1**两个文件夹，一个是编译好的include和lib文件，一个是编译好的protoc.exe文件。然后编写proto文件，可以从网上参考proto语言。这里，syntax = "proto2" 表示是proto2语法格式版本(还有proto3语法等)，package Mynamespace 相当于 C++ 里的命名空间，防止相同类和函数命名的冲突，message Blob 在转化C++ 语法后，会生成Blob类，因此直观理解为package->namespace，message->class。optional int64 num 变量 表示Blob类里可以包含或者不包含int64类型的变量num,(其实还有required int64这种格式，表示必须包含该变量，但在变量初始化上可能会存在无法解决的问题，一般Google的程序员都直接使用optional格式)。 repeated int64 dim 变量在转化为 C++ 语言后大致相当于定义了一个 vector<int64> 的dim变量，表示可以存在一个或者多个dim。最后，将代码保存为proto格式的后缀名文件(Mynamespace.proto文件)，文件保存到和 protoc.exe文件 相同的目录，打开cmd命令提示符并且转到该目录下，执行如下命令**protoc.exe --cpp_out=F:\Protobuf Mynamespace.proto** 后，在F:\Protobuf文件夹里会出现Mynamespace.pb.h和Mynamespace.pb.cc文件,这就是protoc生成的Blob、BlobProto类的C++文件。
``` C++
syntax = "proto2";
package Mynamespace;
message Blob 
{
  repeated int64 dim = 1 [packed = true];
  optional int64 num = 2 [default = 0];
}
message BlobProto 
{
// 4D dimensions -- deprecated.  Use "shape" instead.
  optional int32 num = 1 [default = 0];
  optional int32 channels = 2 [default = 0];
  optional int32 height = 3 [default = 0];
  optional int32 width = 4 [default = 0];
  repeated float data = 5 [packed = true];
  repeated float diff = 6 [packed = true];
  optional Blob blob = 7;
}
```
## 学习使用Protobuf类
方法很简单，直接看生成的.h和.cc文件里类的代码，学习怎么使用，然后研读Caffe源代码，学习protobuf文件io操作。直接码代码，单步调试学习即可。
```
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
```

