/*
 * wf_dense.h - workspace for weighted-feature data in dense format
 *
 */
#ifndef _WF_DENSE_H_
#define _WF_DENSE_H_

typedef struct
  {
  int M;
  int N;
  double *wx; // M x 2N
  double *w;  // M x 2N  cell weights (if NULL, all weights are 1's)
  double *t;  // M row weights
  double *u;  // 2N column weights
  double St2; // sum of row-weight squared (== M if all equal)
  }
wf_dense;


// callback functions for nnc

extern void
wf_dense_nnc_scan(
  void *data,
  int j,
  int nc,
  int *c_,
  double *Sj_
  )
;

extern void
wf_dense_nnmerge(
  void *data,
  int m,
  int j,
  int k
  )
;

// similarity between nodes (for branch flip)

static inline double
wf_dense_wcov(
  void *data,
  int j,
  int k
  )
{
  wf_dense *D = (wf_dense*)data;
  int M = D->M;

  double *wxj = D->wx + M*j;
  double *wxk = D->wx + M*k;
  double swx = 0, sw = 0;
  #pragma omp simd
  for(int i = 0; i < M; i++ )
    swx += wxj[i]*wxk[i];
  if( D->w )
    {
    double *wj = D->w + M*j;
    double *wk = D->w + M*k;
    #pragma omp simd
    for(int i = 0; i < M; i++ )
      sw += wj[i] * wk[i];
    }
  else
    sw = D->St2;

  return swx / sw;
}

static inline double
wf_dense_avecor(
  void *data,
  int j,
  int k
  )
{
  wf_dense *D = (wf_dense*)data;
  int M = D->M;

  double *xj = D->wx + M*j;
  double *xk = D->wx + M*k;
  double *wj = D->w + M*j;
  double *wk = D->w + M*k;
  double sjk = 0, sjj = 0, skk = 0;
  #pragma omp simd
  for(int i = 0; i < M; i++ )
    {
    sjk += wj[i]*wk[i]*xj[i]*xk[i];
    sjj += wj[i]*wj[i]*xj[i]*xj[i];
    skk += wk[i]*wk[i]*xk[i]*xk[i];
    }
  if( sjj*skk > 0 )
    return sjk/sqrt(sjj*skk);
  else
    return 0;
}

static inline double
wf_dense_ward(
  void *data,
  int j,
  int k
  )
{
  wf_dense *D = (wf_dense*)data;
  int M = D->M;

  double *wxj = D->wx + M*j;
  double *wxk = D->wx + M*k;
  double swx = 0, sw = 0;
  #pragma omp simd
  for(int i = 0; i < M; i++ )
    {
    double d = wxj[i]-wxk[i];
    swx -= d*d;
    }
  sw = D->St2;

  double wj = D->u[j], wk = D->u[k];
  return wj*wk/(wj+wk)*swx / sw;
}

extern void
wf_dense_nclust
  (
  const int *dims,
  const int *options,

  double *xx,   // feature data matrix + workspace for branches
  double *ww,   // joint weights + workspace for branches
  double *t,    // feature weights
  double *u,    // item weights

  int *L, int *R, int *U,
  double *S,
  int *order,
  int *n,
  int *leftmost,
  int *level,
  int *retstat
  )
;

#endif // _WF_DENSE_H_
