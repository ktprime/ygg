#include "../src/ygg.hpp"
#include "randomizer.hpp"

#include <algorithm>
#include <boost/icl/interval_map.hpp>
#include <gtest/gtest.h>
#include <random>
#include <vector>

namespace ygg {
namespace testing {
namespace dynamic_segment_tree {

class __DST_BASENAME(Node)
    : public DynSegTreeNodeBase<int, int, int, Combiners, __DST_BASESELECTOR> {
public:
	__DST_BASENAME(Node)
	(int lower_in, int upper_in, int value_in)
	    : lower(lower_in), upper(upper_in), value(value_in){};
	__DST_BASENAME(Node)() = default;
	int lower;
	int upper;
	int value;
};

class __DST_BASENAME(NodeTraits)
    : public DynSegTreeNodeTraits<__DST_BASENAME(Node)> {
public:
	using key_type = int;
	using value_type = int;

	static key_type
	get_lower(const __DST_BASENAME(Node) & n)
	{
		return n.lower;
	}

	static key_type
	get_upper(const __DST_BASENAME(Node) & n)
	{
		return n.upper;
	}

	static value_type
	get_value(const __DST_BASENAME(Node) & n)
	{
		return n.value;
	}
};

using __DST_BASENAME(DynSegTree) =
    DynamicSegmentTree<__DST_BASENAME(Node), __DST_BASENAME(NodeTraits),
                       Combiners, DefaultOptions, __DST_BASESELECTOR>;

TEST(__DST_BASENAME(DynSegTreeTest), TrivialTest)
{
	__DST_BASENAME(Node) n(2, 5, 10);

	__DST_BASENAME(DynSegTree) agg;
	ASSERT_TRUE(agg.empty());
	agg.insert(n);
	ASSERT_FALSE(agg.empty());

	int agg_val = agg.query(3);
	ASSERT_EQ(agg_val, 10);
	int combined = agg.get_combined<MCombiner>();
	ASSERT_EQ(combined, 10);

	int combined_range = agg.get_combined<MCombiner>(3, 4);
	ASSERT_EQ(combined_range, 10);

	combined_range = agg.get_combined<MCombiner>(0, 1);
	ASSERT_EQ(combined_range, 0);
	combined_range = agg.get_combined<MCombiner>(0, 3);
	ASSERT_EQ(combined_range, 10);
	combined_range = agg.get_combined<MCombiner>(2, 5);
	ASSERT_EQ(combined_range, 10);
	combined_range = agg.get_combined<MCombiner>(2, 7);
	ASSERT_EQ(combined_range, 10);
	combined_range = agg.get_combined<MCombiner>(8, 10);
	ASSERT_EQ(combined_range, 0);

	// Test iteration
	auto it = agg.cbegin();
	ASSERT_EQ(it->get_point(), 2);
	ASSERT_EQ(it->is_start(), true);
	ASSERT_EQ(static_cast<const __DST_BASENAME(Node) *>(it->get_interval()), &n);
	it++;
	ASSERT_EQ(it->get_point(), 5);
	ASSERT_EQ(it->is_end(), true);
	ASSERT_EQ(static_cast<const __DST_BASENAME(Node) *>(it->get_interval()), &n);
	it++;
	ASSERT_EQ(it, agg.cend());
}

TEST(__DST_BASENAME(DynSegTreeTest), OneDeletionOneInsertionTest)
{
	__DST_BASENAME(Node) n(721276795, 1814953098, 10);

	__DST_BASENAME(DynSegTree) agg;
	ASSERT_TRUE(agg.empty());
	agg.insert(n);

	agg.remove(n);

	agg.insert(n);
	ASSERT_TRUE(!agg.empty());
}

TEST(__DST_BASENAME(DynSegTreeTest), TestDeletionBug)
{
	__DST_BASENAME(Node) n1(2, 5, 10);
	__DST_BASENAME(Node) n2(3, 8, 15);
	__DST_BASENAME(Node) n3(4, 10, 15);
	__DST_BASENAME(Node) n4(4, 10, 15);

	__DST_BASENAME(DynSegTree) agg;
	agg.insert(n1);
	agg.insert(n2);
	agg.insert(n3);
	agg.insert(n4);

	agg.dbg_verify_max_combiner<MCombiner>();

	agg.remove(n2);
	agg.remove(n1);
	agg.remove(n3);

	agg.dbg_verify_max_combiner<MCombiner>();
}

TEST(__DST_BASENAME(DynSegTreeTest), TestEventBounding)
{
	__DST_BASENAME(Node) n(2, 5, 10);

	__DST_BASENAME(DynSegTree) agg;
	ASSERT_TRUE(agg.empty());
	agg.insert(n);
	ASSERT_FALSE(agg.empty());

	// Test finding the [2 border via upper and lower bounding
	auto it = agg.upper_bound_event(0);
	ASSERT_EQ(it, agg.begin());
	it = agg.lower_bound_event(0);
	ASSERT_EQ(it, agg.begin());

	// Test upper-bounding  vs lower bounding
	it = agg.lower_bound_event(2);
	ASSERT_EQ(it, agg.begin());
	it = agg.upper_bound_event(2);
	ASSERT_EQ(it, agg.begin() + 1);

	// Note that the 5) border is open - thus, lower-bounding on 5 actually
	// correctly gives the end iterator!
	it = agg.lower_bound_event(5);
	ASSERT_EQ(it, agg.end());
	it = agg.upper_bound_event(5);
	ASSERT_EQ(it, agg.end());
}

TEST(__DST_BASENAME(DynSegTreeTest), NestingTest)
{
	__DST_BASENAME(Node) n[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		n[i].lower = static_cast<int>(i);
		n[i].upper = static_cast<int>(2 * DYNSEGTREE_TESTSIZE - i + 1);
		n[i].value = 1;
	}

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		agg.insert(n[i]);
	}

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		auto val = agg.query(static_cast<int>(i));
		ASSERT_EQ(val, i + 1);
	}

