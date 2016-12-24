//
//  lsxsh.c
//
//
//  Created by Lane on 2016/12/11.
//
// pwd, cd,
//
// ls -l, cp, mkdir -p,
//
// rm,
//
// exit


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>

#define BUFFERSIZE 4096
#define COPYMODE 0644
#define NUM 1024
#define MAXSIZE 100
char parsecmd(char *); // 几个内置命令
char *getusername(char buffer[NUM]); // 系统提示符

/* cd 内部命令 */
void shell_command_cd(char *command);

/* ls 内部命令 */
void shell_command_ls(char direname[], int lflag);
void dostat(char *filename);
char *uid_to_name(uid_t uid);
char *gid_to_name(gid_t gid);
void mode_to_letters(int mode, char str[]);
void show_file_info(char *filename, struct stat *info_p);

/* cp 内部命令 */
void shell_command_cp(char *path1, char *path2);
void oops(char *s1, char *s2);

/* mkdir 内部命令  */
void shell_command_mkdir(char *path);

int main(int argc, char const *argv[])
{
    char command[NUM];
    char *system_prompt, buffer[NUM], arguments[MAXSIZE];
    char tmp[MAXSIZE];
    char b = 0;

    // printf("Line: %d Message: %d %s \n", __LINE__, argc, argv);

    while (1)
    {
        system_prompt = getusername(buffer);// 系统提示符
        printf("%s", system_prompt);

        fgets(command, 1000, stdin); // 获取命令, 长 1000 字符
        command[strlen(command) - 1] = 0; // 去除回车

        b = parsecmd(command);
        switch (b) {
            case 1: /* pwd */
                getcwd(tmp, sizeof(tmp));
                printf("%s\n", tmp);
                break;
            case 2: /* cd */
                shell_command_cd(command);
                break;
            case 3: /* ls */
                strncpy(arguments, command+3, 2);
                if (0 == strcmp("", arguments))
                {
                    shell_command_ls(".", 0);
                    printf("Line:%d, arguments:[%s]\n", __LINE__, arguments);
                }
                else if (0 == strcmp("-l", arguments)) shell_command_ls(".", 1);
                break;
            case 4: /* cp */
            case 5: /* mkdir -p */
                // mkdir(command + 6, 0755);
                // for debug
                // printf("Line:%d, %s\n", __LINE__, command);
                strncpy(arguments, command+9, 10);
                // printf("Line:%d, command:%s\n", __LINE__, command);
                // printf("Line:%d, arguments:[%s]\n", __LINE__, arguments);
                shell_command_mkdir(arguments);
                break;
            case 6: /* rm */
                remove(command + 3);
                break;
            case 7: /* pid */
                printf("%d\n", getpid());
                break;
            case 8: /* mv */
            case 9: /* exit */
                 exit(0);
            case 0:
                printf("Sorry, command didn't find!\n");
                break;
            default:break;
        }

    }
    return 0;
}

char parsecmd(char * s)
{
    if (!strncasecmp(s, "pwd", 3))
    {
        return 1;
    }
    else if (!strncasecmp(s, "cd", 2))
    {
        return 2;
    }
    else if (!strncasecmp(s, "ls", 2))
    {
        return 3;
    }
    else if (!strncasecmp(s, "cp", 2))
    {
        return 4;
    }
    else if (!strncasecmp(s, "mkdir -p", 8))
    {
        return 5;
    }
    else if (!strncasecmp(s, "rm", 2))
    {
        return 6;
    }
    else if (!strncasecmp(s, "pid", 3))
    {
        return 7;
    }
    else if (!strncasecmp(s, "mv", 2))
    {
        return 8;
    }
    else if (!strcasecmp(s, "exit"))
    {
        return 9;
    }
    else return 0;
}

// mark -- cd
void shell_command_cd(char *command)
{
    if(chdir(command + 3) != 0) {
        printf("chdir(%s) error!%s\n", command + 3, strerror(errno));
    }
}

// mark -- ls-begin
void shell_command_ls(char dirname[], int lflag)
{
    DIR* dir_ptr;
    struct dirent* direntp;

    if ((dir_ptr = opendir(dirname)) == NULL)
    {
        fprintf(stderr, "ls2: cannot open %s \n", dirname);
    }
    else
    {
        while ((direntp = readdir(dir_ptr)) != NULL)
        {
            if (lflag) dostat(direntp->d_name);
            else printf("%s\t", direntp->d_name);
        }
        closedir(dir_ptr);
        printf("\n");
    }
}

void dostat(char *filename)
{
    struct stat info;

    if (stat(filename, &info) == -1)
    {
        perror(filename);
    }
    else
    {
        show_file_info(filename, &info);
    }
}

