#include "SceneObjects.h"
#include <cassert>
#include "Constants.h"
#include "Util.h"
#include <smmintrin.h>

void SceneObjects::Init()
{
    // init fighter data
    {
#ifdef SIMD
        // init blue
        for (uint32_t i = 0; i < gc::NUM_BLUE_SCALAR; i++)
        {
            fighterSOA.posX[i] = RandF(gc::BOUNDARY_RECT.left, gc::BOUNDARY_RECT.width);
            fighterSOA.posY[i] = RandF(gc::BOUNDARY_RECT.top, gc::BOUNDARY_RECT.height);
            fighterSOA.velX[i] = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterSOA.velY[i] = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterSOA.team[i] = gc::BLUE_TEAM;
            fighterSOA.member[i] = i % gc::MEMBER_COUNT;
            fighterSOA.alive[i] = 1;
            // todo amend 
            fighterSOA.frameOffset[i] = i % gc::NUM_FIGHTER_FRAMES;
            // todo amend 
            fighterSOA.frameNum[i] = i % gc::NUM_FIGHTER_FRAMES;
        }

        //  init purple
        for (uint32_t i = gc::NUM_BLUE_SCALAR; i < gc::NUM_FIGHTERS_SCALAR; i++)
        {
            fighterSOA.posX[i] = RandF(gc::BOUNDARY_RECT.left, gc::BOUNDARY_RECT.width);
            fighterSOA.posY[i] = RandF(gc::BOUNDARY_RECT.top, gc::BOUNDARY_RECT.height);
            fighterSOA.velX[i] = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterSOA.velY[i] = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterSOA.team[i] = gc::PURPLE_TEAM;
            fighterSOA.member[i] = i % gc::MEMBER_COUNT;
            fighterSOA.alive[i] = 1;
            //todo amend
            fighterSOA.frameOffset[i] = i % gc::NUM_FIGHTER_FRAMES;
            // todo amend 
            fighterSOA.frameNum[i] = i % gc::NUM_FIGHTER_FRAMES;
        }

        SetToTeamStartIndex(FighterIndices::BLUE);



#else // scalar



#endif
    }

    // init texture and sprites
    {
        if (!texture.loadFromFile(gc::TEX_PATH, gc::TEX_RECT))
        {
            assert(false);
        }
        texture.setRepeated(true);

        bgSprite.setTexture(texture);
        bgSprite.setTextureRect(gc::SPR_RECT_BG);

        for (uint32_t i = 0; i < gc::NUM_FIGHTERS_SCALAR; ++i)
        {
            fighterSprites[i].setTexture(texture);
            // todo change to reference fighter data
            fighterSprites[i].setPosition({ fighterSOA.posX[i],fighterSOA.posY[i] });

            fighterSprites[i].setTextureRect(
                {
                    (int)(gc::ANIM_X_OFFSET + fighterSOA.frameOffset[i] * gc::FIGHTER_W),
                    (int)(gc::ANIM_Y_OFFSET + ((fighterSOA.team[i] * 4U) + fighterSOA.member[i]) * gc::FIGHTER_H),
                    (int)(gc::FIGHTER_W),
                    (int)(gc::FIGHTER_H)
                });

        }
    }

}

void SceneObjects::Update(float dt)
{
#ifdef SIMD
    
    
    const __m128 m128_ELAPSED_SECS = _mm_set1_ps(dt);

    UpdateCollisionSIMD();
    UpdateLeftPackingMovementSIMD(m128_ELAPSED_SECS, gc::m128_BOUNDS);

    SetToTeamStartIndex(FighterIndices::BLUE);

    fighterIndices.set_m128();

    const size_t ENDLOOP = last_left_element >> 2;
    for (size_t i = 0; i < ENDLOOP; i++)
    {
        UpdateAxisInBounds(fighterIndices.p_m128_pos_x, fighterIndices.p_m128_vel_x, gc::m128_BOUNDS[gc::MIN_X], gc::m128_BOUNDS[gc::MAX_X], m128_ELAPSED_SECS);
        UpdateAxisInBounds(fighterIndices.p_m128_pos_y, fighterIndices.p_m128_vel_y, gc::m128_BOUNDS[gc::MIN_Y], gc::m128_BOUNDS[gc::MAX_Y], m128_ELAPSED_SECS);
        fighterIndices.inc_m128();
    }


    // update sprites
    {
        for (uint32_t i = 0; i < gc::NUM_FIGHTERS_SCALAR; i++)
        {
            
        }
    }


#else // SCALAR

#endif // SIMD

}

void SceneObjects::Draw(sf::RenderWindow& window)
{
    window.draw(bgSprite);
    // order of appearance: Dead Purple, Alive Purple, Blue
    for(int i = (int)gc::NUM_FIGHTERS_SCALAR-1; i >= 0; --i)
    {
        window.draw(fighterSprites[i]);
    }
}


