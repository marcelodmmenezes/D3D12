#pragma once

#include "config.hpp"

class RelativeTimer
{
	using Seconds = std::chrono::duration<float, std::ratio<1>>;
	static constexpr float seconds_per_count = 1.0f / 1000000000.0f;

public:
	RelativeTimer()
	{
		reset();
	}

	float totalTime() const
	{
		if (stopped)
		{
			return ((stop_time - base_time).count() - paused_time) * seconds_per_count;
		}
		else
		{
			return ((curr_time - base_time).count() - paused_time) * seconds_per_count;
		}
	}
	
	float deltaTime() const
	{
		return delta_time;
	}

	void reset()
	{
		curr_time = std::chrono::high_resolution_clock::now();
		base_time = curr_time;
		prev_time = curr_time;
		paused_time = 0u;
		stopped = false;
	}

	void start()
	{
		auto start_time = std::chrono::high_resolution_clock::now();

		if (stopped)
		{
			paused_time += (start_time - stop_time).count();
			prev_time = start_time;
			stopped = false;
		}
	}
	
	void stop()
	{
		if (!stopped)
		{
			curr_time = std::chrono::high_resolution_clock::now();
			stop_time = curr_time;
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

		curr_time = std::chrono::high_resolution_clock::now();
		delta_time = std::chrono::duration_cast<Seconds>(curr_time - prev_time).count();
		prev_time = curr_time;
	}

private:
	std::chrono::high_resolution_clock::time_point prev_time;
	std::chrono::high_resolution_clock::time_point curr_time;
	double delta_time;

	std::chrono::high_resolution_clock::time_point base_time;
	std::chrono::high_resolution_clock::time_point stop_time;
	uint64_t paused_time;

	bool stopped;
};
