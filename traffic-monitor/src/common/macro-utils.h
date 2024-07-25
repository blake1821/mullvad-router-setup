#pragma once
#define I(x) x // identity
#define CONCAT(x, y) x##y
#define VCAT(x, y) CONCAT(x, y)
#define VCALL(fn, ...) fn(__VA_ARGS__)

#define MATCHC(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, NAME, ...) NAME

// XCAT(a, b, c, d) -> abcd
#define XCAT(...) XCATH(__VA_ARGS__)
#define _XCAT(a, b) CONCAT(a, b)
#define XCATH(...) MATCHC(__VA_ARGS__, XCAT16, XCAT15, XCAT14, XCAT13, XCAT12, XCAT11, XCAT10, XCAT9, XCAT8, XCAT7, XCAT6, XCAT5, XCAT4, XCAT3, XCAT2, XCAT1, _)(__VA_ARGS__)
#define XCAT1(a) a
#define XCAT2(a, b) a##b
#define XCAT3(a, b, c) a##b##c
#define XCAT4(a, b, c, d) a##b##c##d
#define XCAT5(a, ...) _XCAT(a, XCAT4(__VA_ARGS__))
#define XCAT6(a, b, ...) _XCAT(a##b, XCAT4(__VA_ARGS__))
#define XCAT7(a, b, c, ...) _XCAT(a##b##c, XCAT4(__VA_ARGS__))
#define XCAT8(a, b, c, d, ...) _XCAT(a##b##c##d, XCAT4(__VA_ARGS__))
#define XCAT9(a, ...) _XCAT(a, XCAT8(__VA_ARGS__))
#define XCAT10(a, b, ...) _XCAT(a##b, XCAT8(__VA_ARGS__))
#define XCAT11(a, b, c, ...) _XCAT(a##b##c, XCAT8(__VA_ARGS__))
#define XCAT12(a, b, c, d, ...) _XCAT(a##b##c##d, XCAT8(__VA_ARGS__))
#define XCAT13(a, ...) _XCAT(a, XCAT12(__VA_ARGS__))
#define XCAT14(a, b, ...) _XCAT(a##b, XCAT12(__VA_ARGS__))
#define XCAT15(a, b, c, ...) _XCAT(a##b##c, XCAT12(__VA_ARGS__))
#define XCAT16(a, b, c, d, ...) _XCAT(a##b##c##d, XCAT12(__VA_ARGS__))

// APPLY(fn, 1, 2, 3) -> fn(1) fn(2) fn(3)
#define APPLY(fn, ...) APPLYH(fn, __VA_ARGS__)
#define APPLYH(fn, ...) MATCHC(__VA_ARGS__, APPLY16, APPLY15, APPLY14, APPLY13, APPLY12, APPLY11, APPLY10, APPLY9, APPLY8, APPLY7, APPLY6, APPLY5, APPLY4, APPLY3, APPLY2, APPLY1, _)(fn, __VA_ARGS__)
#define APPLY1(fn, x) fn(x)
#define APPLY2(fn, x, ...) fn(x) APPLY1(fn, __VA_ARGS__)
#define APPLY3(fn, x, ...) fn(x) APPLY2(fn, __VA_ARGS__)
#define APPLY4(fn, x, ...) fn(x) APPLY3(fn, __VA_ARGS__)
#define APPLY5(fn, x, ...) fn(x) APPLY4(fn, __VA_ARGS__)
#define APPLY6(fn, x, ...) fn(x) APPLY5(fn, __VA_ARGS__)
#define APPLY7(fn, x, ...) fn(x) APPLY6(fn, __VA_ARGS__)
#define APPLY8(fn, x, ...) fn(x) APPLY7(fn, __VA_ARGS__)
#define APPLY9(fn, x, ...) fn(x) APPLY8(fn, __VA_ARGS__)
#define APPLY10(fn, x, ...) fn(x) APPLY9(fn, __VA_ARGS__)
#define APPLY11(fn, x, ...) fn(x) APPLY10(fn, __VA_ARGS__)
#define APPLY12(fn, x, ...) fn(x) APPLY11(fn, __VA_ARGS__)
#define APPLY13(fn, x, ...) fn(x) APPLY12(fn, __VA_ARGS__)
#define APPLY14(fn, x, ...) fn(x) APPLY13(fn, __VA_ARGS__)
#define APPLY15(fn, x, ...) fn(x) APPLY14(fn, __VA_ARGS__)
#define APPLY16(fn, x, ...) fn(x) APPLY15(fn, __VA_ARGS__)

