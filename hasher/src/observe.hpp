#pragma once

#include <iostream>
#include <ostream>
#include <vector>
#include <algorithm>

template <class T>
class Observer {
   public:
	virtual void update(const T &) = 0;
	~Observer() {}
};

template <class T>
class Observable {
   public:
	virtual void addObserver(Observer<T> *observer) {}
	virtual void removeObserver(Observer<T> *observer) {}
	virtual void notifyObservers(const T &) const {}
	~Observable() {}
};

template <class T>
class BasicObservable : public Observable<T> {
	std::vector<Observer<T> *> observers;

   public:
	void addObserver(Observer<T> *observer) override { observers.push_back(observer); }
	void removeObserver(Observer<T> *observer) override {
		observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
	}
	void notifyObservers(const T &v) const override {
		for (auto observer : observers) {
			observer->update(v);
		}
	}
};

template <class T>
class ForwardObservable : public Observable<T> {
	Observable<T> *observable;

   public:
	ForwardObservable(Observable<T> *observable) : observable(observable) {}
	void addObserver(Observer<T> *observer) override { observable->addObserver(observer); }
	void removeObserver(Observer<T> *observer) override { observable->removeObserver(observer); }
	void notifyObservers(const T &v) const override { observable->notifyObservers(v); }
};