void show_file_info(char *filename, struct stat *info_p)
{
    char modestr[11];
    mode_to_letters(info_p->st_mode, modestr);
    printf("%s", modestr);
    printf(" %4d", (int) info_p->st_nlink);
    printf(" %-8s", uid_to_name(info_p->st_uid));
    printf(" %-8s", gid_to_name(info_p->st_gid));
    printf(" %8ld", (long) info_p->st_size);
    printf(" %.12s", 4 + ctime(&info_p->st_mtime));
    printf(" %s\t", filename);
    printf("\n");
}

void mode_to_letters(int mode, char str[])
{
    strcpy(str, "----------");

    if (S_ISDIR(mode)) str[0] = 'd';
    if (S_ISCHR(mode)) str[0] = 'c';
    if (S_ISBLK(mode)) str[0] = 'b';

    if ((mode & S_IRUSR)) str[1] = 'r';
    if ((mode & S_IWUSR)) str[2] = 'w';
    if ((mode & S_IXUSR)) str[3] = 'x';
    if ((mode & S_IRGRP)) str[4] = 'r';
    if ((mode & S_IWGRP)) str[5] = 'w';
    if ((mode & S_IXGRP)) str[6] = 'x';
    if ((mode & S_IROTH)) str[7] = 'r';
    if ((mode & S_IWOTH)) str[8] = 'w';
    if ((mode & S_IXOTH)) str[9] = 'x';
}

char* uid_to_name(uid_t uid)
{
    struct passwd *getpwuid(), *pw_ptr;
    static char numstr[10];

    if((pw_ptr = getpwuid(uid)) == NULL)
    {
        sprintf(numstr,"%d",uid);

        return numstr;
    }
    else
    {
        return pw_ptr->pw_name;
    }
}

char* gid_to_name(gid_t gid)
{
    struct group *getgrgid(), *grp_ptr;
    static char numstr[10];

    if(( grp_ptr = getgrgid(gid)) == NULL)
    {
        sprintf(numstr,"%d",gid);
        return numstr;
    }
    else
    {
        return grp_ptr->gr_name;
    }
}
// mark -- ls-end

// mark -- cp
void shell_command_cp(char *path1, char *path2)
{
    int in_fd, out_fd, n_chars;
    char buf[BUFFERSIZE];
    if ((in_fd = open(path1, O_RDONLY)) == -1)
        oops("Cannot open", path1);
    if ((out_fd = open(path2, O_WRONLY | O_CREAT, COPYMODE)) == -1)
        oops("Cannot create",path2);
    //开始读取
    while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
        if (write(out_fd, buf, n_chars) != n_chars)
            oops("Write error to",path2);
    //判断最后是否写入
    if (n_chars == -1)
        oops("Read error from,path1", "");
    //判断最后是否写入
    if (n_chars == -1)
        oops("Read error from,path1", "");
    //关闭文件
    if  (close(in_fd) == -1 || close(out_fd) == -1)
        oops("Error closing files", "");
}

void oops(char *s1, char *s2)
{
    fprintf(stderr, "Error:%s ", s1);
    perror(s2);
    exit(1);
}

// mark -- mkdir
void shell_command_mkdir(char *path)
{
    int i,len;
    char str[512];
    strncpy(str, path, 512);
    len=strlen(str);
    for( i=0; i<len; i++ )
    {
        if( str[i]=='/' )
        {
            str[i] = '\0';
            if( access(str,0)!=0 )
            {
                mkdir( str, 0777 );
            }
            str[i]='/';
        }
    }
    if( len>0 && access(str,0)!=0 )
    {
        mkdir( str, 0777 );
    }
    return;
}

char *getusername(char buffer[NUM])
{
    char *username;
    char *ptr;      // 路径
    char *p;        // 主机
    char buf1[NUM]; // 主机全名
    char buf2[NUM]; // 路径

    getwd(buf2);    // 对命令提示的路径获得
    strcpy(buffer, "[");
    username = getenv("USER");
    strcat(buffer, username);
    strcat(buffer, "@");
    gethostname(buf1, sizeof buf1);     // 获取主机全名
    p = strtok(buf1, ".");              // 对主机进行截取
    strcat(buffer, p);

    ptr = strrchr(buf2, '/');           // 字符串的从后往前进行截取的函数，讲多得shell命令提示中的路径
    ptr = strtok(ptr, "/");

    // 把获得的 [用户名@主机名: 路径] 连接在一起
    strcat(buffer, ": ");
    strcat(buffer, ptr);
    strcat(buffer, "]$ ");
    return buffer;
}
