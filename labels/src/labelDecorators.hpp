#pragma once

#include <cassert>
#include <functional>
#include <type_traits>
#include "autoref.hpp"
#include "label.hpp"
#include "labelTransformations.hpp"
#include "utils.hpp"

class LabelDecoratorBase : public Label {
	SmartRef<Label> l;

   public:
	template <class Q>
	LabelDecoratorBase(Q &&label) : l(label) {}

	SmartRef<Label> &getLabel() { return l; }

	virtual std::string getText() const override { return l->getText(); }
	bool				operator==(const Label &) const noexcept override { return true; }
};

class TransformDecorator : public LabelDecoratorBase {
	SmartRef<LabelTransformation> t;

   public:
	template <class Q, class R>
	TransformDecorator(Q &&l, R &&t) : LabelDecoratorBase(l), t(t) {}

	std::string getText() const override { return t->apply(LabelDecoratorBase::getText()); }

	bool operator==(const Label &rhs) const noexcept override {
		if (const TransformDecorator *d = static_cast<const TransformDecorator *>(&rhs)) { return t == d->t; }
		return false;
	}
};

class RandomTransformationDecorator : public LabelDecoratorBase {
	std::vector<SmartRef<LabelTransformation>> ts;

   public:
	template <class Q>
	RandomTransformationDecorator(Q &&l, const std::vector<std::reference_wrapper<LabelTransformation>> &v)
		: LabelDecoratorBase(l), ts() {
		for (auto lt : v)
			ts.emplace_back(lt);
	}

	std::string getText() const override {
		if (ts.empty()) return LabelDecoratorBase::getText();
		return ts[rand() % ts.size()]->apply(LabelDecoratorBase::getText());
	}

	bool operator==(const Label &rhs) const noexcept override {
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

	bool operator==(const Label &rhs) const noexcept override {
		if (const CyclingTransformationsDecorator *d = static_cast<const CyclingTransformationsDecorator *>(&rhs)) {
			return ts == d->ts;
		}
		return false;
	}
};

template <class Decorator>
static void removeDecorator_(SmartRef<Label> &label) {
	if (LabelDecoratorBase *decorator = dynamic_cast<Decorator *>(&*label)) {
		// found decorator we are looking for
		SmartRef<Label> inside = std::move(decorator->getLabel());
		label				   = std::move(inside);
	} else if (LabelDecoratorBase *decorator = dynamic_cast<LabelDecoratorBase *>(&*label)) {
		// just a decorator
		removeDecorator_<Decorator>(decorator->getLabel());
	}
}

template <class Decorator>
	requires std::is_base_of_v<LabelDecoratorBase, Decorator>
Label *removeDecorator(SmartRef<Label> &&label) {
	SmartRef<Label> r = std::move(label);
	removeDecorator_<Decorator>(r);
	r.isRef = true;
	return &*r;
}

template <class Decorator>
static void removeDecorator_(SmartRef<Label> &label, Decorator *decorator) {
	if (label->operator==(*decorator)) {
		// found decorator we are looking for
		Decorator	   *subject = dynamic_cast<Decorator *>(&*label);
		SmartRef<Label> inside	= std::move(subject->getLabel());
		label					= std::move(inside);
	} else if (LabelDecoratorBase *subject = dynamic_cast<LabelDecoratorBase *>(&*label)) {
		// just a decorator
		removeDecorator_<Decorator>(subject->getLabel(), decorator);
	}
}

template <class Decorator>
Label *removeDecorator(SmartRef<Label> &&label, Decorator *decorator) {
	SmartRef<Label> r = std::move(label);
	removeDecorator_(r, decorator);
	r.isRef = true;
	return &*r;
}