#ifdef SIMD

void SceneObjects::UpdateCollisionSIMD()
{
    // for validation
    uint32_t result;

    SetToTeamStartIndex(FighterIndices::BLUE);
    result = fighterIndices.validate();

    if (result) assert(false);
    
    // end condition == gc::NUM_BLUE_SCALAR

    uint32_t blueStart = fighterIndices.start[FighterIndices::BLUE];
    uint32_t blueEnd = fighterIndices.start[FighterIndices::PURPLE];
    for (uint32_t b = blueStart; b < blueEnd; ++b)
    {
        // duplicate blue across 4 lanes so as every blue must be compared with purple simd
        __m128 blue_pos_x = _mm_set1_ps(*fighterIndices.p_pos_x);
        __m128 blue_pos_y = _mm_set1_ps(*fighterIndices.p_pos_y);
        __m128 blue_vel_x = _mm_set1_ps(*fighterIndices.p_vel_x);
        __m128 blue_vel_y = _mm_set1_ps(*fighterIndices.p_vel_y);
        __m128i blue_team_id = _mm_set1_epi32(*fighterIndices.p_team_id);
        __m128i blue_member = _mm_set1_epi32(*fighterIndices.p_member);
        __m128i blue_frame_offset = _mm_set1_epi32(*fighterIndices.p_frame_offset);
        __m128i blue_frame_num = _mm_set1_epi32(*fighterIndices.p_frame_num);
        __m128i blue_is_alive = _mm_set1_epi32(*fighterIndices.p_is_alive);


        // reset purple indices with offset
        SetToTeamStartIndex(FighterIndices::PURPLE);

        result = fighterIndices.validate();

        if (result)assert(false);

        // blue interacts with 4 purple members in each iteration
        uint32_t purpleStart = gc::NUM_BLUE_SIMD;
        uint32_t purpleEnd = gc::NUM_FIGHTERS_SIMD;

        for (uint32_t p = gc::NUM_BLUE_SIMD; p < gc::NUM_FIGHTERS_SIMD; p++)
        {

            const __m128 x = *fighterIndices.p_m128_pos_x;
            const __m128 y = *fighterIndices.p_m128_pos_y;
            // calc squ distance between b and p
            __m128 square_dist = DistanceSquaredSIMD(
                blue_pos_x,
                blue_pos_y,
                x,
                y);

            // purple ko'ed if within range of blue
            {
                // if distance is within range
                // casting has zero latency
                __m128i mask = _mm_castps_si128(_mm_cmplt_ps(square_dist, gc::SQU_DISTANCE_BETWEEN_FIGHTERS_PS));

                *fighterIndices.p_m128i_is_alive = _mm_blendv_epi8(*fighterIndices.p_m128i_is_alive, gc::ZERO_EPI, mask);
                *fighterIndices.p_m128i_frame_num = _mm_blendv_epi8(*fighterIndices.p_m128i_frame_num, gc::DEAD_NUM_FRAME_EPI, mask);
                *fighterIndices.p_m128i_frame_offset = _mm_blendv_epi8(*fighterIndices.p_m128i_frame_offset, gc::DEAD_FAME_OFFSET_EPI, mask);

                // modify vel based on mask
                *fighterIndices.p_m128_vel_x = _mm_blendv_ps(*fighterIndices.p_m128_vel_x, gc::ZERO_PS, _mm_castsi128_ps(mask));
                *fighterIndices.p_m128_vel_y = _mm_blendv_ps(*fighterIndices.p_m128_vel_y, gc::ZERO_PS, _mm_castsi128_ps(mask));
            }
            
            // inc purple simd
            fighterIndices.inc_m128();
        }
        // inc blue single
        fighterIndices.int_single();
    }

}

