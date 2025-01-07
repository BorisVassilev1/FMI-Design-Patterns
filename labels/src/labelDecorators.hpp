#pragma once

#include <memory>
#include "label.hpp"
#include "labelTransformations.hpp"

class LabelDecoratorBase : public Label {
	const Label &l;

   public:
	LabelDecoratorBase(const Label &label) : l(label) {}
	std::string getText() const override { return l.getText(); }
};

class LabelTransformDecorator : public LabelDecoratorBase {
	std::unique_ptr<LabelTransformation> t;

   public:
	LabelTransformDecorator(const Label &l, LabelTransformation *t) : LabelDecoratorBase(l), t(t) {}

	std::string getText() const override { return t->apply(LabelDecoratorBase::getText()); }
};

class LabelRandomTransformationDecorator : public LabelDecoratorBase {
	std::vector<std::unique_ptr<LabelTransformation>> ts;

   public:
	LabelRandomTransformationDecorator(const Label &l, const std::vector<LabelTransformation *> &v)
		: LabelDecoratorBase(l), ts() {
		for(auto lt : v) ts.emplace_back(lt);
	}

	std::string getText() const override { return ts[rand() % ts.size()]->apply(LabelDecoratorBase::getText()); }
};

class LabelCyclingTransformationsDecorator : public LabelDecoratorBase {
	std::vector<std::unique_ptr<LabelTransformation>> ts;
	mutable std::size_t				   i = 0;

   public:
	LabelCyclingTransformationsDecorator(const Label &l, const std::vector<LabelTransformation *> &v)
		: LabelDecoratorBase(l), ts(), i(ts.size() - 1) {
		for(auto lt : v) ts.emplace_back(lt);
	}

	std::string getText() const override { return ts[++i %= ts.size()]->apply(LabelDecoratorBase::getText()); }
};
