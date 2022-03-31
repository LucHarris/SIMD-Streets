#pragma once
#include <SFML/Graphics.hpp>
#include <tmmintrin.h>
#ifndef SIMD
#define SIMD
#endif // !SIMD

struct anim_data
{
	enum { IDLE = 0, WALK, ATTACK, FALL, DOWN };
	uint32_t offset_frame;
	uint32_t num_frames;

};




namespace gc
{
	const anim_data ANIM_DATA[5]
	{
	  {0,4},
	  {4,8},
	  {8,2},
	  {12,4},
	  {15,1},
	};

	const sf::IntRect TEX_RECT(0, 0, 512, 512);
	const char TEX_PATH[] = "../Data/street.png";
	// count of all blues
	const uint32_t NUM_BLUE_SCALAR = 1 << 5;
	// count of all purples
	const uint32_t NUM_PURPLE_SCALAR = 1 << 5;
	// count of all fighters
	const uint32_t NUM_FIGHTERS_SCALAR = NUM_BLUE_SCALAR + NUM_PURPLE_SCALAR;
	const uint32_t NUM_FIGHTER_FRAMES = 4;
	const uint32_t WINDOW_WIDTH = 800U;
	const uint32_t WINDOW_HEIGHT = 600U;
	const sf::Color WINDOW_FILL = { 0,100,0,255 };
	const uint32_t FIGHTER_W = 32u;
	const uint32_t FIGHTER_H = 32u;
	const uint32_t ANIM_X_OFFSET = 128;
	const uint32_t ANIM_Y_OFFSET = 0;
	// fighter sprite dimentions
	const sf::IntRect SPR_RECT_BG = { 0U,256U,WINDOW_WIDTH,256U };
	const sf::IntRect SPR_RECT_FIGHTER = {128U,0U,32U,32U };
	const sf::FloatRect BOUNDARY_RECT = { 20.0f,150.0f,(float)WINDOW_WIDTH - 40.0f ,(float)WINDOW_HEIGHT - 100.0f };
	const uint32_t BLUE_TEAM = 0;
	const uint32_t PURPLE_TEAM = 1;
	const uint32_t MEMBER_COUNT = 4;
	// slight variation due to velocity not being normalised
	const float FIGHTER_SPEED = 10.0f;
	const float SQUARE_DISTANCE_BETWEEN_FIGHTERS = 100.0f;
	

#ifdef SIMD
	const char APP_NAME[] = "SIMD Streets";
	// count of simd grouped blues
	const uint32_t NUM_BLUE_SIMD = NUM_BLUE_SCALAR >> 2;
	// count of simd grouped purples
	const uint32_t NUM_PURPLE_SIMD = NUM_PURPLE_SCALAR >> 2;
	// count of all simd grouped fighers
	const uint32_t NUM_FIGHTERS_SIMD = NUM_BLUE_SIMD + NUM_PURPLE_SIMD;
	const uint32_t SHUF_MAX = 16;
	const __m128 ONE_PS = _mm_set1_ps(1);
	const __m128 NEG_ONE_PS = _mm_set1_ps(-1);
	const __m128 ZERO_PS = _mm_set1_ps(0);
	const __m128 SQU_DISTANCE_BETWEEN_FIGHTERS_PS = _mm_set1_ps(SQUARE_DISTANCE_BETWEEN_FIGHTERS);

	const __m128i ZERO_EPI = _mm_set1_epi32(0);
	const __m128i ONE_EPI = _mm_set1_epi32(1);
	const __m128i NEG_ONE_EPI = _mm_set1_epi32(-1);

	const __m128i DEAD_NUM_FRAME_EPI = _mm_set1_epi32(ANIM_DATA[anim_data::DOWN].num_frames);
	const __m128i DEAD_FAME_OFFSET_EPI = _mm_set1_epi32(ANIM_DATA[anim_data::DOWN].offset_frame);

	enum {
		MIN_X = 0, MAX_X, MIN_Y, MAX_Y, BOUNDS_COUNT

	};


	const __m128 m128_BOUNDS[BOUNDS_COUNT]
	{
		_mm_set1_ps((float)BOUNDARY_RECT.left),
		_mm_set1_ps((float)(BOUNDARY_RECT.left + BOUNDARY_RECT.width)),
		_mm_set1_ps((float)(BOUNDARY_RECT.top - BOUNDARY_RECT.height)),
		_mm_set1_ps((float)BOUNDARY_RECT.top)
	};

	// lookup table for left packing
	const __m128i SHUF_TBL[SHUF_MAX]
	{
	  _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0),//mask0  0000 to 0000 4
	  _mm_set_epi8(3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4),//mask1  0001 to 0001 3
	  _mm_set_epi8(7,6,5,4,15,14,13,12,11,10,9,8,3,2,1,0),//mask2  0010 to 0001 3
	  _mm_set_epi8(7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8),//mask3  0011 to 0011 2
	  _mm_set_epi8(11,10,9,8,15,14,13,12,7,6,5,4,3,2,1,0),//mask4  0100 to 0001 3
	  _mm_set_epi8(11,10,9,8,3,2,1,0,15,14,13,12,7,6,5,4),//mask5  0101 to 0011 2
	  _mm_set_epi8(11,10,9,8,7,6,5,4,15,14,13,12,3,2,1,0),//mask6  0110 to 0011 2
	  _mm_set_epi8(11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12),//mask7  0111 to 0111 1
	  _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0),//mask8  1000 to 0001 3
	  _mm_set_epi8(15,14,13,12,3,2,1,0,11,10,9,8,7,6,5,4),//mask9  1001 to 0011 2
	  _mm_set_epi8(15,14,13,12,7,6,5,4,11,10,9,8,3,2,1,0),//maska  1010 to 0011 2
	  _mm_set_epi8(15,14,13,12,7,6,5,4,3,2,1,0,11,10,9,8),//maskb  1011 to 0111 1
	  _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0),//maskc  1100 to 0011 2
	  _mm_set_epi8(15,14,13,12,11,10,9,8,3,2,1,0,7,6,5,4),//maskd  1101 to 0111 1
	  _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0),//maske  1110 to 0111 1
	  _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)	//maskf  1111 to 1111 1
	};

	// increment last left packed element for subsequent loops
	const uint32_t SHUF_MOVES[SHUF_MAX]
	{
	  4,    //1,
	  3,    //1,
	  3,    //1,
	  2,    //2,
	  3,    //1,
	  2,    //2,
	  2,    //2,
	  1,    //3,
	  3,    //1,
	  2,    //2,
	  2,    //2,
	  1,    //3,
	  2,    //2,
	  1,    //3,
	  1,    //3,
	  1,    //4,
	};

	const uint32_t PACK_LIMIT = NUM_FIGHTERS_SIMD - 4;

#else
	const char APP_NAME[] = "Scalar Streets";
#endif // 

}