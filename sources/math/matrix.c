#include <kore3/math/core.h>
#include <kore3/math/matrix.h>
#include <kore3/simd/float32x4.h>

#include <string.h>

float kore_matrix3x3_get(kore_matrix3x3 *matrix, int x, int y) {
	return matrix->m[x * 3 + y];
}

void kore_matrix3x3_set(kore_matrix3x3 *matrix, int x, int y, float value) {
	matrix->m[x * 3 + y] = value;
}

void kore_matrix3x3_transpose(kore_matrix3x3 *matrix) {
	kore_matrix3x3 transposed;
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			kore_matrix3x3_set(&transposed, x, y, kore_matrix3x3_get(matrix, y, x));
		}
	}
	memcpy(matrix->m, transposed.m, sizeof(transposed.m));
}

kore_matrix3x3 kore_matrix3x3_identity(void) {
	kore_matrix3x3 m;
	memset(m.m, 0, sizeof(m.m));
	for (unsigned x = 0; x < 3; ++x) {
		kore_matrix3x3_set(&m, x, x, 1.0f);
	}
	return m;
}

kore_matrix3x3 kore_matrix3x3_rotation_x(float alpha) {
	kore_matrix3x3 m  = kore_matrix3x3_identity();
	float          ca = cosf(alpha);
	float          sa = sinf(alpha);
	kore_matrix3x3_set(&m, 1, 1, ca);
	kore_matrix3x3_set(&m, 2, 1, -sa);
	kore_matrix3x3_set(&m, 1, 2, sa);
	kore_matrix3x3_set(&m, 2, 2, ca);
	return m;
}

kore_matrix3x3 kore_matrix3x3_rotation_y(float alpha) {
	kore_matrix3x3 m  = kore_matrix3x3_identity();
	float          ca = cosf(alpha);
	float          sa = sinf(alpha);
	kore_matrix3x3_set(&m, 0, 0, ca);
	kore_matrix3x3_set(&m, 2, 0, sa);
	kore_matrix3x3_set(&m, 0, 2, -sa);
	kore_matrix3x3_set(&m, 2, 2, ca);
	return m;
}

kore_matrix3x3 kore_matrix3x3_rotation_z(float alpha) {
	kore_matrix3x3 m  = kore_matrix3x3_identity();
	float          ca = cosf(alpha);
	float          sa = sinf(alpha);
	kore_matrix3x3_set(&m, 0, 0, ca);
	kore_matrix3x3_set(&m, 1, 0, -sa);
	kore_matrix3x3_set(&m, 0, 1, sa);
	kore_matrix3x3_set(&m, 1, 1, ca);
	return m;
}

kore_matrix3x3 kore_matrix3x3_translation(float x, float y) {
	kore_matrix3x3 m = kore_matrix3x3_identity();
	kore_matrix3x3_set(&m, 2, 0, x);
	kore_matrix3x3_set(&m, 2, 1, y);
	return m;
}

kore_matrix3x3 kore_matrix3x3_scale(float x, float y, float z) {
	kore_matrix3x3 m = kore_matrix3x3_identity();
	kore_matrix3x3_set(&m, 0, 0, x);
	kore_matrix3x3_set(&m, 1, 1, y);
	kore_matrix3x3_set(&m, 2, 2, z);
	return m;
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#endif

kore_matrix3x3 kore_matrix3x3_multiply(kore_matrix3x3 *a, kore_matrix3x3 *b) {
	kore_matrix3x3 result;
	for (unsigned x = 0; x < 3; ++x) {
		for (unsigned y = 0; y < 3; ++y) {
			float t = kore_matrix3x3_get(a, 0, y) * kore_matrix3x3_get(b, x, 0);
			for (unsigned i = 1; i < 3; ++i) {
				t += kore_matrix3x3_get(a, i, y) * kore_matrix3x3_get(b, x, i);
			}
			kore_matrix3x3_set(&result, x, y, t);
		}
	}
	return result;
}

static float vector3_get(kore_float3 vec, int index) {
	float *values = (float *)&vec;
	return values[index];
}

static void vector3_set(kore_float3 *vec, int index, float value) {
	float *values = (float *)vec;
	values[index] = value;
}

kore_float3 kore_matrix3x3_multiply_vector(kore_matrix3x3 *a, kore_float3 b) {
	kore_float3 product;
	for (unsigned y = 0; y < 3; ++y) {
		float t = 0;
		for (unsigned x = 0; x < 3; ++x) {
			t += kore_matrix3x3_get(a, x, y) * vector3_get(b, x);
		}
		vector3_set(&product, y, t);
	}
	return product;
}

float kore_matrix4x4_get(kore_matrix4x4 *matrix, int x, int y) {
	return matrix->m[x * 4 + y];
}

void kore_matrix4x4_set(kore_matrix4x4 *matrix, int x, int y, float value) {
	matrix->m[x * 4 + y] = value;
}

void kore_matrix4x4_transpose(kore_matrix4x4 *matrix) {
	kore_matrix4x4 transposed;
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			kore_matrix4x4_set(&transposed, x, y, kore_matrix4x4_get(matrix, y, x));
		}
	}
	memcpy(matrix->m, transposed.m, sizeof(transposed.m));
}

