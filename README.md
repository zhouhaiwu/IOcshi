# IOcshi
磁盘异步AIO
--------------------------
Linux下原生异步IO接口Libaio的用法
Posted on June 15, 2011 by Jian Zhou

libaio是linux下原生的异步IO接口。网上对其使用方法讨论较少，这里做个简单说明。libaio的使用并不复杂，过程为：libaio的初始化，io请求的下发和回收，libaio销毁。
一、libaio接口

libaio提供下面五个主要API函数：

int io_setup(int maxevents, io_context_t *ctxp);
int io_destroy(io_context_t ctx);
int io_submit(io_context_t ctx, long nr, struct iocb *ios[]);
int io_cancel(io_context_t ctx, struct iocb *iocb, struct io_event *evt);
int io_getevents(io_context_t ctx_id, long min_nr, long nr, struct io_event *events, struct timespec *timeout);

五个宏定义：

void io_set_callback(struct iocb *iocb, io_callback_t cb);
void io_prep_pwrite(struct iocb *iocb, int fd, void *buf, size_t count, long long offset);
void io_prep_pread(struct iocb *iocb, int fd, void *buf, size_t count, long long offset);
void io_prep_pwritev(struct iocb *iocb, int fd, const struct iovec *iov, int iovcnt, long long offset);
void io_prep_preadv(struct iocb *iocb, int fd, const struct iovec *iov, int iovcnt, long long offset);

这五个宏定义都是操作struct iocb的结构体。struct iocb是libaio中很重要的一个结构体，用于表示IO，但是其结构略显复杂，为了保持封装性不建议直接操作其元素而用上面五个宏定义操作。
二、libaio的初始化和销毁

观察libaio五个主要API，都用到类型为io_context的变量，这个变量为libaio的工作空间。不用具体去了解这个变量的结构，只需要了解其相关操作。创建和销毁libaio分别用到io_setup（也可以用io_queue_init，区别只是名字不一样而已）和io_destroy。

int io_setup(int maxevents, io_context_t *ctxp);
int io_destroy(io_context_t ctx);
三、libaio读写请求的下发和回收

1. 请求下发

libaio的读写请求都用io_submit下发。下发前通过io_prep_pwrite和io_prep_pread生成iocb的结构体，做为io_submit的参数。这个结构体中指定了读写类型、起始扇区、长度和设备标志符。

libaio的初始化不是针对一个具体设备进行初始，而是创建一个libaio的工作环境。读写请求下发到哪个设备是通过open函数打开的设备标志符指定。

2. 请求返回

读写请求下发之后，使用io_getevents函数等待io结束信号：

int io_getevents(io_context_t ctx_id, long min_nr, long nr, struct io_event *events, struct timespec *timeout);

io_getevents返回events的数组，其参数events为数组首地址，nr为数组长度（即最大返回的event数），min_nr为最少返回的events数。timeout可填NULL表示无等待超时。io_event结构体的声明为：

struct io_event {
   PADDEDptr(void *data, __pad1);
   PADDEDptr(struct iocb *obj,  __pad2);
   PADDEDul(res,  __pad3);
   PADDEDul(res2, __pad4);
};

其中，res为实际完成的字节数；res2为读写成功状态，0表示成功；obj为之前下发的struct iocb结构体。这里有必要了解一下struct iocb这个结构体的主要内容：

iocbp->iocb.u.c.nbytes 字节数
iocbp->iocb.u.c.offset 偏移
iocbp->iocb.u.c.buf 缓冲空间
iocbp->iocb.u.c.flags 读写

3. 自定义字段

struct iocb除了自带的元素外，还留有供用户自定义的元素，包括回调函数和void *的data指针。如果在请求下发前用io_set_callback绑定用户自定义的回调函数，那么请求返回后就可以显示的调用该函数。回调函数的类型为：

void callback_function(io_context_t ctx, struct iocb *iocb, long res, long res2);

另外，还可以通过iocbp->data指针挂上用户自己的数据。

注意：实际使用中发现回调函数和data指针不能同时用，可能回调函数本身就是使用的data指针。