// APPLYCOMMA(fn, 1, 2, 3) -> fn(1), fn(2), fn(3)
#define APPLYCOMMA(fn, ...) APPLYCOMMAH(fn, __VA_ARGS__)
#define APPLYCOMMAH(fn, ...) MATCHC(__VA_ARGS__, APPLYCOMMA16, APPLYCOMMA15, APPLYCOMMA14, APPLYCOMMA13, APPLYCOMMA12, APPLYCOMMA11, APPLYCOMMA10, APPLYCOMMA9, APPLYCOMMA8, APPLYCOMMA7, APPLYCOMMA6, APPLYCOMMA5, APPLYCOMMA4, APPLYCOMMA3, APPLYCOMMA2, APPLYCOMMA1, _)(fn, __VA_ARGS__)
#define APPLYCOMMA1(fn, x) fn(x)
#define APPLYCOMMA2(fn, x, ...) fn(x), APPLYCOMMA1(fn, __VA_ARGS__)
#define APPLYCOMMA3(fn, x, ...) fn(x), APPLYCOMMA2(fn, __VA_ARGS__)
#define APPLYCOMMA4(fn, x, ...) fn(x), APPLYCOMMA3(fn, __VA_ARGS__)
#define APPLYCOMMA5(fn, x, ...) fn(x), APPLYCOMMA4(fn, __VA_ARGS__)
#define APPLYCOMMA6(fn, x, ...) fn(x), APPLYCOMMA5(fn, __VA_ARGS__)
#define APPLYCOMMA7(fn, x, ...) fn(x), APPLYCOMMA6(fn, __VA_ARGS__)
#define APPLYCOMMA8(fn, x, ...) fn(x), APPLYCOMMA7(fn, __VA_ARGS__)
#define APPLYCOMMA9(fn, x, ...) fn(x), APPLYCOMMA8(fn, __VA_ARGS__)
#define APPLYCOMMA10(fn, x, ...) fn(x), APPLYCOMMA9(fn, __VA_ARGS__)
#define APPLYCOMMA11(fn, x, ...) fn(x), APPLYCOMMA10(fn, __VA_ARGS__)
#define APPLYCOMMA12(fn, x, ...) fn(x), APPLYCOMMA11(fn, __VA_ARGS__)
#define APPLYCOMMA13(fn, x, ...) fn(x), APPLYCOMMA12(fn, __VA_ARGS__)
#define APPLYCOMMA14(fn, x, ...) fn(x), APPLYCOMMA13(fn, __VA_ARGS__)
#define APPLYCOMMA15(fn, x, ...) fn(x), APPLYCOMMA14(fn, __VA_ARGS__)
#define APPLYCOMMA16(fn, x, ...) fn(x), APPLYCOMMA15(fn, __VA_ARGS__)

// PREPEND(v, 1, 2, 3) -> (v, 1), (v, 2), (v, 3)
#define PREPEND(v, ...) MATCHC(__VA_ARGS__, PREPEND16, PREPEND15, PREPEND14, PREPEND13, PREPEND12, PREPEND11, PREPEND10, PREPEND9, PREPEND8, PREPEND7, PREPEND6, PREPEND5, PREPEND4, PREPEND3, PREPEND2, PREPEND1, _)(v, __VA_ARGS__)
#define PREPEND1(v, x) (v, x)
#define PREPEND2(v, x, ...) (v, x), PREPEND1(v, __VA_ARGS__)
#define PREPEND3(v, x, ...) (v, x), PREPEND2(v, __VA_ARGS__)
#define PREPEND4(v, x, ...) (v, x), PREPEND3(v, __VA_ARGS__)
#define PREPEND5(v, x, ...) (v, x), PREPEND4(v, __VA_ARGS__)
#define PREPEND6(v, x, ...) (v, x), PREPEND5(v, __VA_ARGS__)
#define PREPEND7(v, x, ...) (v, x), PREPEND6(v, __VA_ARGS__)
#define PREPEND8(v, x, ...) (v, x), PREPEND7(v, __VA_ARGS__)
#define PREPEND9(v, x, ...) (v, x), PREPEND8(v, __VA_ARGS__)
#define PREPEND10(v, x, ...) (v, x), PREPEND9(v, __VA_ARGS__)
#define PREPEND11(v, x, ...) (v, x), PREPEND10(v, __VA_ARGS__)
#define PREPEND12(v, x, ...) (v, x), PREPEND11(v, __VA_ARGS__)
#define PREPEND13(v, x, ...) (v, x), PREPEND12(v, __VA_ARGS__)
#define PREPEND14(v, x, ...) (v, x), PREPEND13(v, __VA_ARGS__)
#define PREPEND15(v, x, ...) (v, x), PREPEND14(v, __VA_ARGS__)
#define PREPEND16(v, x, ...) (v, x), PREPEND15(v, __VA_ARGS__)

