# Copyright (c) 2021 vesoft inc. All rights reserved.
#
# This source code is licensed under Apache 2.0 License,
# attached with Common Clause Condition 1.0, found in the LICENSES directory.
@czp
Feature: Parameter

  Background:
    Given a graph with space named "nba"
    Given parameters: {"p1":1,"p2":true,"p3":"Tim Duncan","p4":3.3}

  Scenario: return parameters
    When executing query:
      """
      RETURN abs($p1)+1 AS ival, $p2 and false AS bval, $p3+"ef" AS sval, round($p4)+1.1 AS fval
      """
    Then the result should be, in any order:
      | ival | bval  | sval           | fval |
      | 2    | false | "Tim Duncanef" | 4.1  |

  Scenario: match with parameters
    When executing query:
      """
      MATCH (v:player)-[:like]->(n) where id(v)==$p3 and n.age>$p1+29
      RETURN n.name AS dst
      """
    Then the result should be, in any order:
      | dst             |
      | "Manu Ginobili" |
      | "Tony Parker"   |
