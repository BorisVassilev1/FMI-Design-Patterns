#pragma once

template <class T>
class Factory {
public:
	virtual T *create() = 0;
	virtual ~Factory() {};
};

