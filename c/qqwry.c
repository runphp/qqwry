/**
 * 重新写了stringIpToLong函数，改用sscanf，增加了判断ip地址错误
 *
 *
 * @author runphp.net
 * @version 0.8
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
 
#include "qqwry.h"
#include "localIp.h"
 
extern FILE *ipFile;
extern unsigned long ipBegin, ipEnd, ipSum;
 
/**
 * 把ip字符串转换为unsigned long
 * @param char *ipString
 * @return unsigned long ret
 */
unsigned long stringIpToLong(char *ipString)
{
    int num[4];
    char ch =0;
    unsigned long ret = 0;
    if(sscanf(ipString, "%3d.%3d.%3d.%3d%c",&num[0], &num[1], &num[2], &num[3], &ch) != 4
          //  || ch != 0  //sscanf返回 !=4包括了ch不为0
            || num[0] > 0xFF 
            || num[1] > 0xFF
            || num[2] > 0xFF
            || num[3] > 0xFF
            || num[0] < 0x00 
            || num[1] < 0x00
            || num[2] < 0x00
            || num[3] < 0x00){
        puts("ip地址错误");
        exit(0);
    }
 
    ret = ret | ( num[0] << 24 ) | ( num[1] << 16 ) | ( num[2] << 8 ) | ( num[3] );
    /*  
        char tmp[16];
        unsigned long ret = 0;
        strcpy(tmp, ipString);
        char *str = strtok(tmp, ".");
        ret = ret | (atol(str) << 24);
        str = strtok(NULL, ".");
        ret = ret | (atol(str) << 16);
        str = strtok(NULL, ".");
        ret = ret | (atol(str) << 8);
        str = strtok(NULL, ".");
        ret = ret | (atol(str));
        */
    return ret;
}
 
/**
 * 返回ip记录所在位置,使用二分法
 * @param char *ipString
 * @return unsigned long 
 */
unsigned long seekIpBinarySearch(char *ipString)
{
    unsigned long ipNum = stringIpToLong(ipString);
    unsigned long start = 0L;
    unsigned long end = ipSum;
    unsigned long mid = 0L;
    unsigned long addr[2] = {0L};
 
    while(start != end) {
        mid = (start + end) / 2;
        fseek(ipFile, ipBegin + mid * RECORD_LENGTH, SEEK_SET);
        fread(addr, 1, 7, ipFile);
        if(ipNum == addr[0]) {
            return addr[1];
        } else if(end == start +1) {
            return addr[1];
        } else if(ipNum > addr[0]){
            start = mid;
        } else/* if(ipNum > addr[0])*/{
            end = mid;
        }
    }
 
}
 
/**
 * 从文件offset位置读取字符串到string，直到遇到空字符
 * @param char *string 
 * @param unsigned long offset
 * @return void
 */
void readString(char *string, unsigned long offset)
{
    fseek(ipFile, offset, SEEK_SET);
    while(*string++ = fgetc(ipFile))
        ;
}
 
/**
 * 从文件当前位置读取字符串到string，直到遇到空字符
 * @param char *string 
 * @param unsigned long offset
 * @return void
 */
void readStringCur(char *string)
{
    while(*string++ = fgetc(ipFile))
        ;
}
/**
 * 从文件offset位置读取国家地区到string
 * @param char *string 
 * @param unsigned long offset
 * @return void
 */
void seekCountryArea(char *string, unsigned long offset)
{
    fseek(ipFile, offset+4, SEEK_SET);
    char flag = fgetc(ipFile);
    unsigned long countryOffset[1] = {0L};
    if (AREA_FOLLOWED == flag) {        
        fread(countryOffset, 1, 3, ipFile);
        fseek(ipFile, countryOffset[0], SEEK_SET);
        flag = fgetc(ipFile);
        if(NO_AREA == flag) {
            unsigned long tmp = countryOffset[0];
            fread(countryOffset, 1, 3, ipFile);
            fseek(ipFile, countryOffset[0], SEEK_SET);
            readStringCur(string);
            string = strcat(string, " ");
            fseek(ipFile, tmp+4, SEEK_SET);
        } else {
            fseek(ipFile, -1, SEEK_CUR);
            readStringCur(string);
            string = strcat(string, " ");
 
        }
 
    } else if (NO_AREA == flag){
        fread(countryOffset, 1, 3, ipFile);
        readString(string, countryOffset[0]);
        string = strcat(string, " ");
        fseek(ipFile, offset+8, SEEK_SET);
    } else {
        readString(string, offset+4);
        string = strcat(string, " ");
    }
    readArea(string);
}
 
/**
 * 读取地区字符串到string后面，具体是加在国家字符串后面
 * @param char *string
 * @return void
 */
void readArea(char *string)
{
 
    unsigned long offset[1] = {0L};
    char area[255] = "";
    int  flag = fgetc(ipFile);
    if (AREA_FOLLOWED == flag || NO_AREA == flag) {
        fread(offset, 1, 3, ipFile);
        readString(area, offset[0]);
        string = strcat(string, area);
    } else {
        fseek(ipFile, -1, SEEK_CUR);
        readStringCur(area);
        string = strcat(string, area);
    }
}
 
