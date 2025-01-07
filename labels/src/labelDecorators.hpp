#pragma once

#include <functional>
#include <type_traits>
#include "label.hpp"
#include "labelTransformations.hpp"

class LabelDecoratorBase : public Label {
	std::reference_wrapper<Label> l;

   public:
	LabelDecoratorBase(Label &label) : l(label) {}
	std::reference_wrapper<Label> &getLabel() { return l; }
	const std::reference_wrapper<Label> &getLabel() const { return l; }
	std::string							 getText() const override { return l.get().getText(); }
};

class TransformDecorator : public LabelDecoratorBase {
	const LabelTransformation &t;

   public:
	TransformDecorator(Label &l, const LabelTransformation &t) : LabelDecoratorBase(l), t(t) {}

	std::string getText() const override { return t.apply(LabelDecoratorBase::getText()); }
};

class RandomTransformationDecorator : public LabelDecoratorBase {
	std::vector<std::reference_wrapper<LabelTransformation>> ts;

   public:
	RandomTransformationDecorator(Label &l, const std::vector<std::reference_wrapper<LabelTransformation>> &v)
		: LabelDecoratorBase(l), ts() {
		for (auto lt : v)
			ts.emplace_back(lt);
	}

	std::string getText() const override { return ts[rand() % ts.size()].get().apply(LabelDecoratorBase::getText()); }
};

class CyclingTransformationsDecorator : public LabelDecoratorBase {
	std::vector<std::reference_wrapper<LabelTransformation>> ts;
	mutable std::size_t										 i = 0;

   public:
	CyclingTransformationsDecorator(Label &l, const std::vector<std::reference_wrapper<LabelTransformation>> &v)
		: LabelDecoratorBase(l), ts(), i(ts.size() - 1) {
		for (auto lt : v)
			ts.emplace_back(lt);
	}

	std::string getText() const override { return ts[++i %= ts.size()].get().apply(LabelDecoratorBase::getText()); }
};



template <class Decorator>
	requires std::is_base_of_v<LabelDecoratorBase, Decorator>
std::reference_wrapper<Label> removeDecorator_(std::reference_wrapper<Label> &label) {
	if (LabelDecoratorBase *decorator = dynamic_cast<Decorator *>(&label.get())) {
		// found decorator we are looking for
		std::cout << 1 << std::endl;
		return decorator->getLabel();
	} else if (LabelDecoratorBase *decorator = dynamic_cast<LabelDecoratorBase *>(&label.get())) {
		// just a decorator
		std::cout << 2 << std::endl;
		decorator->getLabel() = removeDecorator_<Decorator>(decorator->getLabel());
		return label;
	} else {
		// plain label, return it
		std::cout << 3 << std::endl;
		return label;
	}
}

template <class Decorator>
	requires std::is_base_of_v<LabelDecoratorBase, Decorator>
const Label &removeDecorator(Label &label) {
	auto r = std::reference_wrapper<Label>(label);
	return ::removeDecorator_<Decorator>(r).get();
}