// RAPPLY is used within APPLY (to deal with no recursion in C preprocessor)
// Note: Unlike APPLY, RAPPLY unpacks the arguments. APPLY is for single argument functions;
// RAPPLY(fn, (1, 2), (3, 4), ...) -> fn(, 3), fn(2, 4), ...
#define RAPPLY(fn, ...) MATCHC(__VA_ARGS__, RAPPLY16, RAPPLY15, RAPPLY14, RAPPLY13, RAPPLY12, RAPPLY11, RAPPLY10, RAPPLY9, RAPPLY8, RAPPLY7, RAPPLY6, RAPPLY5, RAPPLY4, RAPPLY3, RAPPLY2, RAPPLY1, _)(fn, __VA_ARGS__)
#define RAPPLY1(fn, x) fn x
#define RAPPLY2(fn, x, ...) fn x RAPPLY1(fn, __VA_ARGS__)
#define RAPPLY3(fn, x, ...) fn x RAPPLY2(fn, __VA_ARGS__)
#define RAPPLY4(fn, x, ...) fn x RAPPLY3(fn, __VA_ARGS__)
#define RAPPLY5(fn, x, ...) fn x RAPPLY4(fn, __VA_ARGS__)
#define RAPPLY6(fn, x, ...) fn x RAPPLY5(fn, __VA_ARGS__)
#define RAPPLY7(fn, x, ...) fn x RAPPLY6(fn, __VA_ARGS__)
#define RAPPLY8(fn, x, ...) fn x RAPPLY7(fn, __VA_ARGS__)

// version using commas
#define RAPPLYCOMMA(fn, ...) MATCHC(__VA_ARGS__, RAPPLYCOMMA16, RAPPLYCOMMA15, RAPPLYCOMMA14, RAPPLYCOMMA13, RAPPLYCOMMA12, RAPPLYCOMMA11, RAPPLYCOMMA10, RAPPLYCOMMA9, RAPPLYCOMMA8, RAPPLYCOMMA7, RAPPLYCOMMA6, RAPPLYCOMMA5, RAPPLYCOMMA4, RAPPLYCOMMA3, RAPPLYCOMMA2, RAPPLYCOMMA1, _)(fn, __VA_ARGS__)
#define RAPPLYCOMMA1(fn, x) fn x
#define RAPPLYCOMMA2(fn, x, ...) fn x, RAPPLYCOMMA1(fn, __VA_ARGS__)
#define RAPPLYCOMMA3(fn, x, ...) fn x, RAPPLYCOMMA2(fn, __VA_ARGS__)
#define RAPPLYCOMMA4(fn, x, ...) fn x, RAPPLYCOMMA3(fn, __VA_ARGS__)
#define RAPPLYCOMMA5(fn, x, ...) fn x, RAPPLYCOMMA4(fn, __VA_ARGS__)
#define RAPPLYCOMMA6(fn, x, ...) fn x, RAPPLYCOMMA5(fn, __VA_ARGS__)
#define RAPPLYCOMMA7(fn, x, ...) fn x, RAPPLYCOMMA6(fn, __VA_ARGS__)
#define RAPPLYCOMMA8(fn, x, ...) fn x, RAPPLYCOMMA7(fn, __VA_ARGS__)

