#include <linux/kernel.h>
#include "kernelMLP.h"

// 分配矩阵空间
struct matrix *matrix_alloc(int m, int n)
{
    struct matrix *mat;
    int i, j;

    mat = (struct matrix *)my_zmalloc(1, sizeof(struct matrix));
    if(!mat) {
        mat = NULL;
        goto out;
    }

    mat->data = (mat_t **)my_zmalloc(m, sizeof(mat_t *));
    if(!mat->data)
        goto free_mat;

    for(i = 0; i < m; i++) {
        mat->data[i] = (mat_t *)my_zmalloc(n, sizeof(mat_t));
        if(!mat->data[i])
            goto free_data;
    }
    mat->rows = m;
    mat->cols = n;
out:
    return mat;
free_data:
    for(j = 0; j < i; j++){
        if(mat->data[j])
            my_free(mat->data[j]);
    }
    my_free(mat->data);
free_mat:
    my_free(mat);
    return NULL;
}

// 释放空间
void matrix_release(struct matrix *mat)
{
    int i;
    for(i = 0; i < mat->rows; i++) {
        my_free(mat->data[i]);
    }
    my_free(mat->data);
    my_free(mat);
}

// 给整型数据的矩阵乘法
struct matrix *matrix_int_mul(struct matrix *mat1, struct matrix *mat2)
{
    int i, j, k;
    mat_t t;
    struct matrix *ret;

    if(mat1->cols != mat2->rows)
        return NULL;

    ret = matrix_alloc(mat1->rows, mat2->cols);
    if(!ret)
        return NULL;

    for(i = 0; i < ret->rows; i++) {
        for(k = 0; k < mat1->cols; k++) {
            t = mat1->data[i][k];
            for(j = 0; j < ret->cols; j++)
                ret->data[i][j] += t * mat2->data[k][j] / KERNEL_TIMES; // 除以KERNEL_TIMES可以使输出恰好为原数据的100万倍
        }
    }
    return ret;
}

// 矩阵转置
struct matrix *matrix_transpos(struct matrix *mat)
{
    int i, j;
    struct matrix *t_mat = matrix_alloc(mat->cols, mat->rows);
    if(!t_mat)
        return NULL;

    for(i = 0; i < mat->rows; i++){
        for(j = 0; j < mat->cols; j++){
            t_mat->data[j][i] = mat->data[i][j];
        }
    }

    return t_mat;
}

static mat_t int_pow(mat_t base, unsigned int exp)
{
    mat_t result = 1;
    int i;

    if(exp == 0)
        return KERNEL_TIMES;

    for(i = 0; i < exp; i++) {
        result *= base;
        if (i != 0)
            result /= KERNEL_TIMES;
    }

    return result;
}

static mat_t my_sqrt(mat_t x)
{
    return int_sqrt(x);
}

static mat_t my_tanh(mat_t xx)
{
    mat_t sinh_val = 0, cosh_val = 0, tanh_val = 0;
    int i, nr = 14;

    for(i = 1; i < nr; i+=2)
        sinh_val += int_pow(xx, i) / factorial(i);

    for(i = 0; i < nr; i+=2)
        cosh_val += int_pow(xx, i) / factorial(i);

    tanh_val = sinh_val  * KERNEL_TIMES / cosh_val;

    return tanh_val;
}

static mat_t my_exp(mat_t xx)
{
    mat_t exp_val = 0;
    int i, nr = 17;
    char less_zero = 0;
    if(xx < 0) {
        less_zero = 1;
        xx = -xx;
    }
    for(i = 0; i < nr; i++) {
        exp_val += int_pow(xx, i) / factorial(i);
    }
    if(less_zero == 1)
        exp_val = KERNEL_TIMES * KERNEL_TIMES / exp_val;
    return exp_val;
}

static struct matrix *my_softmax(struct matrix *mat)
{
    int m, n;

