#include <iostream>
#include <fileapi.h>
#include "commom.h"


int writeDirect(HANDLE fd,char *buf,int len);
void logInit();

/**
 * Windows下测试
 * 相关信息：
 *  每个扇区字节数 :                  512
    每个簇字节数 :                    4096
    每个 FileRecord 段的字节数 :      1024
 * 测试使用YFile的普通windows系统IO与DIRECT模式的性能差别
 * 因为光纤块大小1M，所以每次1M的write进行模拟
 * @return
 */

int main() {
    logInit();
    char *srcBuf = new char[COMMON_BLOCK_SIZE];
    memset(srcBuf,0,COMMON_BLOCK_SIZE);
    char fileName[] = "D:/testDirect.dat";

    /*使用direct模式*/
#if defined (USE_DIRECT_WRITE)

    /*采用directIO时，直接从用户内存到磁盘空间，所以要使指针地址对齐扇区*/
    char *formatPtr = (char *) _aligned_malloc(COMMON_BLOCK_SIZE,SECTION_SIZE);
    if(formatPtr == NULL)
    {
        PR_ERR("Create aligned Ptr failed");
        return 0;
    }
    PR_DEBUG("Create format ptr succeed!");
    memcpy(formatPtr,srcBuf,COMMON_BLOCK_SIZE);

    /*FILE_FLAG_NO_BUFFERING指定不经过高速页缓存*/
    HANDLE fileFD = CreateFile(fileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_FLAG_NO_BUFFERING,NULL);
    if (fileFD == INVALID_HANDLE_VALUE) {
        PR_ERR("Create File Failed!");
        return 0;
    }
    PR_DEBUG("Create fileFD succeed!");
    DWORD page = 0;
    if(!GetDiskFreeSpaceA(NULL,NULL,&page,NULL,NULL))
        PR_ERR("getPageSize failed!");
    std::cout<<page<<endl;

    /*表示实际写入成功的值*/
    DWORD slice = 0;

    /*总共写的次数,每次大小BLOCK_SIZE*/
    uint64_t blockNum = 1024*10;
    for (int i = 0; i < blockNum; ++i) {
        bool writeRt = WriteFile(fileFD,formatPtr,COMMON_BLOCK_SIZE,&slice,NULL);
        if(!writeRt)
        {
            PR_ERR("Write File Failed!  slice:%ld",slice);
            return 0;
        }
        FlushFileBuffers(fileFD);
        sleep(5);
        if(i%200 == 0)
            /*刷新*/
            FlushFileBuffers(fileFD);
    }
    /*刷新*/
    FlushFileBuffers(fileFD);

    /*release source*/
    delete(srcBuf);
    _aligned_free(formatPtr);
    CloseHandle(fileFD);

#elif defined(USE_DIRECT_READ)

    /*采用directIO时，直接从用户内存到磁盘空间，所以要使指针地址对齐扇区*/
    char *formatPtr;
    formatPtr = (char *) _aligned_malloc(COMMON_BLOCK_SIZE,SECTION_SIZE);
    if(formatPtr == NULL)
    {
        PR_ERR("Create aligned Ptr failed");
        return 0;
    }
    PR_DEBUG("Create format ptr succeed!");
    memset(formatPtr,0,COMMON_BLOCK_SIZE);

    /*FILE_FLAG_NO_BUFFERING指定不经过高速页缓存*/
    HANDLE fileFD = CreateFile(fileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_NO_BUFFERING,NULL);
    if (fileFD == INVALID_HANDLE_VALUE) {
        PR_ERR("Create File Failed!");
        return 0;
    }
    PR_DEBUG("Create fileFD succeed!");

    /*表示实际读取成功的值*/
    DWORD slice = 0;

    /*总共写的次数,每次大小BLOCK_SIZE*/
    uint64_t blockNum = 1024*10;
    for (int i = 0; i < blockNum; ++i) {
        bool readRt = ReadFile(fileFD,formatPtr,COMMON_BLOCK_SIZE,&slice,NULL);
        if(!readRt)
        {
            PR_ERR("Write File Failed!  slice:%ld",slice);
            return 0;
        }
    }


    /*release source*/
    delete(srcBuf);
    _aligned_free(formatPtr);
    CloseHandle(fileFD);


#endif

    std::cout << "exiting..." << std::endl;
    sleep(5);
    return 0;
}

int writeDirect(HANDLE fd,char *buf,int len)
{
    if(fd == INVALID_HANDLE_VALUE)
    {
        ;
    }
    return 0;
}

void logInit() {
    int ret;
    ret = base::logInit(MAINCTL_LOG_NAME,
                        7,
                        TRUE,
                        "D:/log/log.txt",
                        1048576,
                        5
    );
    if(ret) {
        printf("logInit error: %d\n", ret);
    }
    printf("log test\n");
}