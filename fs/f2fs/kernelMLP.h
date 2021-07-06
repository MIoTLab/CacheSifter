#ifndef KERNELMLP_KERNELMLP_H
#define KERNELMLP_KERNELMLP_H

#include <linux/slab.h>
#include <linux/printk.h>

typedef long long mat_t;
#define KERNEL_TIMES (1000000L)  // 不能写UL，因为强制类型转换，会将signed，转化为unsigned

#define HIGH_RECALL_CONFIG

struct matrix {
    mat_t **data;
    int rows; // row
    int cols; // col
};

struct mlp_model {
    struct matrix *w0;
    struct matrix *w1;
    struct matrix *w2;
    struct matrix *w3;

    struct matrix *b0;
    struct matrix *b1;
    struct matrix *b2;
    struct matrix *b3;

    struct matrix *scale;
    struct matrix *mean;
};

struct mlp_conf {
    char *w0_file;
    int m_w0, n_w0;
    char *w1_file;
    int m_w1, n_w1;
    char *w2_file;
    int m_w2, n_w2;
    char *w3_file;
    int m_w3, n_w3;

    char *b0_file;
    int m_b0, n_b0;
    char *b1_file;
    int m_b1, n_b1;
    char *b2_file;
    int m_b2, n_b2;
    char *b3_file;
    int m_b3, n_b3;

    int m_scale, n_scale;
    int m_mean, n_mean;
};

static inline void *my_zmalloc(int num, unsigned long long size)
{
    return kmalloc(num * size, GFP_KERNEL | __GFP_ZERO);
}

static inline void my_free(void *ptr)
{
    kfree(ptr);
}

static inline void print_matrix(struct matrix *mat)
{
    int i, j;
    for(i = 0; i < mat->rows; i++) {
        printk("[%d]", i);
        for(j = 0; j < mat->cols; j++) {
            printk("%lld ", mat->data[i][j]);
        }
        printk("\n");
    }
}

static inline mat_t factorial(int x){
    mat_t factorial[] = {
            1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600, 6227020800,
            87178291200, 1307674368000, 20922789888000, 355687428096000, 6402373705728000, 121645100408832000, 2432902008176640000
    };
    return factorial[x];
};

void init_mlp(struct mlp_model *mlp_model, struct mlp_conf conf);
void init_mlp2(struct mlp_model *mlp_model, struct mlp_conf conf);
void free_mlp(struct mlp_model *mlp_model);
struct matrix *mlp_transform(struct mlp_model *model, struct matrix *input_mat);
struct matrix *mlp_normalize(struct mlp_model *model, struct matrix *input);

struct matrix *matrix_alloc(int m, int n);
void matrix_release(struct matrix *mat);
struct matrix *matrix_normalize(struct matrix *mat, int axis);

extern const long long w0[][26];
extern const long long w1[][512];
extern const long long w2[][200];
extern const long long w3[][2];
extern const long long b0[];
extern const long long b1[];
extern const long long b2[];
extern const long long b3[];
extern const long long mean[];
extern const long long scale[];

extern const long long w0_80[][86];
extern const long long w1_80[][512];
extern const long long w2_80[][200];
extern const long long w3_80[][2];
extern const long long b0_80[];
extern const long long b1_80[];
extern const long long b2_80[];
extern const long long b3_80[];
extern const long long mean_80[];
extern const long long scale_80[];


#endif //KERNELMLP_KERNELMLP_H