// utils
#define UNPACK(...) __VA_ARGS__
#define _FIRST(x, ...) x
#define _AFTERFIRST(x, ...) __VA_ARGS__

// PAIRAPPLY(fn, (a1, a2, ...), b1, b2, ...) -> fn(a1, b1), ..., fn(a2, b2), ...
#define _PAIRAPPLYSUB2(fn_l1, x) RAPPLY(_FIRST fn_l1, PREPEND(x, _AFTERFIRST fn_l1))
#define _PAIRAPPLYSUB(pair) _PAIRAPPLYSUB2(_FIRST pair, _AFTERFIRST pair)
#define PAIRAPPLY(fn, l2, ...) APPLY(_PAIRAPPLYSUB, PREPEND((fn, UNPACK l2), __VA_ARGS__))

// version with commas
#define _PAIRAPPLYCOMMASUB2(fn_l1, x) RAPPLYCOMMA(_FIRST fn_l1, PREPEND(x, _AFTERFIRST fn_l1))
#define _PAIRAPPLYCOMMASUB(pair) _PAIRAPPLYCOMMASUB2(_FIRST pair, _AFTERFIRST pair)
#define PAIRAPPLYCOMMA(fn, l2, ...) APPLYCOMMA(_PAIRAPPLYCOMMASUB, PREPEND((fn, UNPACK l2), __VA_ARGS__))

// Tagged Unions and match()
// - Only works in C++, for now.
// - Similar to enum/match statements from Rust.
// - You can create a highly type-safe union of arbitrary subtypes.
// - The match statement is a switch statement that automatically casts
//   the union to the correct subtype.
// - The match statement also ensures that all subtypes are handled.
//
// Usage:
//
// Declaring the union:
//      First, create a macro definition for the union:
//          #define MyUnion BASED_UNION(BaseType, SubType1, SubType2, ...)
//           -- or --
//          #define MyUnion UNION(SubType1, SubType2, ...)
//    
//          The BaseType is useful when all subtypes are child classes of a base class.
//          For example, consider BASED_UNION(IPAddress, IPv4Address, IPv6Address)
//      Then, make the type visible to the compiler:
//          DECLARE_UNION(MyUnion);
//
// Instantiating or updating a union object:
//      MyUnion u = (SubType1) some_value;
//      u = (SubType2) some_other_value;
//      MyUnion w((SubType3) another_value);
//
// Using the base type:
//      u.base()->base_method();
//
// Matching on the union:
//      match(MyUnion, u, v,
//        (SubType1, {
//           v.a = 432;
//           ...
//        }),
//        (SubType2, {
//           <do stuff with SubType2 v>
//           ...
//        }),
//        ...
//      )
//
//      Note that in each match case, the variable v is automatically declared with the correct subtype.
//      And if you forget to handle a subtype, the compiler will complain.
//      This makes it about as safe as Rust's match statement.
//
//  Warnings:
//      - 

// In some macros below, we exploit the fact that recursive macro calls are not expanded by the preprocessor.
// By using match to call _UNION_T in BASED_UNION, we can prevent match() from expanding the _UNION_T macro.
// This is needed so that we can effectively use a Union type's macro name as a type,
// and then use that macro name in the match() statement.

