#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <libaio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

int srcfd = -1;
int odsfd = -1;

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

    
    tmp = posix_memalign((void **)&wrbuff, getpagesize(), AIO_BLKSIZE);
    if (tmp < 0) {
        printf("posix_memalign222 \n");
        exit(1);
    }

    snprintf(wrbuff, iosize + 1, "%s", buf);

    printf("wrbuff - len = %d:%s \n", strlen(wrbuff), wrbuff);
    printf("wrbuff_len = %d \n", strlen(wrbuff));
    free(buf);

    //io_prep_pwrite(iocb, odsfd, wrbuff, iosize, offset);
    //设置回调函数
    //io_set_callback(iocb, wr_done);

    if (1 != (res = io_submit(ctx, 1, &iocb)))
        printf("io_submit write error \n");

    printf("\nsubmit %d write request \n", res);
}

void main(int args, void *argv[])
{
    pid_t pid;
    pid_t retpid;
    int length = sizeof("abcdefg");
    char *content = (char *)malloc(length);
    io_context_t myctx;
    int rc;
    char *buff = NULL;
    int offset = 0;
    int num, i, tmp; // 变量i是创建多少个子系统
    int status;
    int a;
    int per;

    if (args <= 3) {
        printf("the number of param is wrong\n");
        exit(1);
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

    if ((odsfd = open(argv[1], O_RDWR | O_DIRECT, 0644)) < 0) {
        close(srcfd);
        printf("open odsfile error\n");
        exit(1);
    }
    if(ftruncate(odsfd, 6442450944) < 0) {    //  6 * 1024 * 1024 * 1024 
        perror("ftruncate error");
        //return -1; 
        exit(1);//初始化文件大小
    }
    //per = argv[2];
    for(i=0; i < atoi(argv[3]); i++) {
        if ((pid = Fork()) == 0) {
            //生成随机数
            //srand((unsigned)time(NULL));
            a = rand() % 100;
            printf("a:%d\n", a);
        
            memset(&myctx, 0, sizeof(myctx));
            io_queue_init(AIO_MAXIO, &myctx);

            struct iocb *io = (struct iocb *)malloc(sizeof(struct iocb));
            int iosize = AIO_BLKSIZE;
            tmp = posix_memalign((void **)&buff, getpagesize(), AIO_BLKSIZE);
            if (tmp < 0) {
                printf("posix_memalign error\n");
                exit(1);
            }
            if (NULL == io) {
                printf("io out of memeory\n");
                exit(1);
            }
            if(a >= atoi(argv[2])) {
                io_prep_pread(io, srcfd, buff, iosize, offset);
                io_set_callback(io, rd_done);
            }
            //设置回调函数
            else {
                io_prep_pwrite(io, odsfd, buff, iosize, offset);
                //设置回调函数
                io_set_callback(io, wr_done);
                //io_set_callback(io, rd_done);
            }

            printf("START...\n \n");

            rc = io_submit(myctx, 1, &io);

            if (rc < 0)
                printf("io_submit read error\n");

            printf("\nsubmit %d read request\n", rc);

            //m_io_queue_run(myctx);

            struct io_event events[AIO_MAXIO];
            io_callback_t cb;

            num = io_getevents(myctx, 1, AIO_MAXIO, events, NULL);
            printf("\n %d io_request completed \n \n", num);
            for (i = 0; i < num; i++) {
                cb = (io_callback_t) events[i].data;
                struct iocb *io = events[i].obj;
                printf("events[%d].data = %x, res = %d, res2 = %d \n", i, cb, events[i].res, events[i].res2);
                cb(myctx, io, events[i].res, events[i].res2);
            }
        }
    }

    while ((retpid = waitpid(pid, &status, WNOHANG)) == 0) { //非阻塞模式
        if (WIFEXITED(status)) {
            printf("child %d terminated normally with exit status=%d\n", retpid, WEXITSTATUS(status));
            break;
        }
        else
        {
            printf("child %d terminated abnormally\n", retpid);
        }
        
    }
    /* The only normal termination is if there are no more children */
    if (errno != ECHILD) {
        unix_error("waitpid error");
    }
}