/**
 * 检查命令行输入参数
 */
void checkCmdLine(int argc, char **argv){
    if(argc == 1) {
        //数据库信息
        unsigned long ll = seekIpBinarySearch("255.255.255.0");
        char string[255]="";
        seekCountryArea(string,ll);
        printf("%s %lu条\n", string, ipSum);
 
        //输出你的ip地址
        localIp(string);
        printf("\n你的IP：%s\n",string);
        ll = seekIpBinarySearch(string);
        seekCountryArea(string,ll);
        printf("IP地址：%s\n",string);
 
        //帮助
        puts("----------------------------------------");
        puts("查询ip地址");
        printf("\n用法：\%s ip地址\n例如：%s 127.0.0.1\n", argv[0], argv[0]);
 
        puts("\n解压数据库");
        printf("\n用法：\%s -o 文件名\n例如：%s -o qqwry.txt\n", argv[0], argv[0]);
 
        puts("\n搜索字符串");
        printf("\n用法：\%s -s 字符串\n例如：%s -s 浏阳\n", argv[0], argv[0]);
        puts("----------------------------------------");
        exit(1);
    //搜索ip记录
    }else if(argc==2){
        puts(argv[1]);
        char string[255]="";
        unsigned long ll = seekIpBinarySearch(argv[1]);//二分法搜索ip记录位置
        seekCountryArea(string, ll);//从ll位置读取ip记录信息
        puts(string);  
    }else if(argc == 3 ){
        //导出记录
        if(strcmp(argv[1],"-o")==0){
            puts("准备导出");
            //文件已经存在(，还需要判断是否可写，不然崩溃)
            if(access(argv[2],F_OK) == 0){
               printf("是否覆盖%s(Y/N)",argv[2]);
               char ch = 0;
               while( (ch = tolower(getchar())) != 'y' && ch !='n') {
                   printf("是否覆盖%s(Y/N)",argv[2]);
               }
               if(ch =='n')
                   exit(1);
            }           
            exportData(argv[2]);
        }else if(strcmp(argv[1],"-s")==0){
            searchIpByName(argv[2]);
        }else{
            puts("有任何问题请联系：runphp@qq.com");
        }
    //其他
    }else{
        puts("有任何问题请联系：runphp@qq.com");
    }
}
 
/**
 * 从当前位置读取4个字节转换为ip字符串
 * @param char *string 保存ip字符串
 */
void readIpString(char *string)
{
// if(sizeof(string) < 16)
//     realloc(string, 16);
sprintf(string, "%d.%d.%d.%d", fgetc(ipFile), fgetc(ipFile), fgetc(ipFile), fgetc(ipFile));
//  free(string);
}
 
/**
 * 导出记录
 * @param char *filename 文件名
 *
 */
void exportData(char *fileName)
{
    FILE *fp = fopen(fileName, "wb");
    unsigned long i=0;
    unsigned long data[1]={0};
    char string[255]="";
    unsigned long di = (ipSum/100);
    int j=0;
    while(!feof(ipFile) && i != ipSum){
        //起始ip
        fseek(ipFile, ipBegin + RECORD_LENGTH * i, SEEK_SET);
        readIpString(string);
        fprintf(fp, "%-16s", string);
        //结束ip
        fread(data, 1, 3, ipFile);
        fseek(ipFile, data[0], SEEK_SET);
        readIpString(string);
        fprintf(fp, "%-16s", string);
        //国家地区
        seekCountryArea(string, data[0]);
        fputs(string, fp);
        fputs("\r\n",fp); // \r\n for windows
        //导出进度
        if(i%di==0){
            if(j>9)
                putchar('\b');
            putchar('\b');
            putchar('\b');
            if(j%2==0)
                putchar('#');
            printf("%d%%",j++);
        }
        ++i;
    }   
    puts("\n导出完毕");
}
 
/**
 * 搜索国家地区中包含某字符串的ip段
 * @param char *name 要搜索的字符串
 */
void searchIpByName(char *name)
{
    unsigned long i=0;
    unsigned long data[1]={0};
    char string[255]="";
    unsigned long j=0;
    int k=0;
    while(!feof(ipFile) && i != ipSum){
 
        fseek(ipFile, ipBegin + RECORD_LENGTH * i +4, SEEK_SET);
        fread(data, 1, 3, ipFile);
        seekCountryArea(string, data[0]);
        if(strstr(string, name) != NULL) {
            while(k>0){
                putchar('\b');
                --k;
            }
            char ipStr[16]="";
            fseek(ipFile, ipBegin + RECORD_LENGTH * i, SEEK_SET);
            readIpString(ipStr);
            printf("%-16s", ipStr);
 
            fread(data, 1, 3, ipFile);
            fseek(ipFile, data[0], SEEK_SET);
            readIpString(ipStr);
            printf("%-16s", ipStr);
             
            puts(string);
            ++j;
        }else{
            //等待
             if(i%(ipSum/50)==0){
                 putchar('>');
                 ++k;
             }
        }
        ++i;
    }
     
    while(k>0){
        putchar('\b');
        --k;
    }
    printf("总共搜索到 %lu 条记录。                              ",j);
}