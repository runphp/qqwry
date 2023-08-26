/**
 *
 * 增加了解压数据库功能，包括
 * void readIpString(char *string);
 * void exportData(char *fileName);
 *
 * @author runphp.net
 * @version 0.6
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qqwry.h"
 
FILE *ipFile         = NULL; //ip文件指针
unsigned long ipBegin  = 0; //ip开始地址
unsigned long ipEnd    = 0; //ip结束地址
unsigned long ipSum    = 0; //ip总数
/**
 * main()
 */
int main(int argc, char **argv)
{
    ipFile = fopen("../qqwry.dat", "rb");
    if(!ipFile){
        puts("../qqwry.dat文件丢失");
        exit(1);
    }
    unsigned long data[2];
    fread(data, 1, 8, ipFile);   
    ipBegin = data[0];
    ipEnd = data[1];
    ipSum = ((data[1]-data[0])/RECORD_LENGTH +1);
 
    checkCmdLine(argc, argv);//处理命令行参数
 
    fclose(ipFile);
    return 0;
}