	int combined = agg.get_combined<MCombiner>();
	ASSERT_EQ(combined, DYNSEGTREE_TESTSIZE);

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		int combined_range =
		    agg.get_combined<MCombiner>(0, static_cast<int>(i + 1), true, false);
		ASSERT_EQ(combined_range, i + 1);
	}

	// Test iteration
	auto it = agg.cbegin();
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		ASSERT_EQ(it->get_point(), i);
		ASSERT_TRUE(it->is_start());
		it++;
	}
	for (int i = DYNSEGTREE_TESTSIZE - 1; i >= 0; --i) {
		ASSERT_EQ(it->get_point(), 2 * DYNSEGTREE_TESTSIZE - i + 1);
		ASSERT_TRUE(it->is_end());
		it++;
	}
	ASSERT_EQ(it, agg.cend());
}

TEST(__DST_BASENAME(DynSegTreeTest), OverlappingTest)
{
	__DST_BASENAME(Node) n[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		n[i].lower = static_cast<int>(i);
		n[i].upper = static_cast<int>(DYNSEGTREE_TESTSIZE + i);
		n[i].value = 1;
	}

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		agg.insert(n[i]);
	}

	int combined = agg.get_combined<MCombiner>();
	ASSERT_EQ(combined, DYNSEGTREE_TESTSIZE);

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		auto val = agg.query(static_cast<int>(i));
		ASSERT_EQ(val, i + 1);
	}
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		auto val = agg.query(static_cast<int>(i + DYNSEGTREE_TESTSIZE));
		ASSERT_EQ(val, DYNSEGTREE_TESTSIZE - i - 1);
	}
}

