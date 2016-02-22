#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "mpi.h"

#define ROOT 0
#define ALIVE 'X'
#define DEAD '.'

int toindex(int row, int col, int N)
{
    if (row < 0) {
        row = row + N;
    } else if (row >= N) {
        row = row - N;
    }
    if (col < 0) {
        col = col + N;
    } else if (col >= N) {
        col = col - N;
    }
    return row * N + col;
}

void printgrid(char* grid, FILE* f, int N)
{
    char* buf = (char*) malloc((N + 1) * sizeof(char));
    for (int i = 0; i < N; ++i) {
        strncpy(buf, grid + i * N, N);
        buf[N] = 0;
        fprintf(f, "%s\n", buf);
    }
    free(buf);
}

char is_alive(char* buf, int i, int j, int N)
{
    int current = N * i + j;
    int alive_count = 0;
    for (int di = -1; di <= 1; ++di) {
        for (int dj = -1; dj <= 1; ++dj) {
            if ((di != 0 || dj != 0) && buf[toindex(i + di, j + dj, N)] == ALIVE) {
                ++alive_count;
            }
        }
    }
    if (alive_count == 3 || (alive_count == 2 && buf[current] == ALIVE)) {
        return ALIVE;
    } else {
        return DEAD;
    }
}

void set_front(char* front, int next_front, int i, int j, int N)
{
    for (int di = -1; di <= 1; ++di) {
        for (int dj = -1; dj <= 1; ++dj) {
            front[toindex(i + di, j + dj, N)] = next_front;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Usage: %s N input_file iterations output_file\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]); // grid size
    int iterations = atoi(argv[3]);

    int rank, size, i;
    int local_size, local_max_size, appendix;
    int ch_s = sizeof(char);
    MPI_Status status_up[2], status_down[2];
    MPI_Request reqs_up[2], reqs_down[2];

    char* buf = NULL;

    setbuf(stdout, NULL);

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int s_r_counts[size];
    int displs[size];
    int up = (size + rank - 1) % size;
    int down = (rank + 1) % size;

    local_size = N * (N / size);
    appendix = N % size;
    local_max_size = appendix ? local_size + N : local_size;

    if (rank == ROOT) {
        if(N / size < 3) {
            fprintf(stderr, "Too small size of the array for distribution into %d parts\n", size);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        buf = (char*) malloc(N * N * ch_s);

        FILE* input = fopen(argv[2], "r");
        if (!input)
            MPI_Abort(MPI_COMM_WORLD, 1);
        for (int i = 0; i < N; ++i) {
            fscanf(input, "%s", buf + i * N);
        }
        fclose(input);

    }
    int d = 0;
    for (i = 0; i < size; ++i) {
        s_r_counts[i] = appendix > 0 ? local_size + N : local_size;
        displs[i] = d;
        d += s_r_counts[i];
        appendix--;
    }
    int work_size = s_r_counts[rank];
    MPI_Barrier(MPI_COMM_WORLD);

    char* local_grid = (char*) malloc((local_max_size + 2 * N) * ch_s);
    char* local_buf = (char*) malloc((local_max_size + 2 * N) * ch_s);
    char* local_front = (char*) malloc((local_max_size + 2 * N) * ch_s);
    memset(local_front, 1, local_max_size + 2 * N);

    MPI_Scatterv(buf, s_r_counts, displs, MPI_CHAR,
	        	local_grid + N * ch_s, local_max_size, MPI_CHAR,
        		ROOT, MPI_COMM_WORLD);

    for (int iter = 0; iter < iterations; ++iter) {
        //up line
        MPI_Isend(local_grid + N, N, MPI_CHAR, up, 0, MPI_COMM_WORLD, &reqs_up[0]);
        MPI_Irecv(local_grid + N + work_size, N, MPI_CHAR, down, 0, MPI_COMM_WORLD, &reqs_up[1]);

        //down line
        MPI_Isend(local_grid + work_size, N, MPI_CHAR, down, 1, MPI_COMM_WORLD, &reqs_down[0]);
        MPI_Irecv(local_grid, N, MPI_CHAR, up, 1, MPI_COMM_WORLD, &reqs_down[1]);

        int next_front = (iter + 1) % 2 + 1;
        for (int i = 2; i < work_size / N; ++i) {
            for (int j = 0; j < N; ++j) {
                int current = i * N + j;
                if (local_front[current] != 0) {
                        local_buf[current] = is_alive(local_grid, i, j, N);
                    if (local_buf[current] != local_grid[current]) {
                        set_front(local_front, next_front, i, j, N);
                    } else {
                        if (local_front[current] != next_front) {
                            local_front[current] = 0;
                        }
                    }
                }
            }
        }

        MPI_Waitall(2, reqs_up, status_up);
        MPI_Waitall(2, reqs_down, status_down);

        for (int i = 1; i < work_size / N + 1; i += work_size / N - 1) {
            for (int j = 0; j < N; ++j) {
                int current = i * N + j;
                    local_buf[current] = is_alive(local_grid, i, j, N);
                if (local_buf[current] != local_grid[current]) {
                    set_front(local_front, next_front, i, j, N);
                } else {
                    if (local_front[current] != next_front) {
                        local_front[current] = 0;
                    }
                }
            }
        }
        char* tmp = local_grid; local_grid = local_buf; local_buf = tmp;
    }

    MPI_Gatherv(local_grid + N, work_size, MPI_CHAR, buf,
                s_r_counts, displs, MPI_CHAR, ROOT, MPI_COMM_WORLD);

    if (rank == ROOT) {
        FILE* output = fopen(argv[4], "w");
        printgrid(buf, output, N);
        fclose(output);

        free(buf);
    }
    free(local_grid);
    free(local_buf);
    free(local_front);

    MPI_Finalize();

    return 0;
}