kore_matrix4x4 kore_matrix4x4_identity(void) {
	kore_matrix4x4 m;
	memset(m.m, 0, sizeof(m.m));
	for (unsigned x = 0; x < 4; ++x) {
		kore_matrix4x4_set(&m, x, x, 1.0f);
	}
	return m;
}

kore_matrix4x4 kore_matrix4x4_rotation_x(float alpha) {
	kore_matrix4x4 m  = kore_matrix4x4_identity();
	float          ca = cosf(alpha);
	float          sa = sinf(alpha);
	kore_matrix4x4_set(&m, 1, 1, ca);
	kore_matrix4x4_set(&m, 2, 1, -sa);
	kore_matrix4x4_set(&m, 1, 2, sa);
	kore_matrix4x4_set(&m, 2, 2, ca);
	return m;
}

kore_matrix4x4 kore_matrix4x4_rotation_z(float alpha) {
	kore_matrix4x4 m  = kore_matrix4x4_identity();
	float          ca = cosf(alpha);
	float          sa = sinf(alpha);
	kore_matrix4x4_set(&m, 0, 0, ca);
	kore_matrix4x4_set(&m, 1, 0, -sa);
	kore_matrix4x4_set(&m, 0, 1, sa);
	kore_matrix4x4_set(&m, 1, 1, ca);
	return m;
}

kore_matrix4x4 kore_matrix4x4_rotation_y(float alpha) {
	kore_matrix4x4 m  = kore_matrix4x4_identity();
	float          ca = cosf(alpha);
	float          sa = sinf(alpha);
	kore_matrix4x4_set(&m, 0, 0, ca);
	kore_matrix4x4_set(&m, 2, 0, sa);
	kore_matrix4x4_set(&m, 0, 2, -sa);
	kore_matrix4x4_set(&m, 2, 2, ca);
	return m;
}

kore_matrix4x4 kore_matrix4x4_translation(float x, float y, float z) {
	kore_matrix4x4 m = kore_matrix4x4_identity();
	kore_matrix4x4_set(&m, 3, 0, x);
	kore_matrix4x4_set(&m, 3, 1, y);
	kore_matrix4x4_set(&m, 3, 2, z);
	return m;
}

kore_matrix4x4 kore_matrix4x4_scale(float x, float y, float z) {
	kore_matrix4x4 m = kore_matrix4x4_identity();
	kore_matrix4x4_set(&m, 0, 0, x);
	kore_matrix4x4_set(&m, 1, 1, y);
	kore_matrix4x4_set(&m, 2, 2, z);
	return m;
}

kore_matrix4x4 kore_matrix4x4_multiply(kore_matrix4x4 *a, kore_matrix4x4 *b) {
	kore_matrix4x4 result;
	for (unsigned x = 0; x < 4; ++x)
		for (unsigned y = 0; y < 4; ++y) {
			float t = kore_matrix4x4_get(a, 0, y) * kore_matrix4x4_get(b, x, 0);
			for (unsigned i = 1; i < 4; ++i) {
				t += kore_matrix4x4_get(a, i, y) * kore_matrix4x4_get(b, x, i);
			}
			kore_matrix4x4_set(&result, x, y, t);
		}
	return result;
}

static float vector4_get(kore_float4 vec, int index) {
	float *values = (float *)&vec;
	return values[index];
}

static void vector4_set(kore_float4 *vec, int index, float value) {
	float *values = (float *)vec;
	values[index] = value;
}

kore_float4 kore_matrix4x4_multiply_vector(kore_matrix4x4 *a, kore_float4 b) {
	kore_float4 product;
	for (unsigned y = 0; y < 4; ++y) {
		float t = 0;
		for (unsigned x = 0; x < 4; ++x) {
			t += kore_matrix4x4_get(a, x, y) * vector4_get(b, x);
		}
		vector4_set(&product, y, t);
	}
	return product;
}

kore_matrix4x4 kore_matrix4x4_perspective(float fov, float aspect, float near, float far) {
	float uh = 1.0f / tanf(fov / 2.0f);
	float uw = uh / aspect;
	
	kore_matrix4x4 m = {
		.m = {
			uw, 0, 0, 0,
			0, uh, 0, 0,
			0, 0, (far + near) / (near - far), -1,
			0, 0, (2 * far * near) / (near - far), 0
		}
	};
	return m;
}