TEST(__DST_BASENAME(DynSegTreeTest), DeletionTest)
{
	__DST_BASENAME(Node) n[DYNSEGTREE_DELETION_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_DELETION_TESTSIZE; ++i) {
		n[i].lower = static_cast<int>(i);
		n[i].upper = static_cast<int>(DYNSEGTREE_DELETION_TESTSIZE + i);
		n[i].value = 1;
	}

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_DELETION_TESTSIZE; ++i) {
		agg.insert(n[i]);
	}

	std::mt19937 rng(DYNSEGTREE_SEED);

	for (unsigned int j = 0; j < DYNSEGTREE_DELETION_ITERATIONS; ++j) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, 2 * DYNSEGTREE_DELETION_TESTSIZE);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng) + 1;

		__DST_BASENAME(Node)
		deleteme(static_cast<int>(lower), static_cast<int>(upper), 42);

		agg.insert(deleteme);

		for (unsigned int i = 0; i < DYNSEGTREE_DELETION_TESTSIZE; ++i) {
			auto val = agg.query(static_cast<int>(i));

			if ((i >= lower) && (i < upper)) {
				ASSERT_EQ(val, i + 1 + 42);
			} else {
				ASSERT_EQ(val, i + 1);
			}
		}
		for (unsigned int i = 0; i < DYNSEGTREE_DELETION_TESTSIZE; ++i) {
			auto val = agg.query(static_cast<int>(i + DYNSEGTREE_DELETION_TESTSIZE));
			if ((i + DYNSEGTREE_DELETION_TESTSIZE >= lower) &&
			    (i + DYNSEGTREE_DELETION_TESTSIZE < upper)) {
				ASSERT_EQ(val, DYNSEGTREE_DELETION_TESTSIZE - i - 1 + 42);
			} else {
				ASSERT_EQ(val, DYNSEGTREE_DELETION_TESTSIZE - i - 1);
			}
		}

		agg.remove(deleteme);
		agg.dbg_verify_max_combiner<MCombiner>();

		for (unsigned int i = 0; i < DYNSEGTREE_DELETION_TESTSIZE; ++i) {
			auto val = agg.query(static_cast<int>(i));
			ASSERT_EQ(val, i + 1);
		}
		for (unsigned int i = 0; i < DYNSEGTREE_DELETION_TESTSIZE; ++i) {
			auto val = agg.query(static_cast<int>(i + DYNSEGTREE_DELETION_TESTSIZE));
			ASSERT_EQ(val, DYNSEGTREE_DELETION_TESTSIZE - i - 1);
		}

		int combined = agg.get_combined<MCombiner>();
		ASSERT_EQ(combined, DYNSEGTREE_DELETION_TESTSIZE);
	}
}

TEST(__DST_BASENAME(DynSegTreeTest), NestingInsertionOverlappingDeletionTest)
{
	__DST_BASENAME(Node) n[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		n[i].lower = 10 * static_cast<int>(i);
		n[i].upper = 10 * static_cast<int>(2 * DYNSEGTREE_TESTSIZE - i + 1);
		n[i].value = 1;
	}

	__DST_BASENAME(Node) transient[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		transient[i].lower = 10 * static_cast<int>(i);
		transient[i].upper = 10 * static_cast<int>(DYNSEGTREE_TESTSIZE + i);
		transient[i].value = 10;
	}

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		agg.insert(n[i]);
		agg.insert(transient[DYNSEGTREE_TESTSIZE - i - 1]);
		agg.dbg_verify();
	}

	auto val = agg.query(static_cast<int>(0));
	ASSERT_EQ(val, 11);

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		agg.remove(transient[i]);
		agg.dbg_verify();
	}

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		val = agg.query(static_cast<int>(10 * i));
		ASSERT_EQ(val, i + 1);
	}
}

TEST(__DST_BASENAME(DynSegTreeTest), ManyEqualTest)
{
	__DST_BASENAME(Node) n_middle[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		n_middle[i].lower = 10;
		n_middle[i].upper = 20;
		n_middle[i].value = 1;
	}

	__DST_BASENAME(Node) n_left[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		n_left[i].lower = 0;
		n_left[i].upper = 15;
		n_left[i].value = 7;
	}

	__DST_BASENAME(Node) n_right[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		n_right[i].lower = 15;
		n_right[i].upper = 25;
		n_right[i].value = 29;
	}

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		agg.insert(n_middle[i]);
		agg.insert(n_left[i]);
		agg.insert(n_right[i]);
	}

	auto val = agg.query(0);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7);
	val = agg.query(5);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7);
	val = agg.query(10);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7 + DYNSEGTREE_TESTSIZE * 1);
	val = agg.query(12);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7 + DYNSEGTREE_TESTSIZE * 1);
	val = agg.query(15);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 1 + DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(17);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 1 + DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(20);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(22);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(25);
	ASSERT_EQ(val, 0);
}

