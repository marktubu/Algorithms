#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"

#include "logging/Logging.h"

#include <boost/bind.hpp>

#include <assert.h>
#include <sys/eventfd.h>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

static int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(evtfd < 0)
	{
		LOG_SYSERR << "Failed in eventfd";
		abort();
	}
	return evtfd;
}


EventLoop::EventLoop()
	:_looping(false),
	 _quit(false),
	 _callingPendingFunctors(false),
	 _threadId(CurrentThread::tid()),
	 _poller(new Poller(this)),
	 _timerQueue(new TimerQueue(this)),
	 _wakeupFd(createEventfd()),
	 _wakeupChannel(new Channel(this, _wakeupFd))
{
	LOG_TRACE << "EventLoop created " << this << " in thread " << _threadId;
	if(t_loopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << _threadId;
	}
	else
	{
		t_loopInThisThread = this;
	}

	_wakeupChannel->setReadCallback(boost::bind(&EventLoop::handleRead, this));
	_wakeupChannel->enableReading();
}

EventLoop::~EventLoop()
{
	assert(!_looping);
	::close(_wakeupFd);
	t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
	assert(!_looping);
	assertInLoopThread();

	_looping = true;
	_quit = false;

	while(!_quit)
	{
		_activeChannels.clear();
		_pollReturnTime = _poller->poll(kPollTimeMs, &_activeChannels);
		for(ChannelList::iterator it = _activeChannels.begin();
				it!=_activeChannels.end();++it)
		{
			(*it)->handleEvent();
		}
		doPendingFunctors();
	}

	LOG_TRACE << "EventLoop " << this << " stop looping";
	_looping = false;
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
	return _timerQueue->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return _timerQueue->addTimer(cb, time, interval);
}

void EventLoop::runInLoop(const Functor& cb)
{
	if(isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(cb);
	}
}

void EventLoop::queueInLoop(const Functor& func)
{
	{
		MutexLockGuard lock(_mutex);
		_pendingFunctors.push_back(func);
	}
	if(!isInLoopThread() || _callingPendingFunctors)
	{
		wakeup();
	}
}

void EventLoop::quit()
{
	_quit = true;
	if(!isInLoopThread())
	{
		wakeup();
	}
}

void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	_poller->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
		<< " was created in _threadId = " << _threadId 
		<< ", current thread id = " << CurrentThread::tid();
}

EventLoop* getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = ::write(_wakeupFd, &one, sizeof one);
	if(n != sizeof one)
	{
		LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
	}
}

void EventLoop::handleRead()
{	
	uint64_t one = 1;
	ssize_t n = ::read(_wakeupFd, &one, sizeof one);
	if(n != sizeof one)
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	_callingPendingFunctors = true;

	{
		MutexLockGuard lock(_mutex);
		functors.swap(_pendingFunctors);
	}
	
	for(size_t i = 0;i< functors.size(); ++i)
	{
		functors[i]();
	}

	_callingPendingFunctors = false;
}


