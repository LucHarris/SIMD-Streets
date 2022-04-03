#pragma once

#include "Fighter.h"
#include <SFML/Graphics.hpp>


class SceneObjects
{
#ifdef SIMD
	FighterSOA fighterSOA;
	volatile FighterIndices fighterIndices;



	// left packing break condition
	uint32_t last_left_element = 0;
#else
	FighterAOS fighterAOS;
#endif // SIMD
	sf::Texture texture;
	sf::Sprite fighterSprites[gc::NUM_FIGHTERS_SCALAR];
	sf::Sprite bgSprite;


#ifdef DEBUG_FILE_OUT
	std::ofstream fileout[gc::debug::COUNT];
	time_t debugID = time(0);
#endif


public:
	void Init();
	void Update(float dt);
	void Draw(sf::RenderWindow& window);
	void Release();

#ifdef SIMD

	void UpdateCollisionSIMD();
	void UpdateLeftPackingMovementSIMD(const __m128& m128_elapsed_secs, const __m128 m128_bounds[4]);
	void UpdateAxisInBounds(__m128* axis_pos, __m128* axis_vel, const __m128& min, const __m128& max, const __m128& elapsed_secs);
	void SetToTeamStartIndex(uint32_t index, uint32_t offset = 0);
	__m128 DistanceSquaredSIMD(const __m128& a_x, const __m128& a_y, const __m128& b_x, const __m128& b_y);
	void SetM128(uint32_t team, uint32_t simdOffset = 0);

#else // scalar
	void UpdateCollisionsScalar();
	void UpdateSortScalar(float elapsed_secs);
	void UpdateAxisInBoundsScalar(float elapsed_secs);
	float DistanceSquaredScalar(float a_x,float a_y, float b_x,float b_y);
#endif // SIMD


};