    struct matrix *out = matrix_alloc(mat->rows, mat->cols);
    if(!out)
        return NULL;
    for(m = 0; m < mat->rows; m++) {
        mat_t max_val = mat->data[m][0];
        mat_t x_exp_sum = 0;
        for(n = 0; n < mat->cols; n++) {
            if(mat->data[m][n] > max_val) // 找最大值
                max_val = mat->data[m][n];
        }

        for(n = 0; n < mat->cols; n++) {
            mat_t val = mat->data[m][n] - max_val;
            mat_t x_exp = my_exp(val);
            out->data[m][n] = x_exp;
            x_exp_sum += x_exp;
        }

        for(n = 0; n < mat->cols; n++)
            out->data[m][n] = out->data[m][n] * KERNEL_TIMES / x_exp_sum;
    }

    return out;
}



// 这里计算范数需要一些 trick，有一些数据直接溢出，影响计算
mat_t L2_norm(struct matrix *mat, int pos, int axis)
{
    int n;
    mat_t norm = 0;
    if(axis == 1) {
        for(n = 0; n < mat->cols; n++) {
            norm += mat->data[pos][n] * mat->data[pos][n];
        }
    } else {
        for(n = 0; n < mat->rows; n++) {
            norm += mat->data[n][pos] * mat->data[n][pos];
        }
    }
    return my_sqrt(norm);
}

// 这个函数给一些很大的数据做norm，会溢出，因此可以先除以KERNEL_TIMES回归原始数据，再算norm，最后再乘KERNEL_TIMES回复状态。
mat_t L2_big_norm(struct matrix *mat, int pos, int axis)
{
    int n;
    mat_t norm = 0, val;
    if(axis == 1) {
        for(n = 0; n < mat->cols; n++) {
            val = mat->data[pos][n] / KERNEL_TIMES;
            norm += val * val;
        }
    } else {
        for(n = 0; n < mat->rows; n++) {
            val = mat->data[n][pos] / KERNEL_TIMES;
            norm += val * val;
        }
    }
    val = my_sqrt(norm) * KERNEL_TIMES;
    return val;
}


struct matrix *matrix_normalize(struct matrix *mat, int axis)
{
    int i, j;
    mat_t norm;
    struct matrix *ret;

    ret = matrix_alloc(mat->rows, mat->cols);
    if(!ret)
        return NULL;

    if(axis == 1) { // 每一个属性进行正则化
        for(i = 0; i < mat->rows; i++) {
            norm = L2_big_norm(mat, i, 1);
            for(j = 0; j < mat->cols; j++) {
                if(norm != 0){
                    ret->data[i][j] = mat->data[i][j] * KERNEL_TIMES / norm;
                } else {
                    ret->data[i][j] = 0;
                }
            }
        }
    } else if(axis == 0) { // 每一记录进行正则化
        for(j = 0; j < mat->cols; j++) {
            norm = L2_big_norm(mat, j, 0);
            for(i = 0; i < mat->rows; i++) {
                if(norm != 0) {
                    ret->data[i][j] = mat->data[i][j] * KERNEL_TIMES / norm;
                } else {
                    ret->data[i][j] = 0;
                }
            }
        }
    } else {
        printk("axis must be 0 or 1!\n");
        matrix_release(ret);
        return NULL;
    }
    return ret;
}

