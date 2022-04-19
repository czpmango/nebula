/* Copyright (c) 2022 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License.
 */
#include "graph/visitor/ExtractGroupSuiteVisitor.h"

#include "common/expression/AttributeExpression.h"
#include "graph/util/ExpressionUtils.h"

namespace nebula {
namespace graph {

void ExtractGroupSuiteVisitor::visit(TypeCastingExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  expr->operand()->accept(this);
}

void ExtractGroupSuiteVisitor::visit(UnaryExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  expr->operand()->accept(this);
}

void ExtractGroupSuiteVisitor::visit(FunctionCallExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  for (const auto& arg : expr->args()->args()) {
    arg->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(ListExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  for (const auto& item : expr->items()) {
    item->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(SetExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  for (const auto& item : expr->items()) {
    item->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(MapExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  for (const auto& pair : expr->items()) {
    pair.second->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(CaseExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }

  if (expr->hasCondition()) {
    expr->condition()->accept(this);
  }
  if (expr->hasDefault()) {
    expr->defaultResult()->accept(this);
  }
  for (const auto& whenThen : expr->cases()) {
    whenThen.when->accept(this);
    whenThen.then->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(PredicateExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }

  expr->collection()->accept(this);
  if (expr->hasFilter()) {
    expr->filter()->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(ReduceExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }

  expr->initial()->accept(this);
  expr->collection()->accept(this);
  expr->mapping()->accept(this);
}

void ExtractGroupSuiteVisitor::visit(ListComprehensionExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }

  expr->collection()->accept(this);

  if (expr->hasFilter()) {
    expr->filter()->accept(this);
  }

  if (expr->hasMapping()) {
    expr->mapping()->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(LogicalExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  for (const auto& operand : expr->operands()) {
    operand->accept(this);
  }
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
}

void ExtractGroupSuiteVisitor::visit(PathBuildExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  for (const auto& item : expr->items()) {
    item->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(SubscriptRangeExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }

  expr->list()->accept(this);

  if (expr->lo() != nullptr) {
    expr->lo()->accept(this);
  }

  if (expr->hi() != nullptr) {
    expr->hi()->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(MatchPathPatternExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  if (expr->inputProp() != nullptr) {
    expr->inputProp()->accept(this);
  }
}

void ExtractGroupSuiteVisitor::visit(ConstantExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(EdgePropertyExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(TagPropertyExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(InputPropertyExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(VariablePropertyExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(SourcePropertyExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(DestPropertyExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(EdgeSrcIdExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(EdgeTypeExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(EdgeRankExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(EdgeDstIdExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(UUIDExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(VariableExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(VersionedVariableExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(LabelExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(EdgeExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(ColumnExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(VertexExpression* expr) {
  groupKeys_.emplace_back(expr);
  groupItems_.emplace_back(expr);
}

void ExtractGroupSuiteVisitor::visit(LabelAttributeExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  expr->left()->accept(this);
  expr->right()->accept(this);
}

void ExtractGroupSuiteVisitor::visit(LabelTagPropertyExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  expr->label()->accept(this);
}

void ExtractGroupSuiteVisitor::visit(AggregateExpression* expr) {
  groupItems_.emplace_back(expr->clone());
}

void ExtractGroupSuiteVisitor::visit(AttributeExpression* expr) {
  visitBinaryExpr(expr);
}

void ExtractGroupSuiteVisitor::visit(SubscriptExpression* expr) {
  visitBinaryExpr(expr);
}

void ExtractGroupSuiteVisitor::visit(RelationalExpression* expr) {
  visitBinaryExpr(expr);
}

void ExtractGroupSuiteVisitor::visit(ArithmeticExpression* expr) {
  visitBinaryExpr(expr);
}

void ExtractGroupSuiteVisitor::visitBinaryExpr(BinaryExpression* expr) {
  if (!ExpressionUtils::hasAny(expr, {Expression::Kind::kAggregate})) {
    groupKeys_.emplace_back(expr);
    groupItems_.emplace_back(expr);
    return;
  }
  expr->left()->accept(this);
  expr->right()->accept(this);
}

}  // namespace graph
}  // namespace nebula
