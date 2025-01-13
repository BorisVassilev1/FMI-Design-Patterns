#pragma once

#include <cassert>
#include <functional>
#include <type_traits>
#include "autoref.hpp"
#include "label.hpp"
#include "labelTransformations.hpp"

class LabelDecoratorBase : public Label {
	SmartRef<Label> l;

   public:
	template <class Q>
	LabelDecoratorBase(Q &&label) : l(label) {}

	SmartRef<Label> &getLabel() { return l; }

	virtual std::string getText() const override { return l->getText(); }
};

class TransformDecorator : public LabelDecoratorBase {
	SmartRef<LabelTransformation> t;

   public:
	template <class Q, class R>
	TransformDecorator(Q &&l, R &&t) : LabelDecoratorBase(l), t(t) {}

	std::string getText() const override { return t->apply(LabelDecoratorBase::getText()); }
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
};

template <class Decorator>
	requires std::is_base_of_v<LabelDecoratorBase, Decorator>
void removeDecorator_(SmartRef<Label> &label) {
	if (LabelDecoratorBase *decorator = dynamic_cast<Decorator *>(&*label)) {
		// found decorator we are looking for
		SmartRef<Label> inside = std::move(decorator->getLabel());
		if(inside.isRef) {
			std::cout << "ok" << std::endl;
		} else {
			std::cout << "not ok" << std::endl;
		}
		label = std::move(inside);
	} else if (LabelDecoratorBase *decorator = dynamic_cast<LabelDecoratorBase *>(&*label)) {
		// just a decorator
		removeDecorator_<Decorator>(decorator->getLabel());
	}
}

template <class Decorator>
	requires std::is_base_of_v<LabelDecoratorBase, Decorator>
Label *removeDecorator(Label &label) {
	SmartRef<Label> r = label;
	assert(r.isRef);
	removeDecorator_<Decorator>(r);
	r.isRef = true;
	return &*r;
}
