#pragma once

#include <string_view>

namespace DB
{

class ReadBuffer;
class WriteBuffer;

/// Defines which side of the JOIN is preserved in the result.
enum class JoinKind : uint8_t
{
    Inner, /// Keep only joined rows.
    Left,  /// Keep all rows from left table. Fill with default values for right table where no matches.
    Right, /// Keep all rows from right table. Fill with default values for left table where no matches.
    Full,  /// Keep all rows from both tables. Fill with default values where no matches.
    Cross, /// Direct product. Strictness and condition doesn't matter.
    Comma, /// Same as direct product. Intended to be converted to INNER JOIN with conditions from WHERE.
    Paste, /// Stack columns from left and right tables.
};

void serializeJoinKind(JoinKind kind, WriteBuffer & out);
JoinKind deserializeJoinKind(ReadBuffer & in);

const char * toString(JoinKind kind);

constexpr bool isLeft(JoinKind kind)         { return kind == JoinKind::Left; }
constexpr bool isRight(JoinKind kind)        { return kind == JoinKind::Right; }
constexpr bool isInner(JoinKind kind)        { return kind == JoinKind::Inner; }
constexpr bool isFull(JoinKind kind)         { return kind == JoinKind::Full; }
constexpr bool isOuter(JoinKind kind)        { return kind == JoinKind::Left || kind == JoinKind::Right || kind == JoinKind::Full; }
constexpr bool isCrossOrComma(JoinKind kind) { return kind == JoinKind::Comma || kind == JoinKind::Cross; }
constexpr bool isRightOrFull(JoinKind kind)  { return kind == JoinKind::Right || kind == JoinKind::Full; }
constexpr bool isLeftOrFull(JoinKind kind)   { return kind == JoinKind::Left  || kind == JoinKind::Full; }
constexpr bool isInnerOrRight(JoinKind kind) { return kind == JoinKind::Inner || kind == JoinKind::Right; }
constexpr bool isInnerOrLeft(JoinKind kind)  { return kind == JoinKind::Inner || kind == JoinKind::Left; }
constexpr bool isPaste(JoinKind kind)        { return kind == JoinKind::Paste; }

JoinKind reverseJoinKind(JoinKind kind);

/// Allows more optimal JOIN for typical cases.
enum class JoinStrictness : uint8_t
{
    Unspecified,
    RightAny, /// Old ANY JOIN. If there are many suitable rows in right table, use any from them to join.
    Any, /// Semi Join with any value from filtering table. For LEFT JOIN with Any and RightAny are the same.
    All, /// If there are many suitable rows to join, use all of them and replicate rows of "left" table (usual semantic of JOIN).
    Asof, /// For the last JOIN column, pick the latest value
    Semi, /// LEFT or RIGHT. SEMI LEFT JOIN filters left table by values exists in right table. SEMI RIGHT - otherwise.
    Anti, /// LEFT or RIGHT. Same as SEMI JOIN but filter values that are NOT exists in other table.
};

void serializeJoinStrictness(JoinStrictness strictness, WriteBuffer & out);
JoinStrictness deserializeJoinStrictness(ReadBuffer & in);

const char * toString(JoinStrictness strictness);

/// Algorithm for distributed query processing.
enum class JoinLocality : uint8_t
{
    Unspecified,
    Local, /// Perform JOIN, using only data available on same servers (co-located data).
    Global /// Collect and merge data from remote servers, and broadcast it to each server.
};

void serializeJoinLocality(JoinLocality locality, WriteBuffer & out);
JoinLocality deserializeJoinLocality(ReadBuffer & in);

const char * toString(JoinLocality locality);

/// ASOF JOIN inequality type
enum class ASOFJoinInequality : uint8_t
{
    None,
    Less,
    Greater,
    LessOrEquals,
    GreaterOrEquals,
};

const char * toString(ASOFJoinInequality asof_join_inequality);

constexpr ASOFJoinInequality getASOFJoinInequality(std::string_view func_name)
{
    ASOFJoinInequality inequality = ASOFJoinInequality::None;

    if (func_name == "less")
        inequality = ASOFJoinInequality::Less;
    else if (func_name == "greater")
        inequality = ASOFJoinInequality::Greater;
    else if (func_name == "lessOrEquals")
        inequality = ASOFJoinInequality::LessOrEquals;
    else if (func_name == "greaterOrEquals")
        inequality = ASOFJoinInequality::GreaterOrEquals;

    return inequality;
}

constexpr ASOFJoinInequality reverseASOFJoinInequality(ASOFJoinInequality inequality)
{
    if (inequality == ASOFJoinInequality::Less)
        return ASOFJoinInequality::Greater;
    if (inequality == ASOFJoinInequality::Greater)
        return ASOFJoinInequality::Less;
    if (inequality == ASOFJoinInequality::LessOrEquals)
        return ASOFJoinInequality::GreaterOrEquals;
    if (inequality == ASOFJoinInequality::GreaterOrEquals)
        return ASOFJoinInequality::LessOrEquals;

    return ASOFJoinInequality::None;
}

enum class JoinAlgorithm : uint8_t
{
    DEFAULT = 0, /// deprecated, equivalent to "direct,hash"
    AUTO,
    HASH,
    PARTIAL_MERGE,
    PREFER_PARTIAL_MERGE,
    PARALLEL_HASH,
    GRACE_HASH,
    DIRECT,
    FULL_SORTING_MERGE,
};

const char * toString(JoinAlgorithm join_algorithm);

enum class JoinTableSide : uint8_t
{
    Left,
    Right
};

const char * toString(JoinTableSide join_table_side);

}
