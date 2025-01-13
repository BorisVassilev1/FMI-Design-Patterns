#pragma once

#include <cassert>
#include <type_traits>
#include "autoref.hpp"
#include "label.hpp"
#include "labelTransformations.hpp"

class LabelDecoratorBase : public LabelImp {
	SmartRef<LabelImp> l;

   public:
	template <class Q>
	LabelDecoratorBase(Q &&label) : l(label) {}

	SmartRef<LabelImp> &getLabel() { return l; }

	virtual std::string getText() const override { return l->getText(); }
	bool				operator==(const LabelImp &) const noexcept override { return true; }
};

class TransformDecorator : public LabelDecoratorBase {
	SmartRef<LabelTransformation> t;

   public:
	template <class Q, class R>
	TransformDecorator(Q &&l, R &t) : LabelDecoratorBase(l), t(t) {}
	template <class Q, class R>
	TransformDecorator(Q &&l, R &&t) : LabelDecoratorBase(l), t(std::move(t)) {}

	std::string getText() const override { return t->apply(LabelDecoratorBase::getText()); }

	bool operator==(const LabelImp &rhs) const noexcept override {
		if (const TransformDecorator *d = static_cast<const TransformDecorator *>(&rhs)) { return t == d->t; }
		return false;
	}
};

class RandomTransformationDecorator : public LabelDecoratorBase {
	std::vector<SmartRef<LabelTransformation>> ts;

   public:
	template <class Q>
	RandomTransformationDecorator(Q &&l, const std::vector<SmartRef<LabelTransformation>> &v)
		: LabelDecoratorBase(l), ts() {
		for (auto lt : v)
			ts.emplace_back(lt);
	}

	std::string getText() const override {
		if (ts.empty()) return LabelDecoratorBase::getText();
		return ts[rand() % ts.size()]->apply(LabelDecoratorBase::getText());
	}

	bool operator==(const LabelImp &rhs) const noexcept override {
		if (const RandomTransformationDecorator *d = static_cast<const RandomTransformationDecorator *>(&rhs)) {
			return ts == d->ts;
		}
		return false;
	}
};

class CyclingTransformationsDecorator : public LabelDecoratorBase {
	std::vector<SmartRef<LabelTransformation>> ts;
	mutable std::size_t						   i = 0;

   public:
	template <class Q>
	CyclingTransformationsDecorator(Q &&l, const std::vector<SmartRef<LabelTransformation>> &v)
		: LabelDecoratorBase(l), ts(), i(ts.size() - 1) {
		for (auto lt : v)
			ts.emplace_back(lt);
	}

	std::string getText() const override {
		if (ts.empty()) return LabelDecoratorBase::getText();
		return ts[++i %= ts.size()]->apply(LabelDecoratorBase::getText());
	}

	bool operator==(const LabelImp &rhs) const noexcept override {
		if (const CyclingTransformationsDecorator *d = static_cast<const CyclingTransformationsDecorator *>(&rhs)) {
			return ts == d->ts;
		}
		return false;
	}
};

template <class Decorator>
static void removeDecorator_(SmartRef<LabelImp> &label) {
	if (LabelDecoratorBase *decorator = dynamic_cast<Decorator *>(&*label)) {
		// found decorator we are looking for
		SmartRef<LabelImp> inside = std::move(decorator->getLabel());
		label					  = std::move(inside);
	} else if (LabelDecoratorBase *decorator = dynamic_cast<LabelDecoratorBase *>(&*label)) {
		// just a decorator
		removeDecorator_<Decorator>(decorator->getLabel());
	}
}

template <class Decorator>
	requires std::is_base_of_v<LabelDecoratorBase, Decorator>
LabelImp *removeDecorator(SmartRef<LabelImp> &&label) {
	SmartRef<LabelImp> r = std::move(label);
	removeDecorator_<Decorator>(r);
	r.isRef = true;
	return &*r;
}

template <class Decorator>
static void removeDecorator_(SmartRef<LabelImp> &label, Decorator *decorator) {
	if (label->operator==(*decorator)) {
		// found decorator we are looking for
		Decorator		  *subject = dynamic_cast<Decorator *>(&*label);
		SmartRef<LabelImp> inside  = std::move(subject->getLabel());
		label					   = std::move(inside);
	} else if (LabelDecoratorBase *subject = dynamic_cast<LabelDecoratorBase *>(&*label)) {
		// just a decorator
		removeDecorator_<Decorator>(subject->getLabel(), decorator);
	}
}

template <class Decorator>
LabelImp *removeDecorator(SmartRef<LabelImp> &&label, Decorator *decorator) {
	SmartRef<LabelImp> r = std::move(label);
	removeDecorator_(r, decorator);
	r.isRef = true;
	return &*r;
}
