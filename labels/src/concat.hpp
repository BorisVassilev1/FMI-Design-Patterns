#pragma once

#include <algorithm>
#include <functional>
#include <utility>
#include "autoref.hpp"
#include "flaggedptr.hpp"

template <class T, class U>
class Concat {
	AutoRef<T> rng1;
	AutoRef<U> rng2;

   public:
	template <class A, class B>
	Concat(A &&rng1, B &&rng2) : rng1(std::forward<A>(rng1)), rng2(std::forward<B>(rng2)) {}

	std::size_t size() const { return rng1->size() + rng2->size(); };

	auto begin() { return Iterator<decltype(rng1->begin()), decltype(rng2->begin())>(rng1->begin(), this, 0); }
	auto end() { return Iterator<decltype(rng1->begin()), decltype(rng2->begin())>(rng2->end(), this, 1); }

	auto begin() const { return ConstIterator<decltype(rng1->begin()), decltype(rng2->begin())>(rng1->begin(), this, 0); }
	auto end() const { return ConstIterator<decltype(rng1->begin()), decltype(rng2->begin())>(rng2->end(), this, 1); }

	template <class It1, class It2>
	class Iterator {
		union {
			It1 it1;
			It2 it2;
		};
		FlaggedPtr<Concat<T, U>> container;

		Iterator(It1 &&it1, Concat<T, U> *container, int flag = 0) 
		: it1(std::move(it1)), container(container) {
			this->container.setFlags(flag);
		}
		Iterator(It2 &&it2, Concat<T, U> *container, int flag = 1)
			requires(!std::is_same_v<It1, It2>)
			: it2(std::move(it2)), container(container) {
			this->container.setFlags(1);
		}

		void increment() {
			if (container.getFlags()) {
				++it2;
			} else {
				++it1;
				if (it1 == container->rng1->end()) {
					it2 = container->rng2->begin();
					container.setFlags(1);
				}
			}
		}

	   public:
		inline Iterator &operator++() {
			increment();
			return *this;
		}
		inline Iterator operator++(int) {
			Iterator res(*this);
			increment();
			return res;
		}

		inline auto operator*() { return container.getFlags() ? *it2 : *it1; }
		inline bool operator!=(Iterator other) const {
			if (container != other.container || container.getFlags() != other.container.getFlags()) {
				return true;
			} else {
				if (container.getFlags()) return it2 != other.it2;
				else return it1 != other.it1;
			}
		}
		inline bool operator==(Iterator other) const {
			if (container == other.container && container.getFlags() == other.container.getFlags()) {
				if (container.getFlags()) return it2 == other.it2;
				else return it1 == other.it1;
			} else return false;
		}
		friend class Concat<T, U>;
	};

	template <class It1, class It2>
	class ConstIterator : public Iterator<It1, It2>{
		
		inline auto operator*() = delete;
		inline const auto operator*() const { return this->container.getFlags() ? *this->it2 : *this->it1; }
	};
};

template <class A, class B>
Concat(A &&, B &&) -> Concat<A, B>;
