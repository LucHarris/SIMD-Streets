#include "SceneObjects.h"
#include <cassert>
#include "Util.h"
#include <smmintrin.h>
#include <iostream>
#include <string>
#ifdef  DEBUG_FILE_OUT
#include <chrono>
#endif

void SceneObjects::Init()
{
    
#ifdef DEBUG_FILE_OUT
    for (size_t i = 0; i < gc::debug::COUNT; i++)
    {
        fileout[i].open(std::to_string(debugID) + std::string(gc::DEBUG_FILENAME[i]),std::ios::app);

        if (!fileout[i].is_open())
        {
            assert(false);
        }
    }

#endif // DEBUG_FILE_OUT



    // init fighter data
    {
#ifdef SIMD
        // init fighters AOS
        for (uint32_t i = 0; i < gc::NUM_FIGHTERS_SCALAR; i++)
        {
            fighterSOA.posX[i] = RandF(gc::BOUNDARY_RECT.left, gc::BOUNDARY_RECT.width);
            fighterSOA.posY[i] = RandF(gc::BOUNDARY_RECT.top, gc::BOUNDARY_RECT.height);
            fighterSOA.velX[i] = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterSOA.velY[i] = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterSOA.team[i] = (i < gc::NUM_BLUE_SCALAR) ? gc::BLUE_TEAM : gc::PURPLE_TEAM;
            fighterSOA.member[i] = i % gc::MEMBER_COUNT;
            fighterSOA.alive[i] = 1;
            fighterSOA.frameNum[i] = anim_data::ATTACK;
            fighterSOA.frameOffset[i] = i % gc::ANIM_DATA[fighterSOA.frameNum[i]].num_frames;
        }

        SetToTeamStartIndex(FighterIndices::BLUE);


#else // scalar
        for (uint32_t i = 0; i < gc::NUM_FIGHTERS_SCALAR; i++)
        {
            fighterAOS.data[i].posX = RandF(gc::BOUNDARY_RECT.left, gc::BOUNDARY_RECT.width);
            fighterAOS.data[i].posY = RandF(gc::BOUNDARY_RECT.top, gc::BOUNDARY_RECT.height);
            fighterAOS.data[i].velX = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterAOS.data[i].velY = RandF(-gc::FIGHTER_SPEED, gc::FIGHTER_SPEED);
            fighterAOS.data[i].team = (i < gc::NUM_BLUE_SCALAR)?gc::BLUE_TEAM:gc::PURPLE_TEAM;
            fighterAOS.data[i].member = i % gc::MEMBER_COUNT;
            fighterAOS.data[i].alive = 1;
            // todo amend 
            fighterAOS.data[i].frameNum = anim_data::ATTACK;
            fighterAOS.data[i].frameOffset = i % gc::ANIM_DATA[fighterAOS.data[i].frameNum].num_frames;
        }

    
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
            fighterSprites[i].setOrigin({ (float)(gc::FIGHTER_W >> 1), (float)(gc::FIGHTER_H >> 1) });
#ifdef SIMD
            // assign initial simd data to sprite
            fighterSprites[i].setPosition({ fighterSOA.posX[i],fighterSOA.posY[i] });

            fighterSprites[i].setTextureRect(
                {
                    (int)(gc::ANIM_X_OFFSET + fighterSOA.frameOffset[i] * gc::FIGHTER_W),
                    (int)(gc::ANIM_Y_OFFSET + ((fighterSOA.team[i] * 4U) + fighterSOA.member[i]) * gc::FIGHTER_H),
                    (int)(gc::FIGHTER_W),
                    (int)(gc::FIGHTER_H)
                });
#else
            //todo  initialise scalar data to sprite
            fighterSprites[i].setPosition({ fighterAOS.data[i].posX,fighterAOS.data[i].posY });
            fighterSprites[i].setTextureRect(
                {
                    (int)(gc::ANIM_X_OFFSET + fighterAOS.data[i].frameOffset * gc::FIGHTER_W),
                    (int)(gc::ANIM_Y_OFFSET + ((fighterAOS.data[i].team * 4U) + fighterAOS.data[i].member) * gc::FIGHTER_H),
                    (int)(gc::FIGHTER_W),
                    (int)(gc::FIGHTER_H)
                });

#endif

        }
    }

}

