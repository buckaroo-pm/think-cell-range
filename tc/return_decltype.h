
// think-cell public library
//
// Copyright (C) 2016-2018 think-cell Software GmbH
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once
#include "type_traits.h"

namespace tc {
	template< typename Var, typename Expr>
	struct return_decltype_retval final {
		static_assert( std::is_same< Var, Expr >::value, "choose between return_variable_by_ref and return_by_val" );
		static_assert( !std::is_rvalue_reference< Expr >::value, "choose between return_decltype_rvalue_by_ref and return_decltype_rvalue_by_val");
		using type=Expr;
	};
	template< typename Var, typename Expr>
	using return_decltype_retval_t = typename tc::return_decltype_retval<Var, Expr>::type;

#define return_decltype(...) -> tc::return_decltype_retval_t< decltype(__VA_ARGS__),decltype((__VA_ARGS__)) > { \
	return (__VA_ARGS__); \
}

#define code_return_decltype(code,...) -> tc::return_decltype_retval_t< decltype(__VA_ARGS__),decltype((__VA_ARGS__)) > { \
	code \
	return (__VA_ARGS__); \
}

#define return_by_val(...) -> tc::decay_t<decltype(__VA_ARGS__)> { \
	return (__VA_ARGS__); \
}

	template< typename Var, typename Expr >
	struct return_variable_by_ref_retval final {
		static_assert( std::is_lvalue_reference< Expr >::value, "use return_variable_by_ref only for variables");
		using type=Expr; // rvalue references behave like lvalues
		/* DOES NOT COMPILE
			void foo(int&& n) & noexcept {}
 
			int m;
			int&& n=std::move(m);
			foo(n);
		*/
	};
	template< typename Var, typename Expr >
	using return_variable_by_ref_retval_t = typename tc::return_variable_by_ref_retval< Var, Expr >::type;

#define return_variable_by_ref(...) -> tc::return_variable_by_ref_retval_t< decltype(__VA_ARGS__),decltype((__VA_ARGS__)) > { \
	return (__VA_ARGS__); \
}

	template< typename Var, typename Expr >
	struct return_decltype_rvalue_by_ref_retval final {
		static_assert( std::is_same< Var, Expr >::value, "choose between return_variable_by_ref and return_by_val" );
		using type=Expr;
	};
	template< typename Var, typename Expr >
	using return_decltype_rvalue_by_ref_retval_t = typename tc::return_decltype_rvalue_by_ref_retval< Var, Expr >::type;

#define return_decltype_rvalue_by_ref(...) -> tc::return_decltype_rvalue_by_ref_retval_t< decltype(__VA_ARGS__),decltype((__VA_ARGS__)) > { \
	return (__VA_ARGS__); \
}

	template< typename Expr >
	using lvalue_or_decay_t =
		std::conditional_t<
			std::is_lvalue_reference<Expr>::value,
			Expr,
			tc::decay_t<Expr>
		>;

	template< typename Var, typename Expr >
	struct return_decltype_rvalue_by_val_retval final {
		static_assert( std::is_same< Var, Expr >::value, "choose between return_variable_by_ref and return_by_val" );
		using type=Expr;
	};
	template< typename Var, typename Expr >
	using return_decltype_rvalue_by_val_retval_t = typename tc::return_decltype_rvalue_by_val_retval< Var, Expr >::type;

#define return_decltype_rvalue_by_val(...) -> tc::return_decltype_rvalue_by_val_retval_t< decltype(__VA_ARGS__),decltype((__VA_ARGS__)) > { \
	return boost::implicit_cast<tc::lvalue_or_decay_t< decltype((__VA_ARGS__)) >>((__VA_ARGS__)); \
}

// version commented out below doesn't work for both MSVC (compiler bug https://connect.microsoft.com/VisualStudio/feedback/details/1657760)
// as well as XCode 8 (leads to segfault in the compiler)
//#define return_decltype_rvalue_by_val_variable_by_ref(...) -> tc::lvalue_or_decay_t< decltype((__VA_ARGS__)) > { \
//	return (__VA_ARGS__); \
//}

#define return_decltype_rvalue_by_val_variable_by_ref(...) -> decltype(auto) {\
	return boost::implicit_cast<tc::lvalue_or_decay_t< decltype((__VA_ARGS__)) >>((__VA_ARGS__)); \
}
}

namespace decltype_return_test {
	struct A{
		int a;
		void access_a() & noexcept {
			static_assert( std::is_same<decltype(a),int>::value );
			static_assert( std::is_same<decltype((a)),int&>::value );
		}
		int& b;
		void access_b() & noexcept {
			static_assert( std::is_same<decltype(b),int&>::value );
			static_assert( std::is_same<decltype((b)),int&>::value );
		}
		int&& c;
		void access_c() & noexcept {
			static_assert( std::is_same<decltype(c),int&&>::value );
			static_assert( std::is_same<decltype((b)),int&>::value );
		}
	};
}

#define return_ctor(T,...) ->T { return T __VA_ARGS__ ; }
#define code_return_ctor(code, T, ...) ->T { \
	code \
	return T __VA_ARGS__ ; \
}
