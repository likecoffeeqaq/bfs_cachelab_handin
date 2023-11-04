#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define V 512
#define E V*8

/* Markers used to bound trace regions of interest */
volatile char MARKER_START, MARKER_END;

static int csr_edges[E];     /* edge array */
static int csr_index[V + 1]; /* edge index */

static bool visited[V];
static int frontier[V];

int max_v_index=0;

typedef struct {
    int src;
    int dst;
} edge;

edge edges[E];
int v_map[V];

void load_map(char *map_fn);
void load_csr(char *csr_fn);
void bfs(int source);
void display();


int main(int argc, char* argv[]) {
    int i, source = 0;

    for (i = 0; i < V; i++) {
        visited[i] = false;
        frontier[i] = -1;
        csr_index[i] = -1;
    }
    csr_index[i] = -1;

    load_map("sort.map");
    /****************修改路径*******************/
    load_csr("./data/graph-256-clean.txt");
    /******************************************/


    /* Record marker addresses */
    FILE* marker_fp = fopen(".marker", "w");
    assert(marker_fp);
    fprintf(marker_fp, "%llx %llx", 
            (unsigned long long int) &MARKER_START,
            (unsigned long long int) &MARKER_END );
    fclose(marker_fp);

    MARKER_START = 33; /* access marker to generate a trace record */
    bfs(source);
    MARKER_END = 34; /* access marker again */

    /* validate result */
    FILE* result_fp = fopen(".bfs_result", "w");
    for (i = 0; i < V; i++ ) {
        if (frontier[i] != -1) {
            fprintf(result_fp, "%d\n", frontier[i]);
        }
    }
    fclose(result_fp);
    return 0; 
} /*End of main()*/

/*
use script to sort, remap and dedup edge list file load preprocessed graphs for evaluation
add reverse edge to prepare a bi-directional graph for undirectional traversal load pre-processed edges into csr directly

E.g.:

    grep -v ^# facebook_combined.txt | awk '{print $1" "$2"\n"$2" "$1}' | sort -n -k1 -k2 | uniq

*/
void load_map(char *map_fn) {
    char buf[1000];
    FILE* map_fp = fopen(map_fn, "r");

    if(!map_fp){
        fprintf(stderr, "%s: %s\n", map_fn, strerror(errno));
        exit(1);
    }

    while(fgets(buf, 1000, map_fp)) {
        unsigned int from, to;
        sscanf(buf, "%u %u", &from, &to);
        v_map[from] = to;
    }

    fclose(map_fp);
    return;
}

int cmp(const void* a, const void* b) {
    edge ia = *(edge*)a, ib = *(edge*)b;
    return ia.src - ib.src;
}

void load_csr(char *csr_fn) {
    char buf[1000];
    unsigned int num_v = 0, num_e = 0;
    int v_from, v_to, v_prev = 0;
    FILE* csr_fp = fopen(csr_fn, "r");

    if(!csr_fp){
        fprintf(stderr, "%s: %s\n", csr_fn, strerror(errno));
        exit(1);
    }

    while(fgets(buf, 1000, csr_fp) != NULL) {
        sscanf(buf, "%u %u", &v_from, &v_to);
        edges[num_e].src = v_map[v_from];
        edges[num_e].dst = v_map[v_to];
        // edges[num_e].src = v_from;
        // edges[num_e].dst = v_to;
        num_e++;
    }
    qsort(edges, num_e, sizeof(edge), cmp);
    for (int i = 0; i < num_e; i++) {
        printf("%d %d\n", edges[i].src, edges[i].dst);
    }

    csr_index[0] = 0;
    for (int i = 0; i < num_e; i++) {
        v_from = edges[i].src;
        v_to = edges[i].dst;
        if (v_from > v_prev) {
            num_v++;
            for (int j=(v_prev+1); j<=v_from; j++) csr_index[j] = num_e;
            v_prev = v_from;
        }
        csr_edges[i] = v_to;
    }
 
    csr_index[v_from+1]=num_e;

    max_v_index=v_from;
    fclose(csr_fp);
} /*End of load_csr()*/

void bfs(int source) {
    int v = source, n;
    long long int i, front = 0, rear = 0;
    visited[v] = true;
    frontier[rear] = v;
    while (front <= rear) {
        v = frontier[front]; /* delete from queue */
        front++;
        for (i = csr_index[v]; i < csr_index[v + 1]; i++) {
            /* Check for adjacent unvisited nodes */
            n = csr_edges[i];
            if (visited[n] == false) {
                visited[n] = true;
                rear++;
                frontier[rear] = n;
            }   
        }
    }
} /*End of bfs()*/