TEST(__DST_BASENAME(DynSegTreeTest), ComprehensiveTest)
{
	__DST_BASENAME(Node) persistent_nodes[DYNSEGTREE_COMPREHENSIVE_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(DYNSEGTREE_SEED);

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_COMPREHENSIVE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, 10 * DYNSEGTREE_COMPREHENSIVE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		persistent_nodes[i] = __DST_BASENAME(Node)(
		    static_cast<int>(lower), static_cast<int>(upper), static_cast<int>(i));
		indices.push_back(i);

		/*		std::cout << "Persistent: " << lower << " -> " << upper << ": " << i
		    << "\n";*/
	}

	__DST_BASENAME(Node) transient_nodes[DYNSEGTREE_COMPREHENSIVE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_COMPREHENSIVE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, 10 * DYNSEGTREE_COMPREHENSIVE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		transient_nodes[i] = __DST_BASENAME(Node)(
		    static_cast<int>(lower), static_cast<int>(upper),
		    static_cast<int>(DYNSEGTREE_COMPREHENSIVE_TESTSIZE + i));
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		agg.insert(transient_nodes[index]);
		agg.dbg_verify_max_combiner<MCombiner>();
	}

	// std::cout << "\n\n--- Before persistent: \n";
	// agg.dbg_print_inner_tree();

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		agg.insert(persistent_nodes[index]);
		agg.dbg_verify_max_combiner<MCombiner>();
	}

	// std::cout << "\n\n--- Before removal: \n";
	// agg.dbg_print_inner_tree();

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	auto checker = [&](std::set<size_t> & deleted_indices) {
		// Reference data structure
		using BoostMap = interval_map<int, int>;
		BoostMap reference;
		for (auto node : persistent_nodes) {
			reference += std::make_pair(
			    interval<int>::right_open(node.lower, node.upper), node.value);
		}

		for (size_t i = 0; i < DYNSEGTREE_COMPREHENSIVE_TESTSIZE; ++i) {
			if (deleted_indices.find(i) != deleted_indices.end()) {
				continue;
			}

			auto node = transient_nodes[i];
			reference += std::make_pair(
			    interval<int>::right_open(node.lower, node.upper), node.value);
		}

		int maxval = 0;

		auto it = reference.begin();
		while (it != reference.end()) {
			int lower = it->first.lower();
			int upper = it->first.upper();
			int val = it->second;
			/*			std::cout << "Reference: " << lower << " -> " << upper << " @ " <<
			   val
			      << "\n";*/

			for (int i = lower; i < upper; ++i) {
				int result = agg.query(static_cast<int>(i));
				// std::cout << "Query at " << i << "\n";
				ASSERT_EQ(val, result);
				maxval = std::max(result, maxval);
			}

			++it;
		}
		int combined = agg.get_combined<MCombiner>();
		int ranged_combined = agg.get_combined<RMCombiner>();
		ASSERT_EQ(combined, maxval);
		ASSERT_EQ(ranged_combined, maxval);
	};

	std::set<size_t> deleted;

	for (auto index : indices) {
		//		auto & to_remove = transient_nodes[index];
		/*		std::cout << "========= Removing " << to_remove.lower << " -> "
		    << to_remove.upper << " @ " << to_remove.value << ".\n";*/
		agg.remove(transient_nodes[index]);
		deleted.insert(index);

		agg.dbg_verify();
		checker(deleted);
	}

	// std::cout << "\n\n--- After removal: \n";
	// agg.dbg_print_inner_tree();

	agg.dbg_verify();
	agg.dbg_verify_max_combiner<MCombiner>();

	checker(deleted);
}