// _IF_MATCH_THEN(key, lock, Y, N) -> Y if _key_lock is defined, N otherwise
#define _backdoorkey_backdoorlock ,
#define _IF_MATCH_SUB2(_1, _2, C, ...) C
#define _IF_MATCH_SUB(x, Y, N) _IF_MATCH_SUB2(x, Y, N)
#define _IF_MATCH_THEN(key, lock, Y, N) _IF_MATCH_SUB(_##key##_##lock, Y, N)
#define _UNION_ENUM(x) __Type_##x,
#define _UNION_UNION(x) x _##x;
#define _UNION_CONSTRUCTOR1(typename, SubType) \
    typename(const SubType &s) : _internal_as{._##SubType{s}}, _internal_type{__Type_##SubType} {}
#define _UNION_CONSTRUCTOR(typename_SubType) _UNION_CONSTRUCTOR1 typename_SubType
#define _UNION_ID(...) XCAT(__uid_, __VA_ARGS__)
#define BASED_UNION(BaseType, ...) match(backdoorkey, _UNION_T, BaseType, __VA_ARGS__)
typedef struct
{
} EmptyStruct;
#define UNION(...) BASED_UNION(EmptyStruct, __VA_ARGS__)
#define _UNION_T(BaseType, ...) XCAT(Union_, __VA_ARGS__)
#define _defmatch(_1, _2, BaseType, ...)                                       \
    struct _UNION_T(BaseType, __VA_ARGS__)                                     \
    {                                                                          \
        APPLY(                                                                 \
            _UNION_CONSTRUCTOR,                                                \
            PREPEND(_UNION_T(BaseType, __VA_ARGS__), __VA_ARGS__))             \
        enum                                                                   \
        {                                                                      \
            _UNION_ID(__VA_ARGS__),                                            \
            APPLY(_UNION_ENUM, __VA_ARGS__)                                    \
        } _internal_type;                                                      \
        union                                                                  \
        {                                                                      \
            APPLY(_UNION_UNION, __VA_ARGS__)                                   \
        } _internal_as;                                                        \
        inline BaseType &const base() { return *((BaseType *)&_internal_as); } \
    }
#define _UNION_DEF(CallExp, _2) _def##CallExp
#define DECLARE_UNION(UnionName) match(backdoorkey, _UNION_DEF, ##UnionName)
#define UNION_INIT(SubType, v)              \
    {                                       \
        ._internal_type = __Type_##SubType; \
        ._internal_as._##SubType = v;       \
    }
#define _MATCHCASESUB3(u, v, SubType, ...)           \
    case u.__Type_##SubType:                         \
    {                                                \
        SubType const v = u._internal_as._##SubType; \
        __VA_ARGS__                                  \
    }                                                \
    break;
#define _MATCHCASESUB2(exp1, exp2) _MATCHCASESUB3(exp1, exp2)
#define _MATCHCASESUB1(uv, matchcase) _MATCHCASESUB2(UNPACK uv, UNPACK matchcase)
#define _MATCHCASESUB0(exp) _MATCHCASESUB1(exp) // _MCS1(UNPACK ((u,v), matchase)) -> _MCS1((u,v), matchase)
#define _MATCHCASE(uv_matchcase) _MATCHCASESUB0(UNPACK uv_matchcase)
#define _MATCHCHECKID(u, SubTypes) sizeof(u._UNION_ID(SubTypes));
#define _MATCHMAKESTATIC2(SubType, ...) static char _##SubType##_exists;
#define _MATCHMAKESTATIC1(matchcase) _MATCHMAKESTATIC2(matchcase)
#define _MATCHMAKESTATIC(matchcase) _MATCHMAKESTATIC1(UNPACK matchcase)
#define _MATCHCHECKSTATIC(SubType) sizeof(_##SubType##_exists);
#define _MATCHCHECKSUB0(SubTypes, u, ...)         \
    {                                             \
        _MATCHCHECKID(u, UNPACK SubTypes);        \
        APPLY(_MATCHMAKESTATIC, __VA_ARGS__)      \
        APPLY(_MATCHCHECKSTATIC, UNPACK SubTypes) \
    }
#define _MATCHCHECK(CallExp, u, ...) _MATCHCHECKSUB0((CallExp), u, __VA_ARGS__)
#define _ex_match(key, _f, _base, ...) __VA_ARGS__
#define _MATCHSUB2(CallExp, u, v, ...)                  \
    switch (u._internal_type)                           \
    {                                                   \
        APPLY(_MATCHCASE, PREPEND((u, v), __VA_ARGS__)) \
    }                                                   \
    _MATCHCHECK(_ex_##CallExp, u, __VA_ARGS__);
#define _CALL_SECOND(_1, fn, ...) fn(__VA_ARGS__)
#define _MATCHSUB(key, ...) _IF_MATCH_THEN(key, backdoorlock, _CALL_SECOND, _MATCHSUB2)(key, __VA_ARGS__)
#define match(key, _2, _3, ...) _MATCHSUB(##key, ##_2, ##_3, __VA_ARGS__)

#define MyUnion UNION(int, float)
DECLARE_UNION(MyUnion);