void init_mlp(struct mlp_model *mlp_model, struct mlp_conf conf)
{
    int i, j;
    mlp_model->w0 = matrix_alloc(conf.m_w0, conf.n_w0);
    mlp_model->w1 = matrix_alloc(conf.m_w1, conf.n_w1);
    mlp_model->w2 = matrix_alloc(conf.m_w2, conf.n_w2);
    mlp_model->w3 = matrix_alloc(conf.m_w3, conf.n_w3);

    mlp_model->b0 = matrix_alloc(conf.m_b0, conf.n_b0);
    mlp_model->b1 = matrix_alloc(conf.m_b1, conf.n_b1);
    mlp_model->b2 = matrix_alloc(conf.m_b2, conf.n_b2);
    mlp_model->b3 = matrix_alloc(conf.m_b3, conf.n_b3);

    mlp_model->mean = matrix_alloc(conf.m_mean, conf.n_mean);
    mlp_model->scale = matrix_alloc(conf.m_scale, conf.n_scale);

    for(i = 0; i < conf.m_w0; i++) {
        for(j = 0; j < conf.n_w0; j++) {
            mlp_model->w0->data[i][j] = w0[i][j];
        }
    }

    for(i = 0; i < conf.m_w1; i++) {
        for(j = 0; j < conf.n_w1; j++) {
            mlp_model->w1->data[i][j] = w1[i][j];
        }
    }

    for(i = 0; i < conf.m_w2; i++) {
        for(j = 0; j < conf.n_w2; j++) {
            mlp_model->w2->data[i][j] = w2[i][j];
        }
    }

    for(i = 0; i < conf.m_w3; i++) {
        for(j = 0; j < conf.n_w3; j++) {
            mlp_model->w3->data[i][j] = w3[i][j];
        }
    }

    for(i = 0; i < conf.m_b0; i++) {
        mlp_model->b0->data[i][0] = b0[i];
    }

    for(i = 0; i < conf.m_b1; i++) {
        mlp_model->b1->data[i][0] = b1[i];
    }

    for(i = 0; i < conf.m_b2; i++) {
        mlp_model->b2->data[i][0] = b2[i];
    }

    for(i = 0; i < conf.m_b3; i++) {
        mlp_model->b3->data[i][0] = b3[i];
    }

    for(i = 0; i < conf.m_mean; i++) {
        mlp_model->mean->data[i][0] = mean[i];
    }

    for(i = 0; i < conf.m_scale; i++) {
        mlp_model->scale->data[i][0] = scale[i];
    }
}

void init_mlp2(struct mlp_model *mlp_model, struct mlp_conf conf)
{
    int i, j;
    mlp_model->w0 = matrix_alloc(conf.m_w0, conf.n_w0);
    mlp_model->w1 = matrix_alloc(conf.m_w1, conf.n_w1);
    mlp_model->w2 = matrix_alloc(conf.m_w2, conf.n_w2);
    mlp_model->w3 = matrix_alloc(conf.m_w3, conf.n_w3);

    mlp_model->b0 = matrix_alloc(conf.m_b0, conf.n_b0);
    mlp_model->b1 = matrix_alloc(conf.m_b1, conf.n_b1);
    mlp_model->b2 = matrix_alloc(conf.m_b2, conf.n_b2);
    mlp_model->b3 = matrix_alloc(conf.m_b3, conf.n_b3);

    mlp_model->mean = matrix_alloc(conf.m_mean, conf.n_mean);
    mlp_model->scale = matrix_alloc(conf.m_scale, conf.n_scale);

    for(i = 0; i < conf.m_w0; i++) {
        for(j = 0; j < conf.n_w0; j++) {
            mlp_model->w0->data[i][j] = w0_80[i][j];
        }
    }

    for(i = 0; i < conf.m_w1; i++) {
        for(j = 0; j < conf.n_w1; j++) {
            mlp_model->w1->data[i][j] = w1_80[i][j];
        }
    }

    for(i = 0; i < conf.m_w2; i++) {
        for(j = 0; j < conf.n_w2; j++) {
            mlp_model->w2->data[i][j] = w2_80[i][j];
        }
    }

    for(i = 0; i < conf.m_w3; i++) {
        for(j = 0; j < conf.n_w3; j++) {
            mlp_model->w3->data[i][j] = w3_80[i][j];
        }
    }

    for(i = 0; i < conf.m_b0; i++) {
        mlp_model->b0->data[i][0] = b0_80[i];
    }

    for(i = 0; i < conf.m_b1; i++) {
        mlp_model->b1->data[i][0] = b1_80[i];
    }

    for(i = 0; i < conf.m_b2; i++) {
        mlp_model->b2->data[i][0] = b2_80[i];
    }

    for(i = 0; i < conf.m_b3; i++) {
        mlp_model->b3->data[i][0] = b3_80[i];
    }

    for(i = 0; i < conf.m_mean; i++) {
        mlp_model->mean->data[i][0] = mean_80[i];
    }

    for(i = 0; i < conf.m_scale; i++) {
        mlp_model->scale->data[i][0] = scale_80[i];
    }
}

void free_mlp(struct mlp_model *mlp_model)
{
    matrix_release(mlp_model->w0);
    matrix_release(mlp_model->w1);
    matrix_release(mlp_model->w2);
    matrix_release(mlp_model->w3);

    matrix_release(mlp_model->b0);
    matrix_release(mlp_model->b1);
    matrix_release(mlp_model->b2);
    matrix_release(mlp_model->b3);

    matrix_release(mlp_model->mean);
    matrix_release(mlp_model->scale);
}

