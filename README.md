# DirectIO 📌
## `Windows`下`Direct`模式读写文件相关说明📒



## **1.** ***\*什么是Direct IO\****

正常读写文件时，数据是在物理磁盘——高速页缓存（内核地址空间）——应用缓存（用户地址空间）传递的。

**<u>`DirectIO`就是指读写文件时绕过内核的高速页缓存，自己管理IO缓冲区</u>**，与内存映射没有本质上的差异。这样减少了一次从内核缓存到用户缓存的数据拷贝，在某些情况下，可以适当使用来提高性能。

 

## **2.** ***\*DirectIO的使用限制与适用场景\****

使用限制：由于`Direct IO`不会和内存打交道，而是直接写入存储设置中，由于存储设备的管理单元是扇区，所以`Direct IO`的`offset`和`length`必须和块大小对齐，以`windows`的磁盘扇区大小为512字节为例，`offset`和`length`的大小必须是512的整数倍。**<u>并且申请的存放数据的`buffer`内存地址也必须与扇区大小对齐。</u>**

适用场景：每次都是读写较大块，并且块大小相对比较固定的数据。

 

## **3.** ***\*Windows下Direct IO方式相关的接口以及使用说明\****

这些接口基本都在`Fileapi.h`里定义，实际不仅适用于`Direct`模式读写文件，也可以操作设备，或者以buffered模式写文件等等，这里主要介绍以`Direct IO`模式读写文件使用说明。

### **3.1** ***\*CreateFileA()\****

`HANDLE CreateFileA (`

`LPCSTR lpFileName,` 

`DWORD dwDesiredAccess,` 

`DWORD dwShareMode,` 

`LPSECURITY_ATTRIBUTES lpSecurityAttributes,` 

`DWORD dwCreationDisposition,` 

`DWORD dwFlagsAndAttributes,` 

`HANDLE hTemplateFile`

`);`

 

`lpFileName`参数是`const char *`类型，表示文件名

 

`dwDesiredAccess`参数是读写权限设置，常用的有：`GENERIC_READ，GENERIC_WRITE`，`( GENERIC_READ | GENERIC_WRITE )`



`dwShareMode`参数是控制当文件被成功打开，并获取到句柄时，在关闭句柄前，另一个进程能否再次打开这个文件。常用的有：0 （当文件打开时，只有一个进程有这个文件的读写删除权限），`FILE_SHARE_DELETE` （当一个进程打开文件，另一个进程可以有删除或者重命名权限），`FILE_SHARE_WRITE` （当一个进程打开文件，另一个进程可以有写权限），`FILE_SHARE_READ` （当一个进程打开文件，另一个进程可以有读权限）。

 

`lpSecurityAttributes`参数是一个`SECURITY_ATTRIBUTE`结构体指针，可以设置为`NULL`，当设置为`NULL`时，返回的`HANDLE`不能被任何子进程继承，并为这个文件指定一个默认的安全描述符。当待打开的文件已经存在时，会忽略该参数。

 

`dwCreationDisposition`参数是指定对已存在或不存在文件采取的措施。常用的是`CREATE_ALWAYS`：创建一个新文件，若已存在且可写，则会重写该文件。

`CREATE_NEW`：只有当文件不存在时，会创建新文件并返回成功。

`OPEN_ALWAYS`：当文件存在时，打开该文件，文件不存在时创建该文件。

`OPEN_EXISTING`：只有当文件存在时，打开文件并返回成功。

`TRUNCATE_EXISTIN`：只有当文件存在时，打开文件并截断为0字节。

 

`dwFlagsAndAttributes`参数是指定是否是`Direct`模式写文件的重要参数

需要指定为：`FILE_FLAG_NO_BUFFERING`

也可以选择：`( FILE_FLAG_NO_BUFFERINg | FILE_FLAG_WRITE_THROUGH )` 这样可以不调用`FlushFileBuffers()`，仍能保证每次写入到磁盘中，但速度会有所下降，频繁调用`FlushFileBuffers()`也会导致速度下降。

 

`hTemplateFile`参数是传递另一个文件句柄，然后从这个句柄继承前述这些属性，一般设为`NULL`。

 

返回值：成功返回文件句柄，失败返回`INVALID_HANDLE_VALUE`

使用实例：

`char fileName[] = "D:/testDirect.dat";`

`HANDLE fileFD = CreateFileA(fileName,GENERIC_WRITE,0,NULL,`

`CREATE_ALWAYS,FILE_FLAG_NO_BUFFERING,NULL);`
`if (fileFD == INVALID_HANDLE_VALUE) {`
  `PR_ERR("Create File Failed!");`
  `return 0;`
`}`

 

### **3.2** ***\*_aligned_malloc() 与 _aligned_free()\****

`void _aligned_malloc(size_t _Size,size_t _Alignment);` 