void SceneObjects::Update(float dt)
{
    uint32_t updateTimer = 0;
    std::string testType;


    uint32_t aliveCount = 0;



#ifdef SIMD
    // reset timer
    {
#ifdef SIMD_LEFT_PACKING
        testType = "SIMD_LP";
#else
        testType = "SIMD";
#endif
        loopTimer.reset();
    }

    _declspec(align(16)) const __m128 m128_ELAPSED_SECS = _mm_set1_ps(dt);
    // blue collide with purple
    UpdateCollisionSIMD();
    // reset indices to start of SOA structure
    SetToTeamStartIndex(FighterIndices::BLUE);

#ifdef SIMD_LEFT_PACKING
    // sorting 'down' purple elements to the right
    // updates last_left_element for break condition
    UpdateLeftPackingMovementSIMD(m128_ELAPSED_SECS, gc::m128_BOUNDS);
    // reset indices to start of SOA structure
    SetToTeamStartIndex(FighterIndices::BLUE);
    // 32bit to 128bit position
    const size_t ENDLOOP = last_left_element >> 2;
#else // !SIMD_LEFT_PACKING
    const size_t ENDLOOP = gc::NUM_FIGHTERS_SIMD;
#endif // SIMD_LEFT_PACKING

    for (size_t i = 0; i < ENDLOOP; i++)
    {
        // update x position and velocity
        UpdateAxisInBounds(fighterIndices.p_m128_pos_x, fighterIndices.p_m128_vel_x, gc::m128_BOUNDS[gc::MIN_X], gc::m128_BOUNDS[gc::MAX_X], m128_ELAPSED_SECS);
        // update y position and velocity
        UpdateAxisInBounds(fighterIndices.p_m128_pos_y, fighterIndices.p_m128_vel_y, gc::m128_BOUNDS[gc::MIN_Y], gc::m128_BOUNDS[gc::MAX_Y], m128_ELAPSED_SECS);
        // increment 128 bits
        fighterIndices.inc_m128();
    }

    // get time
    updateTimer = loopTimer.get_elapsed_time().count();




    // update sprites
    {
        for (uint32_t i = 0; i < gc::NUM_FIGHTERS_SCALAR; i++)
        {
            fighterSOA.frameOffset[i] = (fighterSOA.frameOffset[i] + 1) % gc::ANIM_DATA[fighterSOA.frameNum[i]].num_frames;

            fighterSprites[i].setPosition(fighterSOA.posX[i], fighterSOA.posY[i]);
            fighterSprites[i].setTextureRect(
                {
                    (int)(gc::ANIM_X_OFFSET + (fighterSOA.frameOffset[i] + gc::ANIM_DATA[fighterSOA.frameNum[i]].offset_frame) * gc::FIGHTER_W),
                    (int)(gc::ANIM_Y_OFFSET + ((fighterSOA.team[i] * 4U) + fighterSOA.member[i]) * gc::FIGHTER_H),
                    (int)(gc::FIGHTER_W),
                    (int)(gc::FIGHTER_H)
                });

            if (fighterSOA.alive[i])
            {
                ++aliveCount;
            }

        }
    }


#else // SCALAR

    // reset timer
    {
        testType = "SCALAR";
        loopTimer.reset();
    }

    UpdateCollisionsScalar();
    UpdateSortScalar(dt);
    UpdateAxisInBoundsScalar(dt);

    // get time
    updateTimer = loopTimer.get_elapsed_time().count();

    // update sprite data
    for (uint32_t i = 0; i < gc::NUM_FIGHTERS_SCALAR; i++)
    {
        fighterAOS.data[i].frameOffset =( fighterAOS.data[i].frameOffset + 1) % gc::ANIM_DATA[fighterAOS.data[i].frameNum].num_frames;

        fighterSprites[i].setPosition(fighterAOS.data[i].posX, fighterAOS.data[i].posY);
        fighterSprites[i].setTextureRect(
            {
                (int)(gc::ANIM_X_OFFSET + (fighterAOS.data[i].frameOffset + gc::ANIM_DATA[fighterAOS.data[i].frameNum].offset_frame) *  gc::FIGHTER_W),
                (int)(gc::ANIM_Y_OFFSET + ((fighterAOS.data[i].team * 4U) + fighterAOS.data[i].member) * gc::FIGHTER_H),
                (int)(gc::FIGHTER_W),
                (int)(gc::FIGHTER_H)
            });

        if (fighterAOS.data[i].alive)
        {
            ++aliveCount;
        }
    }

#endif // SIMD

    records.ToFile(testType.c_str(), updateTimer, gc::NUM_FIGHTERS_SCALAR, gc::NUM_BLUE_SCALAR, gc::NUM_PURPLE_SCALAR, aliveCount);

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

void SceneObjects::Release()
{
#ifdef DEBUG_FILE_OUT 
    for (size_t i = 0; i < gc::debug::COUNT; i++)
    {
        fileout[i].close();
    }
#endif
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
        {

        }
        // duplicate blue across 4 lanes so as every blue must be compared with purple simd
        _declspec(align(16)) __m128 blue_pos_x = _mm_set1_ps(*fighterIndices.p_pos_x);
        _declspec(align(16)) __m128 blue_pos_y = _mm_set1_ps(*fighterIndices.p_pos_y);
        _declspec(align(16)) __m128 blue_vel_x = _mm_set1_ps(*fighterIndices.p_vel_x);
        _declspec(align(16)) __m128 blue_vel_y = _mm_set1_ps(*fighterIndices.p_vel_y);
        _declspec(align(16)) __m128i blue_team_id = _mm_set1_epi32(*fighterIndices.p_team_id);
        _declspec(align(16)) __m128i blue_member = _mm_set1_epi32(*fighterIndices.p_member);
        _declspec(align(16)) __m128i blue_frame_offset = _mm_set1_epi32(*fighterIndices.p_frame_offset);
        _declspec(align(16)) __m128i blue_frame_num = _mm_set1_epi32(*fighterIndices.p_frame_num);
        _declspec(align(16)) __m128i blue_is_alive = _mm_set1_epi32(*fighterIndices.p_is_alive);

        // reset purple indices 
        SetM128(FighterIndices::PURPLE);

        result = fighterIndices.validate();

        if (result)assert(false);

        // blue interacts with 4 purple members in each iteration
        uint32_t purpleStart = gc::NUM_BLUE_SIMD;
        uint32_t purpleEnd = gc::NUM_FIGHTERS_SIMD;

        for (uint32_t p = gc::NUM_BLUE_SIMD; p < gc::NUM_FIGHTERS_SIMD; p++)
        {

            _declspec(align(16)) __m128 x = *fighterIndices.p_m128_pos_x;
            _declspec(align(16)) __m128 y = *fighterIndices.p_m128_pos_y;
            // calc squ distance between b and p
            _declspec(align(16)) __m128 square_dist = DistanceSquaredSIMD(
                blue_pos_x,
                blue_pos_y,
                x,
                y);

            // purple ko'ed if within range of blue
            {
                // if distance is within range
                // casting has zero latency
                _declspec(align(16)) __m128i mask = _mm_castps_si128(_mm_cmplt_ps(square_dist, gc::SQU_DISTANCE_BETWEEN_FIGHTERS_PS));

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
    _declspec(align(16)) __m128i mask;
    // point to start of data
    SetToTeamStartIndex(FighterIndices::BLUE);

    while (next)
    {
        // stepped from end of previous loop
        fighterIndices.set_m128();
        // mask based on if fighter is alive
        mask = _mm_cmpeq_epi32(*fighterIndices.p_m128i_is_alive, _mm_set1_epi32(0));// gc::ZERO_EPI);
        // shuffle mask
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
    _declspec(align(16)) const __m128 vel = _mm_mul_ps(*axis_vel, elapsed_secs);
    _declspec(align(16)) __m128 next_pos = _mm_add_ps(*axis_pos, vel);

    // compare position with boundary
    _declspec(align(16)) const __m128 mask_min = _mm_cmplt_ps(next_pos, min);
    _declspec(align(16)) const __m128 mask_max = _mm_cmpgt_ps(next_pos, max); // next_pos no longer needed
    _declspec(align(16)) const __m128 mask = _mm_or_ps(mask_max, mask_min);

    // generate multiplier for flipping velocity where next position outside of boundary
    _declspec(align(16)) const __m128 multiplier = _mm_blendv_ps(gc::ONE_PS, gc::NEG_ONE_PS, mask);

    // limit position
    next_pos = _mm_min_ps(next_pos, max);
    next_pos = _mm_max_ps(next_pos, min);

    // update source position and velocity
    *axis_vel = _mm_mul_ps(*axis_vel, multiplier);
    *axis_pos = next_pos;
}

void SceneObjects::SetToTeamStartIndex(uint32_t team, uint32_t offset)
{
    assert(team < FighterIndices::COUNT);

    uint32_t i = fighterIndices.start[team] + offset;
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
    _declspec(align(16)) __m128 x = _mm_sub_ps(a_x, b_x);
    _declspec(align(16)) __m128 y = _mm_sub_ps(a_y, b_y);

    // x^2, y^2
    x = _mm_mul_ps(x, x);
    y = _mm_mul_ps(y, y);

    // x + b
    return _mm_add_ps(x, y);
}

void SceneObjects::SetM128(uint32_t team, uint32_t simdOffset)
{
    uint32_t i = fighterIndices.start[team] + (simdOffset << 2);

    fighterIndices.p_m128_pos_x = (__m128*)(fighterSOA.posX + i);
    fighterIndices.p_m128_pos_y = (__m128*)(fighterSOA.posY + i);
    fighterIndices.p_m128_vel_x = (__m128*)(fighterSOA.velX + i);
    fighterIndices.p_m128_vel_y = (__m128*)(fighterSOA.velY + i);
    fighterIndices.p_m128i_team_id = (__m128i*)(fighterSOA.team + i);
    fighterIndices.p_m128i_member = (__m128i*)(fighterSOA.member + i);
    fighterIndices.p_m128i_frame_offset = (__m128i*)(fighterSOA.frameOffset + i);
    fighterIndices.p_m128i_frame_num = (__m128i*)(fighterSOA.frameNum + i);
    fighterIndices.p_m128i_is_alive = (__m128i*)(fighterSOA.alive + i);
}


#else // scalar



void SceneObjects::UpdateCollisionsScalar()
{

    // compare distance of each blue...
    for (uint32_t blue = 0; blue < gc::NUM_BLUE_SCALAR; ++blue)
    {
        // ... to each purple
        for (uint32_t purple = gc::NUM_BLUE_SCALAR; purple < gc::NUM_FIGHTERS_SCALAR; ++purple)
        {
            float squDist = DistanceSquaredScalar(
                fighterAOS.data[blue].posX,
                fighterAOS.data[blue].posY,
                fighterAOS.data[purple].posX,
                fighterAOS.data[purple].posY );

            if (squDist < gc::SQUARE_DISTANCE_BETWEEN_FIGHTERS)
            {
                fighterAOS.data[purple].alive = 0;
                fighterAOS.data[purple].frameNum = anim_data::DOWN;
                fighterAOS.data[purple].frameOffset = 0;
            }
        }
    }
}

void SceneObjects::UpdateSortScalar(float elapsed_secs)
{
    std::sort(fighterAOS.data, fighterAOS.data + gc::NUM_FIGHTERS_SCALAR, [](FighterScalar& a, FighterScalar& b)
        {
            return a.alive > b.alive;
        });
}

void SceneObjects::UpdateAxisInBoundsScalar(float elapsed_secs)
{
    bool alive = true;
    uint32_t i = 0;
    while (alive && i < gc::NUM_FIGHTERS_SCALAR)
    {
        if (fighterAOS.data[i].alive)
        {
            // calc velocity and next position
            float velX = fighterAOS.data[i].velX * elapsed_secs;
            float velY = fighterAOS.data[i].velY * elapsed_secs;
            float nextPosX = fighterAOS.data[i].posX + velX;
            float nextPosY = fighterAOS.data[i].posY + velY;

            // flip on boundary hit
            if (nextPosX < gc::BOUNDARY_RECT.left || nextPosX > gc::BOUNDARY_RECT.width)
            {
                fighterAOS.data[i].velX *= -1;
            }
            if (nextPosY < gc::BOUNDARY_RECT.top || nextPosY > gc::BOUNDARY_RECT.height)
            {
                fighterAOS.data[i].velY *= -1;
            }
            // limit x
            nextPosX = std::max(nextPosX, gc::BOUNDARY_RECT.left);
            nextPosX = std::min(nextPosX, gc::BOUNDARY_RECT.width);
            // limit y
            nextPosY = std::max(nextPosY, gc::BOUNDARY_RECT.top);
            nextPosY = std::min(nextPosY, gc::BOUNDARY_RECT.height);
            // update fighter data for alive fighters
            fighterAOS.data[i].posX = nextPosX;
            fighterAOS.data[i].posY = nextPosY;
        }
        else
        {
            // break loop
            alive = false;
        }
        ++i;
    }
}

float SceneObjects::DistanceSquaredScalar(float a_x, float a_y, float b_x, float b_y)
{
    // a - b
    float x = a_x - b_x;
    float y = a_y - b_y;
    // x^2 + y^2
    x *= x;
    y *= y;
    return x + y;
}

#endif // simd