#pragma once

#include <cassert>
#include <type_traits>
#include "autoref.hpp"
#include "label.hpp"
#include "labelTransformations.hpp"

class LabelDecoratorBase : public LabelImp {
	SmartRef<LabelImp> l;

   public:
	LabelDecoratorBase(LabelImp *label) : l(label) {}
	LabelDecoratorBase(LabelImp &label) : l(&label) {}
	LabelDecoratorBase(SmartRef<LabelImp> &&label) : l(std::move(label)) {}

	SmartRef<LabelImp> &getLabel() { return l; }

	virtual std::string getText() const override { return l->getText(); }
	bool				operator==(const LabelImp &) const noexcept override { return true; }
};

class TransformDecorator : public LabelDecoratorBase {
	SmartRef<LabelTransformation> t;

   public:
	TransformDecorator(SmartRef<LabelImp> &&l, SmartRef<LabelTransformation> &&t) : LabelDecoratorBase(std::move(l)), t(std::move(t)) {}

	std::string getText() const override { return t->apply(LabelDecoratorBase::getText()); }

	bool operator==(const LabelImp &rhs) const noexcept override {
		if (const TransformDecorator *d = static_cast<const TransformDecorator *>(&rhs)) { return t == d->t; }
		return false;
	}
};

class RandomTransformationDecorator : public LabelDecoratorBase {
	std::vector<SmartRef<LabelTransformation>> ts;

   public:
	RandomTransformationDecorator(SmartRef<LabelImp> &&l, const std::vector<SmartRef<LabelTransformation>> &v)
		: LabelDecoratorBase(std::move(l)), ts() {
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
	CyclingTransformationsDecorator(SmartRef<LabelImp> &&l, const std::vector<SmartRef<LabelTransformation>> &v)
		: LabelDecoratorBase(std::move(l)), ts(), i(ts.size() - 1) {
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
	if (LabelDecoratorBase *decorator = dynamic_cast<Decorator *>(&label)) {
		// found decorator we are looking for
		SmartRef<LabelImp> inside = std::move(decorator->getLabel());
		label					  = std::move(inside);
	} else if (LabelDecoratorBase *decorator = dynamic_cast<LabelDecoratorBase *>(&label)) {
		// just a decorator
		removeDecorator_<Decorator>(decorator->getLabel());
	}
}

template <class Decorator>
	requires std::is_base_of_v<LabelDecoratorBase, Decorator>
Label &removeDecorator(Label &label) {
	removeDecorator_<Decorator>(label.getImp());
	return label;
}

static void removeDecorator_(SmartRef<LabelImp> &label, LabelDecoratorBase *decorator) {
	if (label->operator==(*decorator)) {
		// found decorator we are looking for
		LabelDecoratorBase		  *subject = dynamic_cast<LabelDecoratorBase *>(&*label);
		SmartRef<LabelImp> inside  = std::move(subject->getLabel());
		label					   = std::move(inside);
	} else if (LabelDecoratorBase *subject = dynamic_cast<LabelDecoratorBase *>(&*label)) {
		// just a decorator
		removeDecorator_(subject->getLabel(), decorator);
	}
}

inline Label &removeDecorator(Label &label, LabelDecoratorBase *decorator) {
	removeDecorator_(label.getImp(), decorator);
	return label;
}


static void removeDecoratorByType_(SmartRef<LabelImp> &label, LabelDecoratorBase *decorator) {
	auto &labelType = *label; // brought out to silence warning
	if (typeid(labelType) == typeid(*decorator)) {
		// found decorator we are looking for
		LabelDecoratorBase		  *subject = dynamic_cast<LabelDecoratorBase *>(&*label);
		SmartRef<LabelImp> inside  = std::move(subject->getLabel());
		label					   = std::move(inside);
	} else if (LabelDecoratorBase *subject = dynamic_cast<LabelDecoratorBase *>(&*label)) {
		// just a decorator
		removeDecoratorByType_(subject->getLabel(), decorator);
	}
}

inline Label &removeDecoratorByType(Label &label, LabelDecoratorBase *decorator) {
	removeDecoratorByType_(label.getImp(), decorator);
	return label;
}



