// This file is part of LatMRG.
//
// LatMRG
// Copyright (C) 2012-2016  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LATMRG__COORDINATE_SETS_H
#define LATMRG__COORDINATE_SETS_H

#include <set>
#include <map>
#include <iostream>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/config.hpp>
#include "latticetester/Util.h"
#include "latticetester/Types.h"
#include "latmrg/Coordinates.h"

namespace LatMRG {

/**
 * Sequences of coordinate sets.
 *
 * These can be used to specify a set of projections of lattices or point sets.
 * The classes in this namespace are iterable but are not containers, so they
 * require very little storage.  They virtually contain objects of type
 * LatticeTester::Coordinates.
 */
namespace CoordinateSets {

/**
 * Implementation of CoordinateSets for coordinates within a given range.
 */
class FromRanges {

public:
    /**
     * Constructs a set of all subsets of \f$\{\mathtt{minCoord}, \dots,
     * \mathtt{maxCoord}\}\f$ with minimum and maximum cardinality
     * specified by \c minOrder and \c maxOrder.
     * For example, to select  all 1, 2, and 3-tuples over coordinates 2, 3, 4,
     * one may use the declaration <tt>FromRanges range(1, 3, 2, 4)</tt>;
     * this gives the sets
     * <tt>range = {{2}, {3}, {4}, {2, 3}, {2, 4}, {3, 4}, {2, 3, 4}}</tt>.
     */
    FromRanges (Coordinates::size_type  minOrder, Coordinates::size_type  maxOrder,
                Coordinates::value_type minCoord, Coordinates::value_type maxCoord);

    /**
     * Constructs an empty set of coordinate sets.
     *
     */
    FromRanges();

    /**
     * Include all subsets \f$\mathfrak u\f$ of
     * \f$\{\mathtt{minCoord}, \dots, \mathtt{maxCoord}\}\f$ of order
     * \f$|\mathfrak u| = \mathtt{order}\f$.
     * For example, calling \c includeOrder(3, 1, 5) causes all 3-tuples over
     * coordinates \f$1, \dots, 5\f$ to be included.
     * If \c order is \c 0 (corresponding to the empty set),
     * \c minCoord and \c maxCoord are ignored.
     * Except for the case where \c order = 0, an exception is thrown if
     * \f$\mathtt{maxCoord} < \mathtt{minCoord} + \mathtt{order} - 1\f$.
     */
    void includeOrder (Coordinates::size_type order,
                       Coordinates::value_type minCoord,
                       Coordinates::value_type maxCoord);

    /**
     * Excludes coordinate sets \f$\mathfrak u\f$ of order
     * \f$|\mathfrak u| = \mathtt{order}\f$.
     */
    void excludeOrder (Coordinates::size_type order);

private:
    typedef std::pair<Coordinates::value_type, Coordinates::value_type> Range;
    typedef std::map<Coordinates::size_type, Range> RangeMap;

    const RangeMap& ranges() const { return m_ranges; }

public:
    class const_iterator : public boost::iterators::iterator_facade<const_iterator,
                                                                    const Coordinates, // value
                                                                    boost::iterators::forward_traversal_tag> // traversal
    {
        public:
            struct end_tag {};

            explicit const_iterator(const FromRanges& seq):
            m_seq(&seq), m_atEnd(false)
            { resetToOrder (m_seq->ranges().begin()); }

            const_iterator(const FromRanges& seq, end_tag):
            m_seq(&seq), m_atEnd(true) // end
            {}

            const_iterator():
            m_seq(nullptr), m_atEnd(false)
            { }

        private:
            friend class boost::iterators::iterator_core_access;

            void increment();

            bool equal(const const_iterator& other) const
            { return m_seq == other.m_seq && (other.m_atEnd ? m_atEnd : m_value == other.m_value); }

            const Coordinates& dereference() const
            { return m_value; }

            void resetToOrder (const RangeMap::const_iterator& it);

        private:
            const FromRanges* m_seq;
            bool m_atEnd;
            Coordinates m_value;

    }; // end cons_iterator class

    /**
     * Returns an iterator pointing to the first element in the seq.
     */
    const_iterator begin() const
    { return const_iterator(*this); }

    /**
     * Returns an iterator pointing past the last element in the seq.
     */
    const_iterator end() const
    { return const_iterator(*this, typename const_iterator::end_tag{}); }

private:
    RangeMap m_ranges; // coordinate range for each order

}; // end FromRanges class


/**
 * This class implements CoordinateSets for any set of coordinates.
 * It is more powerful than the class Range, but
 * slightly slower (by 15--20\,\% according to empirical tests).
 */
class Subsets {

/**
 * Constructs a set of all subsets of \c coords with minimum and maximum
 * cardinality specified by \c minOrder and \c maxOrder.
 * For example, to select  all 1, 2, and 3-tuples over coordinates 2, 4, 6,
 * one may use the declaration <tt>Subsets tousens(ens, 1, 3)</tt>,
 * where  set <tt>ens</tt> is \{2,4,6\}; this gives the sets
 * <tt>tousens = {{2}, {4}, {6}, {2, 4}, {2, 6}, {4, 6}, {2, 4, 6}}</tt>.
 */
public:
    Subsets (const Coordinates & coords,
             Coordinates::size_type minOrder,
             Coordinates::size_type maxOrder);