TEST(__DST_BASENAME(DynSegTreeTest), ComprehensiveCombinerTest)
{
	std::mt19937 rng(DYNSEGTREE_SEED + 1);

	__DST_BASENAME(Node) persistent_nodes[DYNSEGTREE_COMPREHENSIVE_TESTSIZE];
	std::vector<unsigned int> indices;

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_COMPREHENSIVE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, 10 * DYNSEGTREE_COMPREHENSIVE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		persistent_nodes[i] =
			__DST_BASENAME(Node)(static_cast<int>(lower), static_cast<int>(upper), static_cast<int>(i));
		indices.push_back(i);
	}

	__DST_BASENAME(Node) transient_nodes[DYNSEGTREE_COMPREHENSIVE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_COMPREHENSIVE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, 10 * DYNSEGTREE_COMPREHENSIVE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		transient_nodes[i] = __DST_BASENAME(Node)(
		                                          static_cast<int>(lower), static_cast<int>(upper), static_cast<int>(DYNSEGTREE_COMPREHENSIVE_TESTSIZE + i));
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		agg.insert(transient_nodes[index]);
		agg.dbg_verify_max_combiner<MCombiner>();
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		agg.insert(persistent_nodes[index]);
		agg.dbg_verify_max_combiner<MCombiner>();
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		agg.remove(transient_nodes[index]);
		agg.dbg_verify_max_combiner<MCombiner>();
	}

	// Reference data structure
	using BoostMap = interval_map<int, int>;
	BoostMap reference;

	for (auto node : persistent_nodes) {
		reference += std::make_pair(
		    interval<int>::right_open(node.lower, node.upper), node.value);
	}

	auto start_it = reference.begin();
	while (start_it != reference.end()) {
		auto end_it = start_it;
		++end_it;

		while (end_it != reference.end()) {
			auto it = start_it;
			int max_seen = it->second;
			int range_lower = start_it->first.lower();
			int range_upper = start_it->first.upper();

			bool upper_closed = false;
			std::set<std::pair<int, int>> max_seen_ranges;
			while (it != end_it) {
				if (it->second == max_seen) {
					max_seen_ranges.insert({it->first.lower(), it->first.upper()});
				}
				if (it->second > max_seen) {
					max_seen_ranges.clear();
					max_seen_ranges.insert({it->first.lower(), it->first.upper()});
				}

				max_seen = std::max(max_seen, it->second);
				range_upper = it->first.upper();
				if (boost::icl::contains(it->first, it->first.upper())) {
					upper_closed = true;
				} else {
					upper_closed = false;
				}
				++it;
			}

			bool lower_closed = false;
			if (boost::icl::contains(start_it->first, start_it->first.lower())) {
				lower_closed = true;
			}

			int combined = agg.get_combined<MCombiner>(range_lower, range_upper,
			                                           lower_closed, upper_closed);

			ASSERT_EQ(combined, max_seen);

			auto range_combiner = agg.get_combiner<RMCombiner>(
			    range_lower, range_upper, lower_closed, upper_closed);
			std::pair<int, int> max_range{
			    std::max(range_combiner.get_left_border(), range_lower),
			    std::min(range_combiner.get_right_border(), range_upper)};

			++end_it;
		}
		++start_it;
	}
}

TEST(__DST_BASENAME(RangedMaxCombinerTest), TrivialTest)
{
	__DST_BASENAME(Node) n(2, 5, 10);

	__DST_BASENAME(DynSegTree) agg;
	agg.insert(n);

	auto combiner = agg.get_combiner<RMCombiner>();

	ASSERT_EQ(combiner.get(), 10);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 2);
	ASSERT_EQ(combiner.get_right_border(), 5);

	combiner = agg.get_combiner<RMCombiner>(0, 10, true, true);
	ASSERT_EQ(combiner.get(), 10);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 2);
	ASSERT_EQ(combiner.get_right_border(), 5);
}

TEST(__DST_BASENAME(RangedMaxCombinerTest), StepTest)
{
	__DST_BASENAME(Node) n1(0, 10, 1);
	__DST_BASENAME(Node) n2(1, 10, 1);
	__DST_BASENAME(Node) n3(2, 10, 1);

	__DST_BASENAME(DynSegTree) agg;
	agg.insert(n1);
	agg.insert(n2);
	agg.insert(n3);

	auto combiner = agg.get_combiner<RMCombiner>();

	ASSERT_EQ(combiner.get(), 3);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 2);
	ASSERT_EQ(combiner.get_right_border(), 10);

	combiner = agg.get_combiner<RMCombiner>(0, 1, true, false);
	ASSERT_EQ(combiner.get(), 1);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 0);
	ASSERT_EQ(combiner.get_right_border(), 1);

	combiner = agg.get_combiner<RMCombiner>(0, 1, true, true);
	ASSERT_EQ(combiner.get(), 2);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 1);
	ASSERT_EQ(combiner.get_right_border(), 2);

	combiner = agg.get_combiner<RMCombiner>(0, 2, true, false);
	ASSERT_EQ(combiner.get(), 2);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 1);
	ASSERT_EQ(combiner.get_right_border(), 2);

	combiner = agg.get_combiner<RMCombiner>(0, 2, true, true);
	ASSERT_EQ(combiner.get(), 3);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 2);
	ASSERT_EQ(combiner.get_right_border(), 10);
}

