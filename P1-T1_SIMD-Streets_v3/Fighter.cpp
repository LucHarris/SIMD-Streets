#include "Fighter.h"
#include "Constants.h"

#ifdef SIMD


FighterIndices::FighterIndices()
	:
	start{ 0,gc::NUM_BLUE_SCALAR }
{
}

void FighterIndices::inc_m128() volatile
{
	++p_m128_pos_x;
	++p_m128_pos_y;
	++p_m128_vel_x;
	++p_m128_vel_y;
	++p_m128i_team_id;
	++p_m128i_member;
	++p_m128i_frame_offset;
	++p_m128i_frame_num;
	++p_m128i_is_alive;
}

void FighterIndices::int_single() volatile
{
	++p_pos_x;
	++p_pos_y;
	++p_vel_x;
	++p_vel_y;
	++p_team_id;
	++p_member;
	++p_frame_offset;
	++p_frame_num;
	++p_is_alive;
}

void FighterIndices::step_single_and_set(uint32_t num_steps) volatile
{
	p_pos_x = p_pos_x + num_steps;
	p_pos_y = p_pos_y + num_steps;
	p_vel_x = p_vel_x + num_steps;
	p_vel_y = p_vel_y + num_steps;
	p_team_id = p_team_id + num_steps;
	p_member = p_member + num_steps;
	p_frame_offset = p_frame_offset + num_steps;
	p_frame_num = p_frame_num + num_steps;
	p_is_alive = p_is_alive + num_steps;
}

void FighterIndices::set_m128() volatile
{
	p_m128_pos_x = (__m128*)p_pos_x;
	p_m128_pos_y = (__m128*)p_pos_y;
	p_m128_vel_x = (__m128*)p_vel_x;
	p_m128_vel_y = (__m128*)p_vel_y;
	p_m128i_team_id = (__m128i*)p_team_id;
	p_m128i_member = (__m128i*)p_member;
	p_m128i_frame_offset = (__m128i*)p_frame_offset;
	p_m128i_frame_num = (__m128i*)p_frame_num;
	p_m128i_is_alive = (__m128i*)p_is_alive;
}

uint32_t FighterIndices::validate() volatile
{
	int result = 0;
	if (!p_pos_x) result = result & 0x01;
	if (!p_pos_y) result = result & 0x02;
	if (!p_vel_x) result = result & 0x04;
	if (!p_vel_y) result = result & 0x08;

	if (!p_m128_pos_x) result = result & 0x10;
	if (!p_m128_pos_y) result = result & 0x20;
	if (!p_m128_vel_x) result = result & 0x30;
	if (!p_m128_vel_y) result = result & 0x40;

	return result;
}

#else



#endif
