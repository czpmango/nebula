/* Copyright (c) 2021 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License.
 */

#include <gtest/gtest.h>

#include <memory>

#include "common/expression/AggregateExpression.h"
#include "common/expression/ArithmeticExpression.h"
#include "common/expression/ConstantExpression.h"
#include "common/expression/FunctionCallExpression.h"
#include "common/expression/LabelAttributeExpression.h"
#include "common/expression/LabelExpression.h"
#include "graph/visitor/ExtractGroupSuiteVisitor.h"
#include "graph/visitor/test/VisitorTestBase.h"

namespace nebula {
namespace graph {

class ExtractGroupSuiteVisitorTest : public VisitorTestBase {
 public:
  void checkGroupSuite(Expression* expr,
                       std::vector<Expression*> expectGroupKeys,
                       std::vector<Expression*> expectGroupItems) {
    std::string exprInfo = "The original expr is: " + expr->toString();
    ExtractGroupSuiteVisitor visitor;
    expr->accept(&visitor);
    auto groupKeys = visitor.groupKeys();
    auto groupItems = visitor.groupItems();
    EXPECT_EQ(expectGroupKeys.size(), groupKeys.size()) << "Unexpected group key size.";

    for (auto* expectGroupKey : expectGroupKeys) {
      auto keyIter = std::find_if(
          groupKeys.cbegin(), groupKeys.cend(), [expectGroupKey](const Expression* groupKey) {
            return groupKey->toString() == expectGroupKey->toString();
          });
      if (groupKeys.cend() == keyIter) {
        ASSERT_TRUE(false) << "Expected group key: " << expectGroupKey->toString() << " not found.";
      }
    }
    EXPECT_EQ(expectGroupItems.size(), groupItems.size()) << "Unexpected group item size.";
    for (auto* expectGroupItem : expectGroupItems) {
      auto itemIter = std::find_if(
          groupItems.cbegin(), groupItems.cend(), [expectGroupItem](const Expression* groupItem) {
            return groupItem->toString() == expectGroupItem->toString();
          });
      if (groupItems.cend() == itemIter) {
        ASSERT_TRUE(false) << "Expected group item " << expectGroupItem->toString()
                           << " not found.";
      }
    }
  }
};

TEST_F(ExtractGroupSuiteVisitorTest, SingleAggregate) {
  auto* constExpr = ConstantExpression::make(pool_.get(), 1);
  auto* labelExpr = LabelExpression::make(pool_.get(), "v.age");
  auto* addExpr = ArithmeticExpression::makeAdd(pool_.get(), labelExpr, constExpr);
  auto* aggExpr = AggregateExpression::make(pool_.get(), "AVG", constExpr);
  // avg(1)
  checkGroupSuite(aggExpr, {}, {aggExpr});
  // avg(v.age)
  aggExpr->setArg(labelExpr);
  checkGroupSuite(aggExpr, {}, {aggExpr});
  // avg(v.age+1)
  aggExpr->setArg(addExpr);
  checkGroupSuite(aggExpr, {}, {aggExpr});
}

TEST_F(ExtractGroupSuiteVisitorTest, SingleNonAggregate) {
  auto* constExpr = ConstantExpression::make(pool_.get(), 1);
  auto* labelExpr = LabelExpression::make(pool_.get(), "v.age");
  auto* addExpr = ArithmeticExpression::makeAdd(pool_.get(), labelExpr, constExpr);
  // v.age
  checkGroupSuite(labelExpr, {labelExpr}, {labelExpr});
  // v.age+1
  checkGroupSuite(addExpr, {addExpr}, {addExpr});
}

TEST_F(ExtractGroupSuiteVisitorTest, MixedExpression) {
  auto* constExpr = ConstantExpression::make(pool_.get(), 1);
  auto* labelExpr1 = LabelExpression::make(pool_.get(), "v.age");
  auto* labelExpr2 = LabelExpression::make(pool_.get(), "v.score");
  auto* labelExpr3 = LabelExpression::make(pool_.get(), "v.name");

  ArgumentList* argList1 = ArgumentList::make(pool_.get());
  argList1->addArgument(labelExpr2);
  auto* funcExpr1 = FunctionCallExpression::make(pool_.get(), "abs", argList1);
  ArgumentList* argList2 = ArgumentList::make(pool_.get());
  argList2->addArgument(funcExpr1);
  // round(abs(v.score))
  auto* funcExpr2 = FunctionCallExpression::make(pool_.get(), "round", argList2);

  auto* aggExpr1 = AggregateExpression::make(pool_.get(), "AVG", labelExpr1);
  auto* aggExpr2 = AggregateExpression::make(pool_.get(), "SUM", labelExpr2);
  auto* aggExpr3 = AggregateExpression::make(pool_.get(), "COUNT", labelExpr3);

  // abs(v.score)+1
  auto* addExpr1 = ArithmeticExpression::makeAdd(pool_.get(), funcExpr1, constExpr);
  // sum(v.score)+v.age
  auto* addExpr2 = ArithmeticExpression::makeAdd(pool_.get(), aggExpr2, labelExpr1);
  // abs(v.score)+count(v.name)
  auto* addExpr3 = ArithmeticExpression::makeAdd(pool_.get(), funcExpr1, aggExpr3);
  // sum(v.score)+v.age+(abs(v.score)+count(v.name))
  auto* addExpr4 = ArithmeticExpression::makeAdd(pool_.get(), addExpr2, addExpr3);
  // sum(v.score)+v.age+(abs(v.score)+count(v.name))+round(abs(v.score))
  auto* addExpr5 = ArithmeticExpression::makeAdd(pool_.get(), addExpr4, funcExpr2);
  // (abs(v.score)+1) / avg(v.age)
  auto* divExpr1 = ArithmeticExpression::makeDivision(pool_.get(), addExpr1, aggExpr1);
  // (abs(v.score)+1) / avg(v.age) /
  // (sum(v.score)+v.age+(abs(v.score)+count(v.name))+round(abs(v.score)))
  auto* divExpr2 = ArithmeticExpression::makeDivision(pool_.get(), divExpr1, addExpr5);

  // keys: abs(v.score)+1, abs(v.score), round(abs(v.score)), v.age
  std::vector<Expression*> expectGroupKeys{addExpr1, funcExpr1, funcExpr2, labelExpr1};
  // items: abs(v.score)+1, abs(v.score), round(abs(v.score)), v.age, avg(v.age), sum(v.score),
  // count(v.name)
  std::vector<Expression*> expectGroupItems{
      addExpr1, funcExpr1, funcExpr2, labelExpr1, aggExpr1, aggExpr2, aggExpr3};
  checkGroupSuite(divExpr2, expectGroupKeys, expectGroupItems);
}
}  // namespace graph
}  // namespace nebula