TEST(__DST_BASENAME(RangedMaxCombinerTest), MergingTest)
{
	// Two non-merging intervals with the same value
	__DST_BASENAME(Node) n1(2, 5, 10);
	__DST_BASENAME(Node) n2(6, 10, 10);

	// Two merging intervals with the same value
	__DST_BASENAME(Node) n3(12, 15, 10);
	__DST_BASENAME(Node) n4(15, 20, 10);

	// Two non-merging intervals with different values
	__DST_BASENAME(Node) n5(22, 25, 10);
	__DST_BASENAME(Node) n6(25, 30, 15);

	__DST_BASENAME(DynSegTree) agg;
	agg.insert(n1);
	agg.insert(n2);
	agg.insert(n3);
	agg.insert(n4);
	agg.insert(n5);
	agg.insert(n6);

	auto combiner = agg.get_combiner<RMCombiner>();
	ASSERT_EQ(combiner.get(), 15);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 25);
	ASSERT_EQ(combiner.get_right_border(), 30);

	combiner = agg.get_combiner<RMCombiner>(1, 11, true, true);
	ASSERT_EQ(combiner.get(), 10);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	bool left_found =
	    (combiner.get_left_border() == 2) && (combiner.get_right_border() == 5);
	bool right_found =
	    (combiner.get_left_border() == 6) && (combiner.get_right_border() == 10);
	ASSERT_TRUE(left_found || right_found);

	combiner = agg.get_combiner<RMCombiner>(11, 21, true, true);
	ASSERT_EQ(combiner.get(), 10);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 12);
	ASSERT_EQ(combiner.get_right_border(), 20);

	combiner = agg.get_combiner<RMCombiner>(21, 31, true, true);
	ASSERT_EQ(combiner.get(), 15);
	ASSERT_TRUE(combiner.is_left_border_valid());
	ASSERT_TRUE(combiner.is_right_border_valid());
	ASSERT_EQ(combiner.get_left_border(), 25);
	ASSERT_EQ(combiner.get_right_border(), 30);
}

TEST(__DST_BASENAME(RangedMaxCombinerTest), NestingTest)
{
	__DST_BASENAME(Node) n[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		n[i].lower = static_cast<int>(i);
		n[i].upper = static_cast<int>(2 * DYNSEGTREE_TESTSIZE - i + 1);
		n[i].value = 1;
	}

	__DST_BASENAME(DynSegTree) agg;

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		agg.insert(n[i]);
	}

	auto combiner = agg.get_combiner<RMCombiner>();
	ASSERT_EQ(combiner.get(), DYNSEGTREE_TESTSIZE);
	ASSERT_EQ(combiner.get_left_border(), DYNSEGTREE_TESTSIZE - 1);
	ASSERT_EQ(combiner.get_right_border(),
	          2 * DYNSEGTREE_TESTSIZE - (DYNSEGTREE_TESTSIZE - 1) + 1);

	combiner = agg.get_combiner<RMCombiner>(
	    DYNSEGTREE_TESTSIZE / 2, (DYNSEGTREE_TESTSIZE / 2) + DYNSEGTREE_TESTSIZE,
	    true, true);
	ASSERT_EQ(combiner.get(), DYNSEGTREE_TESTSIZE);
	ASSERT_EQ(combiner.get_left_border(), DYNSEGTREE_TESTSIZE - 1);
	ASSERT_EQ(combiner.get_right_border(),
	          2 * DYNSEGTREE_TESTSIZE - (DYNSEGTREE_TESTSIZE - 1) + 1);

	for (unsigned int i = 0; i < static_cast<unsigned int>(DYNSEGTREE_TESTSIZE) - 1u; ++i) {
		combiner = agg.get_combiner<RMCombiner>(static_cast<int>(i), static_cast<int>(i + 1),
		                                        true, false);
		ASSERT_EQ(combiner.get(), i + 1);
		ASSERT_EQ(combiner.get_left_border(), i);
		ASSERT_EQ(combiner.get_right_border(), i + 1);

		combiner = agg.get_combiner<RMCombiner>(0, static_cast<int>(i + 1), true, true);
		ASSERT_EQ(combiner.get(), i + 2);
		ASSERT_EQ(combiner.get_left_border(), i + 1);
		ASSERT_TRUE(combiner.get_right_border() >= static_cast<int>(i + 1));
	}
}

} // namespace dynamic_segment_tree
} // namespace testing
} // namespace ygg