void SceneObjects::UpdateLeftPackingMovementSIMD(const __m128& m128_elapsed_secs, const __m128 m128_bounds[4])
{
    // break loop condition
    bool next = true;
    // lookup for shuffle table
    int mask_lookup = 0;
    uint32_t inc = 0;

    // result of alive comparision
    __m128i mask;

    SetToTeamStartIndex(FighterIndices::BLUE);

    while (next)
    {
        fighterIndices.set_m128();

        mask = _mm_cmpeq_epi32(*fighterIndices.p_m128i_is_alive, gc::ZERO_EPI);
        mask_lookup = _mm_movemask_ps(_mm_castsi128_ps(mask));

        assert(mask_lookup < gc::SHUF_MAX);

        // shuffle
        {
            // float values
            *fighterIndices.p_m128_pos_x = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(*fighterIndices.p_m128_pos_x), gc::SHUF_TBL[mask_lookup]));
            *fighterIndices.p_m128_pos_y = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(*fighterIndices.p_m128_pos_y), gc::SHUF_TBL[mask_lookup]));
            *fighterIndices.p_m128_vel_x = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(*fighterIndices.p_m128_vel_x), gc::SHUF_TBL[mask_lookup]));
            *fighterIndices.p_m128_vel_y = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(*fighterIndices.p_m128_vel_y), gc::SHUF_TBL[mask_lookup]));
            // integer values
            *fighterIndices.p_m128i_team_id = _mm_shuffle_epi8(*fighterIndices.p_m128i_team_id, gc::SHUF_TBL[mask_lookup]);
            *fighterIndices.p_m128i_member = _mm_shuffle_epi8(*fighterIndices.p_m128i_member, gc::SHUF_TBL[mask_lookup]);
            *fighterIndices.p_m128i_frame_offset = _mm_shuffle_epi8(*fighterIndices.p_m128i_frame_offset, gc::SHUF_TBL[mask_lookup]);
            *fighterIndices.p_m128i_frame_num = _mm_shuffle_epi8(*fighterIndices.p_m128i_frame_num, gc::SHUF_TBL[mask_lookup]);
            *fighterIndices.p_m128i_is_alive = _mm_shuffle_epi8(*fighterIndices.p_m128i_is_alive, gc::SHUF_TBL[mask_lookup]);
        }

        // loop inc and break
        if (inc == gc::PACK_LIMIT || inc >= gc::NUM_FIGHTERS_SCALAR)
        {
            // reset position and break loop
            inc = 0;
            next = false;
        }
        else
        {
            // continue
            if (inc + gc::SHUF_MOVES[mask_lookup] < gc::PACK_LIMIT)
            {
                // move appropriate positions
                inc += gc::SHUF_MOVES[mask_lookup];
            }
            else
            {
                // snap to last 4 elements
                inc = gc::PACK_LIMIT;
            }

            // update last left packed element
            if (mask_lookup < gc::SHUF_MAX - 1)
            {
                last_left_element = inc;
            }
        }

        assert(inc <= gc::PACK_LIMIT);

        // inc from start
        SetToTeamStartIndex(FighterIndices::BLUE, inc);
    }

}

void SceneObjects::UpdateAxisInBounds(__m128* axis_pos, __m128* axis_vel, const __m128& min, const __m128& max, const __m128& elapsed_secs)
{
    assert(axis_pos && axis_vel);
    // calc position
    const __m128 vel = _mm_mul_ps(*axis_vel, elapsed_secs);
    __m128 next_pos = _mm_add_ps(*axis_pos, vel);

    // compare position with boundary
    const __m128 mask_min = _mm_cmplt_ps(next_pos, min);
    const __m128 mask_max = _mm_cmpgt_ps(next_pos, max); // next_pos no longer needed
    const __m128 mask = _mm_or_ps(mask_max, mask_min);

    // generate multiplier for flipping velocity where next position outside of boundary
    const __m128 multiplier = _mm_blendv_ps(gc::ONE_PS, gc::NEG_ONE_PS, mask);

    // limit position
    next_pos = _mm_min_ps(next_pos, max);
    next_pos = _mm_max_ps(next_pos, min);

    // update source position and velocity
    *axis_vel = _mm_mul_ps(*axis_vel, multiplier);
    *axis_pos = next_pos;
}

void SceneObjects::SetToTeamStartIndex(uint32_t index, uint32_t offset)
{
    assert(index < FighterIndices::COUNT);

    uint32_t i = fighterIndices.start[index] + offset;
    // points to specific offset in soa
    fighterIndices.p_pos_x = fighterSOA.posX + i;
    fighterIndices.p_pos_y = fighterSOA.posY + i;
    fighterIndices.p_vel_x = fighterSOA.velX + i;
    fighterIndices.p_vel_y = fighterSOA.velY + i;
    fighterIndices.p_team_id = fighterSOA.team + i;
    fighterIndices.p_member = fighterSOA.member + i;
    fighterIndices.p_frame_offset = fighterSOA.frameOffset + i;
    fighterIndices.p_frame_num = fighterSOA.frameNum + i;
    fighterIndices.p_is_alive = fighterSOA.alive + i;

    // line up __m128(i)
    fighterIndices.set_m128();

}

__m128 SceneObjects::DistanceSquaredSIMD(const __m128& a_x, const __m128& a_y,   const  __m128& b_x, const __m128& b_y)
{
    // a - b
    __m128 x = _mm_sub_ps(a_x, b_x);
    __m128 y = _mm_sub_ps(a_y, b_y);

    // x^2, y^2
    x = _mm_mul_ps(x, x);
    y = _mm_mul_ps(y, y);

    // x + b
    return _mm_add_ps(x, y);
}



#endif