`void _aligned_free(void *_Memory);`

 

`_aligned_malloc`的作用是在申请内存时，保证申请的内存地址与限制的大小对齐。`_Size`表示需要申请的内存大小，_`Alignment`是表示要对其的字节数，一般设为扇区大小。

`_aligned_free`的作用是释放通过_`aligned_malloc`申请的内存。

使用实例：

`char *formatPtr = (char *) _aligned_malloc(COMMON_BLOCK_SIZE,`

`SECTION_SIZE);`
`if(formatPtr == NULL)`
`{`
  `PR_ERR("Create aligned Ptr failed");`
  `return 0;`
`}`

`// do something here...`

`_aligned_free(formatPtr);`



***\*注意：\******<u>如果申请的`size`不是`alignment`的整数倍，在`malloc`时仍然能成功，但是在`free`时程序会崩溃。</u>**

### **3.3** ***\*GetDiskFreeSpaceA()\****

`WINBOOL GetDiskFreeSpaceA (`

`LPCSTR lpRootPathName,` 

`LPDWORD lpSectorsPerCluster,` 

`LPDWORD lpBytesPerSector,` 

`LPDWORD lpNumberOfFreeClusters,` 

`LPDWORD lpTotalNumberOfClusters`

`);`

 

该函数可以获取磁盘大小，扇区大小，簇个数等等信息，这里主要利用该函数获取磁盘扇区大小。

使用实例：

`DWORD pageSize = 0;`
`if(!GetDiskFreeSpaceA(NULL,NULL,&pageSize,NULL,NULL))`
  `PR_ERR("getPageSize failed!");`

 

### **3.4** ***\*WriteFile()\****

`WINBOOL WriteFile (`

`HANDLE hFile,` 

`LPCVOID lpBuffer,` 

`DWORD nNumberOfBytesToWrite,` 

`LPDWORD lpNumberOfBytesWritten,` 

`LPOVERLAPPED lpOverlapped`

`);`

 

参数分别为文件句柄，存放数据的buffer，要求写入字节，实际写入字节，最后一个参数只有当`CreateFileA`时`dwFlagsAndAttributes`为`FILE_FLAG_OVERLAPPED`时需要使用，一般可设为`NULL`。

 

使用实例：

 

`DWORD slice = 0;`

`bool writeRt = WriteFile(fileFD,formatPtr,COMMON_BLOCK_SIZE,`

`&slice,NULL);`

``if (!writeRt)`
 `{`
  `PR_ERR("Write File Failed!  slice:%ld",slice);`
  `return 0;`
 }`

 

***\*注意：\****存放数据的buffer地址必须对齐扇区大小

 

### **3.5** ***\*ReadFile()\****

`WINBOOL ReadFile (`

`HANDLE hFile,` 

`LPVOID lpBuffer,` 

`DWORD nNumberOfBytesToRead,` 

`LPDWORD lpNumberOfBytesRead,` 

`LPOVERLAPPED lpOverlapped`

`);`

 

用法和注意事项同`WriteFile()`，这里不再赘述。

 

### **3.6** ***\*FlushFileBuffers()\****

`WINBOOL FlushFileBuffers (HANDLE hFile);`

 

强制刷新数据到磁盘中，类似于`fflush()`，如果`CreateFileA`时`dwFlagsAndAttributes`为( `FILE_FLAG_NO_BUFFERINg | FILE_FLAG_WRITE_THROUGH )`，那么就不用调用`FlushFileBuffers()，`频繁刷新会导致速度很慢，平常使用时建议300M数据左右刷新一次。

 

### **3.7** ***\*SetFilePointer()\****

`DWORD SetFilePointer (`

`HANDLE hFile,` 

`LONG lDistanceToMove,` 

`PLONG lpDistanceToMoveHigh,` 

`DWORD dwMoveMethod`

`);`

 

`hFile`表示待操作的文件句柄

`lDistanceToMove`是`int32`类型，正数表示向前偏移，负数向回偏移。当偏移大小在`int32`能表示的范围时，应将`lpDistanceToMoveHigh`设为`NULL` ，当偏移大小在`int32`能表示的范围之外时，需要将`lDistanceToMove`作为`int64`的低32位，`lDistanceToMoveHigh`作为`int64`的高32位组成的新数，作为偏移大小。

`dwMoveMethod`表示偏移开始位置，常用参数：`FILE_BEGIN，FILE_CURRENT，FILE_END`。

返回值：成功时返回成功偏移的低32位，如果用到的高32位，则会传递到`lpDistanceToMoveHigh`参数中，失败则返回`INVALID_SET_FILE_POINTER`。

 

***\*注意：\* ***`Direct IO`方式时，偏移量也必须是扇区大小的整数倍。

 

### **3.8** ***\*CloseHandle()\****

`WINBOOL CloseHandle (HANDLE hObject);`



关闭文件句柄。

 