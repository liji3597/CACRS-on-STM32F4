#include "filter.h"
#include "callback.h"
#include "math.h"
#include "sys.h"

//#define ECG_DATE_SIZE  300    /* �������� */
#define BLOCK_SIZE           30     /* ����һ��arm_fir_f32����Ĳ�������� */
#define NUM_TAPS             29     /* �˲���ϵ������ */


uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = ECG_DATE_SIZE/BLOCK_SIZE;            /* ��Ҫ����arm_fir_f32�Ĵ��� */
 
//static float32_t testInput_f32_50Hz_200Hz[ECG_DATE_SIZE]; /* ������ */
//static float32_t testOutput[ECG_DATE_SIZE];               /* �˲������� */

static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];        /* ״̬���棬��СnumTaps + blockSize - 1*/

arm_fir_instance_f32 S;

/* ��ͨ�˲���ϵ�� ͨ��fadtool��ȡ*/
const float32_t firCoeffs32BP[NUM_TAPS] = {
0.003531039227f,    0.0002660876198f,   -0.001947779674f,  0.001266813371f,  -0.008019094355f,
-0.01986379735f,    0.01018396299f,     0.03163734451f,    0.00165955862f,   0.03312643617f,
0.0622616075f,      -0.1229852438f,     -0.2399847955f,    0.07637182623f,   0.3482480049f,
0.07637182623f,     -0.2399847955f,     -0.1229852438f,    0.0622616075f,    0.03312643617f,
0.00165955862f,     0.03163734451f,     0.01018396299f,    -0.01986379735f,  -0.008019094355f,
0.001266813371f,   -0.001947779674f,    0.0002660876198f,  0.003531039227f
};
	/* ��ʼ���ṹ��S */
void arm_fir_init()
{
	arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32BP[0], &firStateF32[0], blockSize);
}

void arm_fir(float32_t *inputF32, float32_t *outputF32)
{
uint32_t i;
//float32_t  *inputF32, *outputF32;
 
/* ��ʼ�������������ָ�� */
//inputF32 = &testInput_f32_50Hz_200Hz[0];
//outputF32 = &testOutput[0];
// 


/* ʵ��FIR�˲� */
	for(i=0; i < numBlocks; i++)
	{
		arm_fir_f32(&S, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
	}
 
/* ��ӡ�˲����� */
//	for(i=0; i<ECG_DATE_SIZE; i++)
//	{
//		printf("%f\r\n", testOutput[i]);
//	}
}

