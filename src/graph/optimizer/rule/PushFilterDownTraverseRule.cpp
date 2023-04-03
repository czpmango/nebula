/* Copyright (c) 2022 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License.
 */

#include "graph/optimizer/rule/PushFilterDownTraverseRule.h"

#include "common/expression/AttributeExpression.h"
#include "common/expression/ConstantExpression.h"
#include "common/expression/Expression.h"
#include "common/expression/PredicateExpression.h"
#include "common/expression/PropertyExpression.h"
#include "common/expression/VariableExpression.h"
#include "graph/optimizer/OptContext.h"
#include "graph/optimizer/OptGroup.h"
#include "graph/planner/plan/PlanNode.h"
#include "graph/planner/plan/Query.h"
#include "graph/util/ExpressionUtils.h"
#include "graph/visitor/ExtractFilterExprVisitor.h"
#include "graph/visitor/RewriteVisitor.h"

using nebula::Expression;
using nebula::graph::Filter;
using nebula::graph::PlanNode;
using nebula::graph::QueryContext;
using nebula::graph::Traverse;

namespace nebula {
namespace opt {

std::unique_ptr<OptRule> PushFilterDownTraverseRule::kInstance =
    std::unique_ptr<PushFilterDownTraverseRule>(new PushFilterDownTraverseRule());

PushFilterDownTraverseRule::PushFilterDownTraverseRule() {
  RuleSet::QueryRules().addRule(this);
}

const Pattern& PushFilterDownTraverseRule::pattern() const {
  static Pattern pattern =
      Pattern::create(PlanNode::Kind::kFilter, {Pattern::create(PlanNode::Kind::kTraverse)});
  return pattern;
}

bool PushFilterDownTraverseRule::match(OptContext* ctx, const MatchedResult& matched) const {
  return OptRule::match(ctx, matched);
}

// Pick the `all` predicate for edges can be scattered as a single-hop edge predicate
bool isEdgeAllPredicate(const Expression* e, const std::string& edgeAlias) {
  if (e->kind() != Expression::Kind::kPredicate) {
    return false;
  }
  auto* pe = static_cast<const PredicateExpression*>(e);
  if (pe->name() != "all" || !pe->hasInnerVar()) {
    return false;
  }
  auto var = pe->innerVar();
  if (pe->collection()->kind() != Expression::Kind::kInputProperty) {
    return false;
  }
  // Check edge collection expression
  if (static_cast<const PropertyExpression*>(pe->collection())->prop() != edgeAlias) {
    return false;
  }
  auto ves = graph::ExpressionUtils::collectAll(pe->filter(), {Expression::Kind::kAttribute});
  for (const auto& ve : ves) {
    auto iv = static_cast<const AttributeExpression*>(ve)->left();
    if (iv->kind() != Expression::Kind::kVar) {
      return false;
    }
    // Check inner vars
    if (!static_cast<const VariableExpression*>(iv)->isInner()) {
      // Only care inner edge vars
      continue;
    }
    // Edge property must be ConstantExpression
    auto ep = static_cast<const AttributeExpression*>(ve)->right();
    if (ep->kind() != Expression::Kind::kConstant) {
      return false;
    }
    // Edge property name should be string
    if (!static_cast<const ConstantExpression*>(ep)->value().isStr()) {
      return false;
    }
  }

  return true;
}

// where true==all(i in e where i.prop1>3 and i.prop2<=5) and all(i in e where i.prop3>30 and
// i.prop4<=50) and <unpickedPredicateExpr> like.prop1>3 and like.prop2<=5 and like.prop3>30 and
// like.prop4<=50

// Pick sub-predicate

// Rewrite edge all predicate to scattered single-hop edge predicate
Expression* rewriteScatteredEdgePredicate(const Expression* edgeAllPredicate,
                                          const std::string& edgeAlias) {
  auto matcher = [&edgeAlias](const Expression* e) -> bool {
    return isEdgeAllPredicate(e, edgeAlias);
  };
  auto rewriter = [](const Expression* e) -> Expression* {
    DCHECK_EQ(e->kind(), Expression::Kind::kPredicate);
    auto fe = static_cast<const PredicateExpression*>(e)->filter();

    auto innerMatcher = [](const Expression* ae) {
      if (ae->kind() != Expression::Kind::kAttribute) {
        return false;
      }
      // All inner vars have been checked as matched edge in the external matcher and they all need
      // to be rewritten
      return static_cast<const AttributeExpression*>(ae)->left()->kind() == Expression::Kind::kVar;
    };

    auto innerRewriter = [](const Expression* ae) {
      DCHECK_EQ(ae->kind(), Expression::Kind::kAttribute);
      auto attributeExpr = static_cast<const AttributeExpression*>(ae);
      auto* right = attributeExpr->right();
      // Edge property name expressions have been checked in the external matcher
      DCHECK_EQ(right->kind(), Expression::Kind::kConstant);
      auto& prop = static_cast<const ConstantExpression*>(right)->value().getStr();
      return EdgePropertyExpression::make(ae->getObjPool(), "*", prop);
    };
    // Rewrite all the inner var edge attribute expressions of `all` predicate's filter to
    // EdgePropertyExpression
    return graph::RewriteVisitor::transform(fe, std::move(innerMatcher), std::move(innerRewriter));
  };
  return graph::RewriteVisitor::transform(
      edgeAllPredicate, std::move(matcher), std::move(rewriter));
}

StatusOr<OptRule::TransformResult> PushFilterDownTraverseRule::transform(
    OptContext* ctx, const MatchedResult& matched) const {
  auto* filterGroupNode = matched.node;
  auto* filterGroup = filterGroupNode->group();
  auto* filter = static_cast<graph::Filter*>(filterGroupNode->node());
  auto* condition = filter->condition();

  auto* tvGroupNode = matched.dependencies[0].node;
  auto* tv = static_cast<graph::Traverse*>(tvGroupNode->node());
  auto& edgeAlias = tv->edgeAlias();
  auto srcNodeAlias = tv->nodeAlias();

  auto qctx = ctx->qctx();

  auto picker = [&edgeAlias](const Expression* expr) -> bool {
    bool neverPicked = false;
    auto finder = [&neverPicked, &edgeAlias](const Expression* e) -> bool {
      if (neverPicked) {
        return false;
      }
      // UnaryNot change the semantics of `all` predicate to `any`, resulting in the inability to
      // scatter the `all` edge predicate into a single-hop edge predicate(not cover double-not
      // cases)
      if (e->kind() == Expression::Kind::kUnaryNot) {
        neverPicked = true;
        return false;
      }
      return isEdgeAllPredicate(e, edgeAlias);
    };
    graph::FindVisitor visitor(finder);
    const_cast<Expression*>(expr)->accept(&visitor);
    return !visitor.results().empty();
  };
  Expression* filterPicked = nullptr;
  Expression* filterUnpicked = nullptr;
  graph::ExpressionUtils::splitFilter(condition, picker, &filterPicked, &filterUnpicked);

  if (!filterPicked) {
    return TransformResult::noTransform();
  }

  auto* scatteredEdgeFilter = rewriteScatteredEdgePredicate(filterPicked, edgeAlias);

  return result;
}

std::string PushFilterDownTraverseRule::toString() const {
  return "PushFilterDownTraverseRule";
}

}  // namespace opt
}  // namespace nebula