kore_matrix4x4 kore_matrix4x4_look_at(kore_float3 eye, kore_float3 at, kore_float3 up) {
	kore_float3 zaxis = kore_float3_normalize(kore_float3_sub(at, eye));
	kore_float3 xaxis = kore_float3_normalize(kore_float3_cross(zaxis, up));
	kore_float3 yaxis = kore_float3_cross(xaxis, zaxis);
	
	kore_matrix4x4 m = {
		xaxis.x, yaxis.x, -zaxis.x, 0,
		xaxis.y, yaxis.y, -zaxis.y, 0,
		xaxis.z, yaxis.z, -zaxis.z, 0,
		-kore_float3_dot(xaxis, eye), -kore_float3_dot(yaxis, eye), kore_float3_dot(zaxis, eye), 1
	};
	return m;
}

kore_matrix4x4 kore_matrix4x4_multiply_simd(kore_matrix4x4 *a, kore_matrix4x4 *b) {
	kore_matrix4x4 result;
	
	float *am = a->m;
	float *bm = b->m;
	float *rm = result.m;
	
	kore_float32x4 a_col0 = kore_float32x4_intrin_load_unaligned(&am[0]);
	kore_float32x4 a_col1 = kore_float32x4_intrin_load_unaligned(&am[4]);
	kore_float32x4 a_col2 = kore_float32x4_intrin_load_unaligned(&am[8]);
	kore_float32x4 a_col3 = kore_float32x4_intrin_load_unaligned(&am[12]);
	
	for (int x = 0; x < 4; x++) {
		float b0 = bm[x * 4 + 0];
		float b1 = bm[x * 4 + 1];
		float b2 = bm[x * 4 + 2];
		float b3 = bm[x * 4 + 3];
		
		kore_float32x4 result_col = kore_float32x4_add(
			kore_float32x4_add(
				kore_float32x4_mul(kore_float32x4_load_all(b0), a_col0),
				kore_float32x4_mul(kore_float32x4_load_all(b1), a_col1)
			),
			kore_float32x4_add(
				kore_float32x4_mul(kore_float32x4_load_all(b2), a_col2),
				kore_float32x4_mul(kore_float32x4_load_all(b3), a_col3)
			)
		);
		
		kore_float32x4_store_unaligned(&rm[x * 4], result_col);
	}
	
	return result;
}

kore_float4 kore_matrix4x4_multiply_vector_simd(kore_matrix4x4 *a, kore_float4 b) {
	kore_float32x4 a_col0 = kore_float32x4_intrin_load_unaligned(&a->m[0]);
	kore_float32x4 a_col1 = kore_float32x4_intrin_load_unaligned(&a->m[4]);
	kore_float32x4 a_col2 = kore_float32x4_intrin_load_unaligned(&a->m[8]);
	kore_float32x4 a_col3 = kore_float32x4_intrin_load_unaligned(&a->m[12]);
	
	kore_float32x4 b0 = kore_float32x4_load_all(b.x);
	kore_float32x4 b1 = kore_float32x4_load_all(b.y);
	kore_float32x4 b2 = kore_float32x4_load_all(b.z);
	kore_float32x4 b3 = kore_float32x4_load_all(b.w);
	
	kore_float32x4 result_vec = kore_float32x4_add(
		kore_float32x4_add(kore_float32x4_mul(b0, a_col0), kore_float32x4_mul(b1, a_col1)),
		kore_float32x4_add(kore_float32x4_mul(b2, a_col2), kore_float32x4_mul(b3, a_col3))
	);
	
	kore_float4 result;
	result.x = kore_float32x4_get(result_vec, 0);
	result.y = kore_float32x4_get(result_vec, 1);
	result.z = kore_float32x4_get(result_vec, 2);
	result.w = kore_float32x4_get(result_vec, 3);
	
	return result;
}

void kore_matrix4x4_transpose_simd(kore_matrix4x4 *matrix) {
	float *m = matrix->m;
	
	float tmp;
	tmp = m[1];  m[1]  = m[4];  m[4]  = tmp;
	tmp = m[2];  m[2]  = m[8];  m[8]  = tmp;
	tmp = m[3];  m[3]  = m[12]; m[12] = tmp;
	tmp = m[6];  m[6]  = m[9];  m[9]  = tmp;
	tmp = m[7];  m[7]  = m[13]; m[13] = tmp;
	tmp = m[11]; m[11] = m[14]; m[14] = tmp;
}
