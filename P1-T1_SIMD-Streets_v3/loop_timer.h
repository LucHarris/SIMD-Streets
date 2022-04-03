#ifndef LOOP_TIMER_H
#define LOOP_TIMER_H

#include <chrono> // for std::chrono


using duration_secs_ui = std::chrono::duration <unsigned>;             // unsigned int seconds type
using duration_secs_f = std::chrono::duration <float>;                // float seconds type
using duration_msecs_ui = std::chrono::duration <unsigned, std::milli>; // unsigned int milliseconds type
using duration_msecs_f = std::chrono::duration <float, std::milli>; // float milliseconds type
using duration_misecs_ui = std::chrono::duration <unsigned, std::micro>; // unsigned int microseconds type
using duration_misecs_f = std::chrono::duration <float, std::micro>; // float microseconds type
using duration_nsecs_ui = std::chrono::duration <unsigned, std::nano>;  // unsigned int nanoseconds type
using duration_nsecs_f = std::chrono::duration <float, std::nano>;  // float nanoseconds type


// https://stackoverflow.com/a/41851068/3750310
template <class T>
struct loop_timer_is_chrono_duration : std::false_type {};
template <class rep, typename period>
struct loop_timer_is_chrono_duration <std::chrono::duration <rep, period>> : std::true_type {};


template <class duration = duration_secs_f>
class loop_timer
{
public:
	static_assert (loop_timer_is_chrono_duration <duration>::value, "duration must be of type std::chrono::duration");

	using clock = std::chrono::high_resolution_clock;

	loop_timer() : previous(clock::now()) { }

	duration get_elapsed_time()
	{
		clock::time_point const current = clock::now();
		return std::chrono::duration_cast <duration> (current - previous);
	}

	void reset()
	{
		previous = clock::now();
	}

private:
	clock::time_point previous;
};

#endif // LOOP_TIMER_H

/*
example 1: time task

loop_timer <duration_msecs_f> my_timer;

for (...)
{
	my_timer.reset ();
	{
		// DO WORK
	}
	auto const elapsed_time = my_timer.get_elapsed_time ();

	// use elapsed_time, delete as appropriate
	//auto const elapsed_secs_ui = std::chrono::duration_cast <duration_secs_ui> (elapsed_time).count ();
	//auto const elapsed_secs_f = std::chrono::duration_cast <duration_secs_f> (elapsed_time).count ();
	//auto const elapsed_msecs_ui = std::chrono::duration_cast <duration_msecs_ui> (elapsed_time).count ();
	//auto const elapsed_msecs_f = elapsed_time.count (); // no cast needed
	//auto const elapsed_nsecs_ui = std::chrono::duration_cast <duration_nsecs_ui> (elapsed_time).count ();
	//auto const elapsed_nsecs_f = std::chrono::duration_cast <duration_nsecs_f> (elapsed_time).count ();
}
*/

/*
example 2: calculate elapsed time since start of last loop

loop_timer <duration_secs_f> my_timer;

while (...)
{
	my_timer.reset ();
	float_t const elapsed_seconds = my_timer.get_elapsed_time ().count ();

	// DO WORK
	// USE elapsed_seconds
}
*/