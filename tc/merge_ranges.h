
// think-cell public library
//
// Copyright (C) 2016-2018 think-cell Software GmbH
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include "algorithm.h"
#include "as_lvalue.h"
#include "zip_range.h"
#include "filter_adaptor.h"

namespace tc {
	namespace no_adl {
		struct MakeReferenceOrValue final {
			template<typename T>
			auto operator()(T&& t) const {
				return tc::reference_or_value<T>(aggregate_tag(), std::forward<T>(t));
			}
		};
	}

	namespace merge_many_detail {
		template<typename Rng, std::enable_if_t<!std::is_reference< tc::range_reference_t<Rng> >::value>* =nullptr>
		auto make_cached(Rng const& rng) {
			return tc::make_vector(
				tc::transform(
					tc::transform(
						rng,
						tc::no_adl::MakeReferenceOrValue()
					),
					tc::fn_indirection()
				)
			);
		}

		template<typename Rng, std::enable_if_t<std::is_reference< tc::range_reference_t<Rng> >::value>* =nullptr>
		auto make_cached(Rng&& rng) -> Rng&& {
			return std::forward<Rng>(rng);
		}
	}

	namespace no_adl {
		template<typename RngRng, typename Pred>
		struct merge_many_adaptor {
		private:
			tc::reference_or_value<RngRng> m_baserng;
			Pred m_pred;

		public:
			template<typename Rhs, typename PredRhs>
			explicit merge_many_adaptor(aggregate_tag, Rhs&& rhs, PredRhs&& pred) noexcept
				: m_baserng( aggregate_tag(), std::forward<Rhs>(rhs) )
				, m_pred(std::forward<PredRhs>(pred))
			{}

			template< typename Func >
			auto operator()(Func func) const& MAYTHROW -> tc::common_type_t<decltype(tc::continue_if_not_break(func, tc_front(tc_front(*m_baserng)))), INTEGRAL_CONSTANT(tc::continue_)> {
				auto vecrngrng = tc::make_vector(
					tc::transform(
						*m_baserng,
						tc::fn_make_view()
					)
				);

				for (;;) {
					auto it = tc::best_element<tc::return_element_or_null>(
						tc::as_lvalue(tc::filter( // as_lvalue -> yield non-const iterator
							vecrngrng,
							tc::not_fn(tc::fn_empty())
						)),
						projected_front(m_pred)
					).element_base();

					if (!it) break;

					RETURN_IF_BREAK(tc::continue_if_not_break(func, tc_front(*it)));
					tc::drop_first_inplace(*it);
				}
				return INTEGRAL_CONSTANT(tc::continue_)();
			}

		};
	}

	namespace merge_many_detail {
		template<typename RngRng, typename Pred>
		auto merge_many_impl(RngRng&& rngrng, Pred&& pred) noexcept return_ctor(
			tc::no_adl::merge_many_adaptor<RngRng BOOST_PP_COMMA() Pred>,
			(aggregate_tag(), std::forward<RngRng>(rngrng) BOOST_PP_COMMA() std::forward<Pred>(pred))
		)
	}

	namespace no_adl {
		template< typename RngRng, bool bConst >
		struct range_reference_merge_many_adaptor {
			using type = tc::range_reference_t<
				reference_for_value_or_reference_with_index_range_t<RngRng, bConst>
			>;
		};

		template<typename RngRng, typename Pred>
		struct range_reference<tc::no_adl::merge_many_adaptor<RngRng, Pred>> : range_reference_merge_many_adaptor<RngRng, false> {};

		template<typename RngRng, typename Pred>
		struct range_reference<tc::no_adl::merge_many_adaptor<RngRng, Pred> const> : range_reference_merge_many_adaptor<RngRng, true> {};
	}


	template<typename RngRng, typename Pred>
	auto merge_many(RngRng&& rngrng, Pred&& pred) {
		return merge_many_detail::merge_many_impl(
			merge_many_detail::make_cached(std::forward<RngRng>(rngrng)),
			std::forward<Pred>(pred)
		);
	}

	template<typename RngRng>
	auto merge_many(RngRng&& rngrng) {
		return merge_many(std::forward<RngRng>(rngrng), tc::fn_less());
	}

}
