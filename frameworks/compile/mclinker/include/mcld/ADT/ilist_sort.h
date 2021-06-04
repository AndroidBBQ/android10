//===- ilist_sort.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_ADT_ILIST_SORT_H_
#define MCLD_ADT_ILIST_SORT_H_

#include <llvm/ADT/ilist.h>

#include <functional>
#include <iterator>

namespace mcld {

namespace detail {

template <typename T, typename Alloc, typename Comparator>
void sort(llvm::iplist<T, Alloc>& xs, size_t size, Comparator is_less_than) {
  typedef llvm::iplist<T, Alloc> iplist;
  typedef typename iplist::iterator iterator;

  assert(xs.size() == size && "Incorrect list size argument");

  // Special cases for induction basis.
  switch (size) {
    case 0:
    case 1: {
      return;
    }
    case 2: {
      iterator first = xs.begin();
      iterator second = xs.begin();
      ++second;
      if (is_less_than(*second, *first)) {
        xs.splice(first, xs, second);
      }
      return;
    }
  }

  // Split the input linked list.
  size_t mid = size / 2;
  iterator mid_iter(xs.begin());
  std::advance(mid_iter, mid);

  iplist ys;
  ys.splice(ys.begin(), xs, mid_iter, xs.end());

  // Sort the xs and ys recursively.
  sort(xs, mid, is_less_than);
  sort(ys, size - mid, is_less_than);

  // Merge two sorted linked lists.
  iterator xs_it = xs.begin();
  iterator xs_end = xs.end();
  iterator ys_it = ys.begin();
  iterator ys_end = ys.end();

  while (xs_it != xs_end && ys_it != ys_end) {
    if (is_less_than(*ys_it, *xs_it)) {
      xs.splice(xs_it, ys, ys_it++);
    } else {
      ++xs_it;
    }
  }

  xs.splice(xs_end, ys, ys_it, ys_end);
}

}  // namespace detail

template <typename T, typename Alloc, typename Comparator = std::less<T> >
void sort(llvm::iplist<T, Alloc>& list,
          Comparator is_less_than = Comparator()) {
  detail::sort(list, list.size(), is_less_than);
}

}  // namespace mcld

#endif  // MCLD_ADT_ILIST_SORT_H_
