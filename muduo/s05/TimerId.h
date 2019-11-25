#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include "datetime/copyable.h"

namespace muduo
{

class Timer;

class TimerId : public muduo::copyable
{
public:
	explicit TimerId(Timer* timer) : _value(timer)
	{
	}

private:
	Timer* _value;
};

}


#endif
