#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <libaio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>

//int srcfd = -1;
//int odsfd = -1;
int srcfd[5] = {-1, -1, -1, -1, -1};
//int b[10] = {};  //  数组用来存放IO大小
const char *mysrc[5] = {"hello.txt", "hello1.txt", "hello2.txt", "hello3.txt", "hello4.txt"};

#define AIO_BLKSIZE  1024
#define AIO_MAXIO 64

void unix_error(char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

pid_t Fork(void) {
    pid_t pid;

    if((pid = fork()) < 0)
        unix_error("Fork error");
    return pid;
}

static void wr_done(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
    if (res2 != 0) {
        printf("aio write error \n");
    }
    if (res != iocb->u.c.nbytes) {
        printf("write missed bytes expect %d got %d \n", iocb->u.c.nbytes, res);
        exit(1);
    }

    free(iocb->u.c.buf);
    free(iocb);
}

static void rd_done(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
    
    int iosize = iocb->u.c.nbytes;
    char *buf = (char *)iocb->u.c.buf;
    off_t offset = iocb->u.c.offset;
    int tmp;
    char *wrbuff = NULL;

    if (res2 != 0) {
        printf("aio read \n");
    }
    if (res != iosize) {
        printf("read missing bytes expect %d got %d", iocb->u.c.nbytes, res);
        exit(1);
    }

    
    /*tmp = posix_memalign((void **)&wrbuff, getpagesize(), AIO_BLKSIZE);
    if (tmp < 0) {
        printf("posix_memalign222 \n");
        exit(1);
    }

    snprintf(wrbuff, iosize + 1, "%s", buf);

    printf("wrbuff - len = %d:%s \n", strlen(wrbuff), wrbuff);
    printf("wrbuff_len = %d \n", strlen(wrbuff));
    free(buf);*/

    //io_prep_pwrite(iocb, odsfd, wrbuff, iosize, offset);
    //设置回调函数
    //io_set_callback(iocb, wr_done);

    /*if (1 != (res = io_submit(ctx, 1, &iocb)))
        printf("io_submit write error \n");

    printf("\nsubmit %d write request \n", res);*/
}

int main(int args, void *argv[])
{
    pid_t pid[atoi(argv[2])];
    pid_t pidt;
    pid_t retpid;
    pid_t retpidt;
    int length = sizeof("abcdefg");
    char *content = (char *)malloc(length);
    io_context_t myctx;
    int rc;
    int res;
    char *buff = NULL;
    int offset = 0;
    int num, i, tmp; // 变量i是创建多少个子进程
    int status;
    int statust;
    int a; //随机数，通过a的大小，判断是读还是写
    int b; // 随机读取哪个文件
    int per;
    int k = 0; // k 数组的元素下标
    int j = 0; //  变量j是统计子进程的个数

    //long    n = 10000000L;  //计算时间
    //clock_t start, finish;  //计算时间
    //double  duration;  //计算时间

    struct timeval start, end; //获取AIO时间
    int timeuse; //获取AIO时间
    //int b[10] = {};  //  数组用来存放IO大小
    int fd = shm_open("posixsm", O_CREAT | O_RDWR, 0666);  // 共享内存
    ftruncate(fd, 0x400000); // 共享内存

    char *p = mmap(NULL, 0x400000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(p, 0, 0x400000);

    if (args < 2) {
        printf("the number of param is wrong\n");
        exit(1);
    }
    for(i=0; i<5; i++) {
        printf("mysrc, %s\n", mysrc[i]);
        /*if ((srcfd[i] = open(mysrc[i], O_RDWR)) < 0) {
            printf("open srcfile error\n");
            exit(1);
        }*/ 
        if ((srcfd[i] = open(mysrc[i], O_RDWR | O_DIRECT, 0644)) < 0) {
            close(srcfd[i]);
            printf("open odsfile error\n");
            exit(1);
        }
        if(ftruncate(srcfd[i], 6442450944) < 0) {    //  6 * 1024 * 1024 * 1024
            perror("ftruncate error");
            //return -1;
            exit(1);//初始化文件大小
        }
    }
    /*if ((srcfd = open(argv[1], O_RDWR)) < 0) {
        printf("open srcfile error\n");
        exit(1);
    }*/

    //printf("srcfd = %d\n", srcfd);

    /*lseek(srcfd, 0, SEEK_SET); //文件开头
    write(srcfd, "abcdefg", length);

    lseek(srcfd, 0, SEEK_SET);
    read(srcfd, content, length);

    printf("write in the srcfile successful, content is %s\n", content);*/

    /*if ((odsfd = open(argv[1], O_RDWR | O_DIRECT, 0644)) < 0) {
        close(odsfd);
        printf("open odsfile error\n");
        exit(1);
    }
    if(ftruncate(odsfd, 6442450944) < 0) {    //  6 * 1024 * 1024 * 1024
        perror("ftruncate error");
        //return -1;
        exit(1);//初始化文件大小
    }
    if ((srcfd = open(argv[1], O_RDWR | O_DIRECT, 0644)) < 0) {
        close(srcfd);
        printf("open odsfile error\n");
        exit(1);
    }*/
    /*if(ftruncate(srcfd, 6442450944) < 0) {    //  6 * 1024 * 1024 * 1024
        perror("ftruncate error");
        //return -1;
        exit(1);//初始化文件大小
    }*/
    //per = argv[2];
    if ((pidt = Fork()) == 0) {
        sleep(1);
        while(1) {
            for(i=0; i < atoi(argv[2]); i++) {
                if(p[i] == 1) {
                    printf("i:%d, b:%d\n", i, p[i]);
                }
            }
        }
        //exit(0);
    }
    for(k=0; k < atoi(argv[2]); k++) {
        if ((pid[k] = Fork()) == 0) {
            //生成随机数
            //srand((unsigned)time(NULL));
            a = rand()% 100;
            printf("a:%d\n", a);

            b = rand() % 4;
            printf("b:%d\n", b); //  随机读取哪个文件

            printf( "Reckon by time\n");  
            //start = clock();  // 获取IO时间
            gettimeofday(&start, NULL);
            memset(&myctx, 0, sizeof(myctx));
            io_queue_init(AIO_MAXIO, &myctx);

            struct iocb *io = (struct iocb *)malloc(sizeof(struct iocb));
            int iosize = AIO_BLKSIZE; // 1024 
            tmp = posix_memalign((void **)&buff, getpagesize(), AIO_BLKSIZE);
            if (tmp < 0) {
                printf("posix_memalign error\n");
                exit(1);
            }
            if (NULL == io) {
                printf("io out of memeory\n");
                exit(1);
            }
            if(a >= atoi(argv[1])) {
                printf("START...\n \n");
                //io_prep_pread(io, odsfd, buff, iosize, offset);
                io_prep_pread(io, srcfd[b], buff, iosize, offset);
                io_set_callback(io, rd_done);
                rc = io_submit(myctx, 1, &io);

                if (rc < 0) {
                    printf("io_submit read error\n");
                    exit(1);
                }
                printf("\nsubmit %d read request\n", rc);

            }
            //设置回调函数
            else {
                printf("START...\n \n");
                //io_prep_pwrite(io, odsfd, buff, iosize, offset);
                io_prep_pwrite(io, srcfd[b], buff, iosize, offset);
                //设置回调函数
                io_set_callback(io, wr_done);
                //io_set_callback(io, rd_done);
                 if (1 != (res = io_submit(myctx, 1, &io))) {
                    printf("io_submit write error \n");
                    exit(1);
                 }

                printf("\nsubmit %d write request \n", res);
            }
            //finish = clock();  
            //duration = (double)(finish - start) / CLOCKS_PER_SEC;
            //duration = (double)(finish - start);
            //printf( "%0.10f seconds\n", duration);
            /*printf("START...\n \n");

            rc = io_submit(myctx, 1, &io);

            if (rc < 0)
                printf("io_submit read error\n");

            printf("\nsubmit %d read request\n", rc);*/

            //m_io_queue_run(myctx);

            struct io_event events[AIO_MAXIO];
            io_callback_t cb;
            //sleep(60);
            num = io_getevents(myctx, 1, AIO_MAXIO, events, NULL);
            //while( n-- ) ;
            //finish = clock();  // 获取IO时间
            //duration = (double)(finish - start) / CLOCKS_PER_SEC;
            //duration = (double)(finish - start);
            //printf( "%f seconds\n", (double)start);
            //printf( "%f seconds\n", (double)finish);
            //printf( "%f seconds\n", duration);

            printf("\n %d io_request completed \n \n", num);

            gettimeofday(&end, NULL);
            timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
            printf("timeofday=%.3fs\n", (double)timeuse/1000000);
            p[k] = 1;
            printf("k:%d, b-t:%d\n", k, p[k]);
            for (i = 0; i < num; i++) {
                cb = (io_callback_t) events[i].data;
                struct iocb *io = events[i].obj;
                printf("events[%d].data = %x, res = %d, res2 = %d \n", i, cb, events[i].res, events[i].res2);
                cb(myctx, io, events[i].res, events[i].res2);
            }
            //b[i] = 1;
            //printf("k:%d, b-t:%d\n", i, b[i]);
            //k++;
            exit(0);
        }
    }

    /*if ((pidt = Fork()) == 0) {
        sleep(1);
        while(1) {
            for(i=0; i < 2000; i++) {
                if(p[i] == 1) {
                    printf("i:%d, b:%d\n", i, p[i]);
                }
            }
        }
        //exit(0);
    }*/

    while(j < atoi(argv[2])) {
        if ((retpid = waitpid(pid[j], &status, WNOHANG)) > 0) {
            printf("before j:%d\n", j);
            j++;
            printf("after j:%d\n", j);
            if (WIFEXITED(status)) {
                printf("child %d terminated normally with exit status=%d\n", retpid, WEXITSTATUS(status));
            }
            else {
                printf("child %d terminated abnormally\n", retpid);
            }
        }
        if ((retpid = waitpid(pid[j], &status, WNOHANG)) == 0) {
            //printf("wait wait wait \n");
        }
    }
    /*while ((retpid = waitpid(pid[i++], &status, WNOHANG)) > 0) { //非阻塞模式
        //b[k] = 1024;
        //k++;
        if (WIFEXITED(status)) {
            printf("child %d terminated normally with exit status=%d\n", retpid, WEXITSTATUS(status));
        }
        else
        {
            printf("child %d terminated abnormally\n", retpid);
        }
        
    }*/
    /* The only normal termination is if there are no more children */
    if (errno != ECHILD) {
        unix_error("waitpid error");
    }
    /*for(k; k>0;k--) {
        printf("a[]:%d", b[k]);
    }*/
    return 0;
}