/* Copyright (c) 2021 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License.
 */

#ifndef GRAPH_VISITOR_EXTRACTGROUPSUITEVISITOR_H_
#define GRAPH_VISITOR_EXTRACTGROUPSUITEVISITOR_H_

#include <vector>

#include "common/expression/ExprVisitor.h"

namespace nebula {
namespace graph {
class ExtractGroupSuiteVisitor final : public ExprVisitor {
 public:
  ExtractGroupSuiteVisitor() = default;

  std::vector<Expression*> groupKeys() const {
    return groupKeys_;
  }

  std::vector<Expression*> groupItems() const {
    return groupItems_;
  }

 private:
  void visit(ConstantExpression*) override;
  void visit(EdgePropertyExpression*) override;
  void visit(TagPropertyExpression*) override;
  void visit(LabelTagPropertyExpression*) override;
  void visit(InputPropertyExpression*) override;
  void visit(VariablePropertyExpression*) override;
  void visit(SourcePropertyExpression*) override;
  void visit(DestPropertyExpression*) override;
  void visit(EdgeSrcIdExpression*) override;
  void visit(EdgeTypeExpression*) override;
  void visit(EdgeRankExpression*) override;
  void visit(EdgeDstIdExpression*) override;
  void visit(UUIDExpression*) override;
  void visit(VariableExpression*) override;
  void visit(VersionedVariableExpression*) override;
  void visit(LabelExpression*) override;
  void visit(AttributeExpression*) override;
  void visit(LabelAttributeExpression*) override;
  void visit(VertexExpression*) override;
  void visit(EdgeExpression*) override;

  void visit(TypeCastingExpression*) override;
  void visit(UnaryExpression*) override;
  void visit(AggregateExpression*) override;

  void visit(SubscriptExpression*) override;
  void visit(SubscriptRangeExpression*) override;
  void visit(LogicalExpression*) override;
  void visit(RelationalExpression*) override;
  void visit(ArithmeticExpression*) override;

  void visit(FunctionCallExpression*) override;
  void visit(ListExpression*) override;
  void visit(SetExpression*) override;
  void visit(MapExpression*) override;
  void visit(CaseExpression*) override;
  void visit(PredicateExpression*) override;
  void visit(ReduceExpression*) override;
  void visit(PathBuildExpression*) override;
  void visit(ColumnExpression*) override;
  void visit(ListComprehensionExpression*) override;
  void visit(MatchPathPatternExpression*) override;

  void visitBinaryExpr(BinaryExpression* expr);

 private:
  std::vector<Expression*> groupKeys_;
  std::vector<Expression*> groupItems_;
};

}  // namespace graph
}  // namespace nebula

#endif  // GRAPH_VISITOR_EXTRACTGROUPSUITEVISITOR_H_
