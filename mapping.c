// 入度排序重排顶点

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define VERTEX 512
#define EDGE 2048
#define DATA_LEN 1024

char *filename="data/graph-256-clean.txt";

// 定义顶点的结构体
typedef struct{
    int ori_id;
    int aft_id;
    int In_Degree;
}Vertex;

Vertex *vertex[VERTEX]; // 顶点结构体
bool is_visited[VERTEX]; // 访问数组
int edge[EDGE][2]={0}; // 边集
int num_v, num_e; 

void in_degree_sort(){
    Vertex *temp_vertex;
    int i, j;

    // 初始化顶点数组
    for(i = 0; i < VERTEX; i++){
        vertex[i] = (Vertex*)malloc(sizeof(Vertex)); // 分配空间
        vertex[i]->ori_id = -1;
        vertex[i]->In_Degree = 0;
    }

    // 计算入度
    for(i = 0; i < num_e; i++){
        int start_edge = edge[i][0];
        int end_edge = edge[i][1];
        bool is_used = false;

        for(j = 0; j < num_v; j++) {
            if(vertex[j]->ori_id == end_edge){
                vertex[j]->In_Degree++;
                is_used = true;
                break;
            }
        }

        if(is_used == false){
            vertex[num_v]->ori_id = end_edge;
            vertex[num_v]->In_Degree = 1;
            num_v++;
            is_visited[end_edge] = true;
        }
    }

    // 使用冒泡排序对顶点数组进行排序
    for(i = 0; i < num_v - 1; i++) {
        for(j = 0; j < num_v - i - 1; j++) {
            if(vertex[j]->In_Degree < vertex[j + 1]->In_Degree) {
                temp_vertex = vertex[j];
                vertex[j] = vertex[j + 1];
                vertex[j + 1] = temp_vertex;
            }
        }
    }

    // 更新aft_id
    for (i = 0; i < num_v; i++){
        vertex[i]->aft_id = i;
    }

    // 添加未访问的顶点
    for(i = 0; i < num_e; i++){
        if(!is_visited[edge[i][0]]) {
            vertex[num_v]->ori_id = edge[i][0];
            vertex[num_v]->aft_id = num_v;
            num_v++;
        }
        if(!is_visited[edge[i][1]]){
            vertex[num_v]->ori_id = edge[i][1];
            vertex[num_v]->aft_id = num_v;
            num_v++;
        }
    }
} 

int main(int argc, char* argv[]){
    FILE *fd = fopen(filename, "r");
    char data[DATA_LEN];

    // 检查文件是否打开成功
    if(fd == NULL){
        printf("文件打开失败！");
        exit(1);
    }

    // char *token;
    unsigned int start, end = 0;
    while(fgets(data, 1024, fd)){
        // 从data中解析两个无符号整数，并将它们存储在start和end中
        sscanf(data, "%u %u", &start, &end);
        edge[num_e][0] = start;
        edge[num_e][1] = end;
        num_e++;
    }
    fclose(fd);
    in_degree_sort();

    FILE *fd1 = fopen("sort.map", "w");

    // 检查文件是否打开成功
    if(fd1 ==  NULL){
        printf("文件打开失败！");
        exit(1);
    }

    for(int i = 0; i < num_v; i++){
        fprintf(fd1, "%u %u\n", vertex[i]->ori_id, vertex[i]->aft_id);
    }
    fclose(fd1);
    return 0;  
}