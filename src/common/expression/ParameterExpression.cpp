/* Copyright (c) 2021 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "common/expression/ParameterExpression.h"
#include "common/expression/ExprVisitor.h"

namespace nebula {

const Value& ParameterExpression::eval(ExpressionContext&) {
    // TODO : get param from session context  (czp)
    return result_;
}

std::string ParameterExpression::toString() const {
    return '$' + name_;
}

bool ParameterExpression::operator==(const Expression& rhs) const {
    return kind_ == rhs.kind() && name_ == rhs.toString();
}

void ParameterExpression::writeTo(Encoder& encoder) const {
    // kind_
    encoder << kind_;

    // name_
    encoder << name_;
}

void ParameterExpression::resetFrom(Decoder& decoder) {
    // Read name_
    name_ = decoder.readStr();
}

void ParameterExpression::accept(ExprVisitor* visitor) {
    visitor->visit(this);
}

}   // namespace nebula
