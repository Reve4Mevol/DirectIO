//
// Created by Reve4Mevol on 2020/4/20.
//

#ifndef DIRTCTIO_TEST_COMMOM_H
#define DIRTCTIO_TEST_COMMOM_H


/*块大小*/
#define COMMON_BLOCK_SIZE 1024*1024UL

/*扇区大小*/
#define SECTION_SIZE 512

/*是否使用direct模式*/
#define USE_DIRECT_WRITE 1
//#define USE_DIRECT_READ 1


#define MAINCTL_LOG_NAME      "DIRECT_IO"    // 日志名称
#define PR_CRIT(pFmt, ...)   PRINT_LOG(MAINCTL_LOG_NAME, LOG_LEVEL_CRIT, pFmt, ##__VA_ARGS__)
#define PR_ERR(pFmt, ...)    PRINT_LOG(MAINCTL_LOG_NAME, LOG_LEVEL_ERR, pFmt, ##__VA_ARGS__)
#define PR_WARN(pFmt, ...)   PRINT_LOG(MAINCTL_LOG_NAME, LOG_LEVEL_WARN, pFmt, ##__VA_ARGS__)
#define PR_NOTICE(pFmt, ...) PRINT_LOG(MAINCTL_LOG_NAME, LOG_LEVEL_NOTICE, pFmt, ##__VA_ARGS__)
#define PR_INFO(pFmt, ...)   PRINT_LOG(MAINCTL_LOG_NAME, LOG_LEVEL_INFO, pFmt, ##__VA_ARGS__)
#define PR_DEBUG(pFmt, ...)  PRINT_LOG(MAINCTL_LOG_NAME, LOG_LEVEL_DEBUG, pFmt, ##__VA_ARGS__)
#define PR_TRACE(pFmt, ...)  PRINT_LOG(MAINCTL_LOG_NAME, LOG_LEVEL_TRACE, pFmt, ##__VA_ARGS__)

#endif //DIRTCTIO_TEST_COMMOM_H
