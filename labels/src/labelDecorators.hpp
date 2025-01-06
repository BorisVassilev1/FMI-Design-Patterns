#pragma once

#include "label.hpp"
#include "labelTransformations.hpp"

class LabelDecoratorBase : public Label {
	const Label &l;

   public:
	LabelDecoratorBase(const Label &label) : l(label) {}
};

class LabelTransformDecorator : public LabelDecoratorBase {
	const LabelTransformation &t;

   public:
	LabelTransformDecorator(const Label &l, const LabelTransformation &t) : LabelDecoratorBase(l), t(t) {}

	std::string getText() const override { return t.apply(LabelDecoratorBase::getText()); }
};

class RandomTransformationDecorator : public LabelDecoratorBase {
	std::vector<LabelTransformation *> ts;

   public:
	RandomTransformationDecorator(const Label &l, const std::vector<LabelTransformation *> &v)
		: LabelDecoratorBase(l), ts(v) {}

	std::string getText() const override { return ts[rand() % ts.size()]->apply(LabelDecoratorBase::getText()); }
};


class CyclingTransformationsDecorator : public LabelDecoratorBase {
	std::vector<LabelTransformation *> ts;
	mutable std::size_t i = 0;
public:
	CyclingTransformationsDecorator(const Label &l, const std::vector<LabelTransformation *> &&v)
		: LabelDecoratorBase(l), ts(v), i(ts.size()-1) {}

	std::string getText() const override { return ts[++i %= ts.size()]->apply(LabelDecoratorBase::getText()); }
};


