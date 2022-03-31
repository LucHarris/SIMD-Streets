#pragma once
#include <emmintrin.h> // SIMD intrinsics

#include "Constants.h"
#ifdef SIMD
struct FighterSOA
{
    _declspec(align(16)) float posX[gc::NUM_FIGHTERS_SCALAR];
		_declspec(align(16))float posY[gc::NUM_FIGHTERS_SCALAR];
		_declspec(align(16))float velX[gc::NUM_FIGHTERS_SCALAR];
		_declspec(align(16))float velY[gc::NUM_FIGHTERS_SCALAR];
		_declspec(align(16))uint32_t team[gc::NUM_FIGHTERS_SCALAR];
		_declspec(align(16))uint32_t member[gc::NUM_FIGHTERS_SCALAR];
		_declspec(align(16))uint32_t frameOffset[gc::NUM_FIGHTERS_SCALAR];
	// todo initialise
    _declspec(align(16))uint32_t frameNum[gc::NUM_FIGHTERS_SCALAR];
    _declspec(align(16))uint32_t alive[gc::NUM_FIGHTERS_SCALAR];
};
struct FighterIndices
{
	enum{BLUE,PURPLE,COUNT};
	// initial loop requires indervidual incrementing for sorting
	// where to start in soa
	uint32_t start[COUNT];
	// points to start of arrays
	float* p_pos_x = nullptr;
	float* p_pos_y = nullptr;
	float* p_vel_x = nullptr;
	float* p_vel_y = nullptr;
	uint32_t* p_team_id = nullptr;
	uint32_t* p_member = nullptr;
	uint32_t* p_frame_offset = nullptr;
	uint32_t* p_frame_num = nullptr;
	uint32_t* p_is_alive = nullptr;

	// initial loop convert above into __m128(i) 

	__m128* p_m128_pos_x = nullptr;
	__m128* p_m128_pos_y = nullptr;
	__m128* p_m128_vel_x = nullptr;
	__m128* p_m128_vel_y = nullptr;
	__m128i* p_m128i_team_id = nullptr;
	__m128i* p_m128i_member = nullptr;
	__m128i* p_m128i_frame_offset = nullptr;
	// todo test and remove if not required
	__m128i* p_m128i_frame_num = nullptr;
	__m128i* p_m128i_is_alive = nullptr;

	FighterIndices();

	void inc_m128() volatile;

	void int_single() volatile;
	void step_single_and_set(uint32_t num_steps) volatile;
	// updates m128 ptrs based on single pointers
	void set_m128() volatile;
	uint32_t validate() volatile;
};
#else //SIMD
struct FighterScalar
{
	float posX, posY, velX, velY;
	uint32_t team, member, frameOffset, frameNum alive;
};
struct FighterAOS
{
	FighterScalar data[gc::NUM_FIGHTERS_SCALAR];
};

#endif // SIMD

