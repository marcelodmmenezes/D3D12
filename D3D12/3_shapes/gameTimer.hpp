#pragma once

#include "config.hpp"

class GameTimer
{
public:
	GameTimer()
		:
		seconds_per_count{ 0.0 },
		delta_time{ -1.0 },
		base_time{ 0 },
		paused_time{ 0 },
		prev_time{ 0 },
		curr_time{ 0 },
		stopped{ false }
	{
		__int64 counts_per_second;

		QueryPerformanceFrequency(
			reinterpret_cast<LARGE_INTEGER*>(&counts_per_second));

		seconds_per_count = 1.0 / (double)counts_per_second;
	}

	float totalTime() const
	{
		if (stopped)
		{
			return static_cast<float>(
				((stop_time - paused_time) - base_time) * seconds_per_count);
		}
		else
		{
			return static_cast<float>(
				((curr_time - paused_time) - base_time) * seconds_per_count);
		}
	}

	float deltaTime() const
	{
		return static_cast<float>(delta_time);
	}

	void reset()
	{
		__int64 time;

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));

		base_time = time;
		prev_time = time;
		stop_time = 0;
		stopped = false;
	}

	void start()
	{
		__int64 time;
		
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));

		if (stopped)
		{
			paused_time += (time - stop_time);

			prev_time = time;
			stop_time = 0;
			stopped = false;
		}
	}

	void stop()
	{
		if (!stopped)
		{
			__int64 time;

			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));

			stop_time = time;
			stopped = true;
		}
	}

	void tick()
	{
		if (stopped)
		{
			delta_time = 0.0;
			return;
		}

		__int64 time;

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));

		curr_time = time;
		delta_time = (curr_time - prev_time) * seconds_per_count;
		prev_time = curr_time;

		if (delta_time < 0.0)
		{
			delta_time = 0.0;
		}
	}

private:
	double seconds_per_count;
	double delta_time;

	__int64 base_time;
	__int64 paused_time;
	__int64 stop_time;
	__int64 prev_time;
	__int64 curr_time;

	bool stopped;
};