static mat_t relu(mat_t v)
{
    return v > 0 ? v : 0;
}

struct matrix *mlp_transform(struct mlp_model *model, struct matrix *input_mat)
{
    int i, j;
    struct matrix *mlp0, *mlp1, *mlp2, *mlp_out, *mlp_out_T, *mlp_out_softmax;
    struct matrix *input_T = matrix_transpos(input_mat);
    struct matrix *pred;

    if(!input_T)
        return NULL;

    pred = matrix_alloc(input_mat->rows, 1);
    if(!pred) {
        matrix_release(input_T);
        return NULL;
    }

    mlp0 = matrix_int_mul(model->w0, input_T);
    if(!mlp0) {
        matrix_release(input_T);
        matrix_release(pred);
        return NULL;
    }

    // tanh
    for(j = 0; j < mlp0->cols; j++) {
        for(i = 0; i < mlp0->rows; i++) {
            mlp0->data[i][j] += model->b0->data[i][0]; // 加上偏置b0
            mlp0->data[i][j] = my_tanh(mlp0->data[i][j]);
        }
    }
    matrix_release(input_T);

    mlp1 = matrix_int_mul(model->w1, mlp0);
    if(!mlp1) {
        matrix_release(mlp0);
        matrix_release(pred);
        return NULL;
    }

    // relu
    for(i = 0; i < mlp1->rows; i++) {
        for(j = 0; j < mlp1->cols; j++) {
            mlp1->data[i][j] += model->b1->data[i][0]; // 加上偏置b1
            mlp1->data[i][j] = relu(mlp1->data[i][j]);
        }
    }
    matrix_release(mlp0);

    mlp2 = matrix_int_mul(model->w2, mlp1);
    if(!mlp2) {
        matrix_release(mlp1);
        matrix_release(pred);
        return NULL;
    }

    // relu
    for(i = 0; i < mlp2->rows; i++) {
        for(j = 0; j < mlp2->cols; j++) {
            mlp2->data[i][j] += model->b2->data[i][0]; // 加上偏置b2
            mlp2->data[i][j] = relu(mlp2->data[i][j]);
        }
    }
    matrix_release(mlp1);

    mlp_out = matrix_int_mul(model->w3, mlp2);
    if(!mlp_out) {
        matrix_release(mlp2);
        matrix_release(pred);
        return NULL;
    }

    for(i = 0; i < mlp_out->rows; i++) {
        for(j = 0; j < mlp_out->cols; j++) {
            mlp_out->data[i][j] += model->b3->data[i][0]; // 加上偏置b3
        }
    }
    matrix_release(mlp2);

    mlp_out_T = matrix_transpos(mlp_out);
    if(!mlp_out_T) {
        matrix_release(mlp_out);
        matrix_release(pred);
        return NULL;
    }

    matrix_release(mlp_out);

    // softmax
    mlp_out_softmax = my_softmax(mlp_out_T); // 调整kernel time的使用
    if(!mlp_out_softmax) {
        matrix_release(mlp_out_T);
        matrix_release(pred);
        return NULL;
    }

    matrix_release(mlp_out_T);

    // 分类
    for(i = 0; i < mlp_out_softmax->rows; i++) {
        if(mlp_out_softmax->data[i][1] >=  mlp_out_softmax->data[i][0]) {
            pred->data[i][0] = 1;
        } else {
            pred->data[i][0] = 0;
        }
    }

    matrix_release(mlp_out_softmax);
    return pred;
}

struct matrix *mlp_normalize(struct mlp_model *model, struct matrix *input)
{
    int i, j;
    struct matrix *norm_input = matrix_normalize(input, 1); // 暂时不需要归一化
    for(i = 0; i < norm_input->rows; i++) {
        for(j = 0; j < norm_input->cols; j++) {
            norm_input->data[i][j] = (norm_input->data[i][j] - model->mean->data[j][0]) * KERNEL_TIMES / model->scale->data[j][0];
        }
    }

    return norm_input;
}