    const Coordinates& coords() const { return  m_coords; }
    Coordinates::size_type minOrder() const { return m_minOrder; }
    Coordinates::size_type maxOrder() const { return m_maxOrder; }

public:
    /**
     * Iterator
     */
    class const_iterator : public boost::iterators::iterator_facade<const_iterator,
                                                                    const Coordinates, // value
                                                                    boost::iterators::forward_traversal_tag> // traversal
    {
        public:
            struct end_tag {};

            explicit const_iterator(const Subsets& seq):
            m_seq(&seq), m_atEnd(false)
            { resetToOrder (m_seq->minOrder()); }

            const_iterator(const Subsets& seq, end_tag):
            m_seq(&seq), m_atEnd(true) // end
            {}

            const_iterator():
            m_seq(nullptr), m_atEnd(false)
            { }

        private:
            friend class boost::iterators::iterator_core_access;

            void increment();

            bool equal(const const_iterator& other) const
            { return m_seq == other.m_seq && (other.m_atEnd ? m_atEnd : m_value == other.m_value); }

            const Coordinates& dereference() const
            { return m_value; }

            void resetToOrder (Coordinates::size_type order);

        private:
            const Subsets* m_seq;
            bool m_atEnd;
            Coordinates m_value;

    }; // end const_iterator class

    /**
     * Returns an iterator pointing to the first element in the seq.
     */
    const_iterator begin() const
    { return const_iterator(*this); }

    /**
     * Returns an iterator pointing past the last element in the seq.
     */
    const_iterator end() const
    { return const_iterator(*this, typename const_iterator::end_tag{}); }

private:
    const Coordinates m_coords;
    Coordinates::size_type m_minOrder;
    Coordinates::size_type m_maxOrder;

}; // end Subsets class



/**
 * This template class wraps any implementation of CoordinateSets and
 * adds a specific coordinate to each coordinate sets.
 *
 * \tparam BASE Type of coordinate sets that serves as a base.
 */
template <typename BASE>
class AddCoordinate {

/**
 * Constructs a set based on <tt>baseSets</tt> by adding coordinate
 * \c coord to each of the sets in <tt>baseSets</tt>.
 * Warning: do not use a temporary object as <tt>baseSets</tt>.
 */
public:
    typedef BASE BMat;

    /**
     * Constructs a sequence of coordinate sets by adding the coordinate \c coord
     * to each element in the base sequence \c base.
     */
    AddCoordinate (const BMat& base, Coordinates::value_type coord):
    m_base(base), m_coord(coord)
    { }

    #ifdef BOOST_HAS_RVALUE_REFS
    AddCoordinate (BMat&& base, Coordinates::value_type coord):
    m_base(std::move(base)), m_coord(coord)
    { }
    #endif

    /**
     * Returns the base sequence.
     */
    const BMat& base() const
    { return  m_base; }

    /**
     * Returns the added coordinate.
     */
    const Coordinates::value_type& coord() const
    { return m_coord; }

public:
    class const_iterator : public boost::iterators::iterator_adaptor<const_iterator,
    typename BMat::const_iterator>
    {
        public:
            struct end_tag {};

            explicit const_iterator(const AddCoordinate& seq):
            const_iterator::iterator_adaptor_(seq.base().begin()),
            m_seq(&seq)
            { updateValue(); }

            const_iterator(const AddCoordinate& seq, end_tag):
            const_iterator::iterator_adaptor_(seq.base().end()),
            m_seq(&seq)
            {}

            const_iterator():
            const_iterator::iterator_adaptor_(),
            m_seq(nullptr)
            { }

        private:
            friend class boost::iterators::iterator_core_access;

            void increment()
            { ++this->base_reference(); updateValue(); }

            const Coordinates& dereference() const
            { return m_value; }

            void updateValue()
            { m_value = *this->base_reference(); m_value.insert(m_seq->coord()); }

        private:
            const AddCoordinate* m_seq;
            Coordinates m_value;
    }; // end iterator_adaptator class

    /**
     * Returns an iterator pointing to the first element in the seq.
     */
    const_iterator begin() const
    { return const_iterator(*this); }

    /**
     * Returns an iterator pointing past the last element in the seq.
     */
    const_iterator end() const
    { return const_iterator(*this, typename const_iterator::end_tag{}); }

private:
    BMat m_base;
    Coordinates::value_type m_coord;

}; // end AddCoordinate class


} // end CoordinateSets namespace
} // end LatMRG namspace

#endif // COORDINATE_SETS_H
